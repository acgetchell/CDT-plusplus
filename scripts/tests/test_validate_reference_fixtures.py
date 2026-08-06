"""Regression tests for the cross-language reference-package validator."""

import copy
import tempfile
import unittest
from pathlib import Path
from typing import override
from unittest import mock

from scripts import validate_reference_fixtures as validator


class ReferenceFixtureValidationTests(unittest.TestCase):
    """Exercise the validation gaps that previously admitted stale fixtures."""

    @classmethod
    @override
    def setUpClass(cls) -> None:
        """Load the committed package once; individual tests mutate copies."""
        cls.fixture_path = validator.REFERENCE / "fixtures" / "v1" / "protocol.json"
        cls.result_path = validator.REFERENCE / "raw" / "v1" / "cpp-reference.json"
        cls.fixture_schema_path = validator.REFERENCE / "schema" / "fixture-v1.schema.json"
        cls.result_schema_path = validator.REFERENCE / "schema" / "result-v1.schema.json"
        cls.end_to_end_path = validator.REFERENCE / "raw" / "v1" / "end-to-end.txt"
        cls.manifest_path = validator.REFERENCE / "manifests" / "v1" / "macos-arm64.json"
        cls.protocol = validator.load_json(cls.fixture_path)
        cls.raw = validator.load_json(cls.result_path)
        cls.manifest = validator.load_json(cls.manifest_path)
        cls.fixture_schema = validator.load_json(cls.fixture_schema_path)
        cls.result_schema = validator.load_json(cls.result_schema_path)

    def test_complete_schema_rejects_nested_unknown_fields(self) -> None:
        """Nested entity shapes are enforced, not only top-level keys."""
        raw = copy.deepcopy(self.raw)
        raw["states"][0]["vertices"][0]["unexpected"] = True

        with self.assertRaisesRegex(ValueError, "schema violation.*unexpected"):
            validator.validate_document(raw, self.result_schema, Path("result.json"))

    def test_transition_site_must_resolve_in_raw_topology(self) -> None:
        """A stale canonical entity id cannot survive protocol validation."""
        protocol = copy.deepcopy(self.protocol)
        protocol["transitions"][0]["site"]["entity"] = "c99"

        with self.assertRaisesRegex(ValueError, "unknown cell id 'c99'"):
            validator.validate_protocol(protocol, self.raw)

    def test_state_edges_must_be_derived_from_cells(self) -> None:
        """A canonical-looking edge outside every cell is rejected."""
        state = copy.deepcopy(next(item for item in self.raw["states"] if item["id"] == "move-23-before"))
        state["edges"][2]["vertices"] = ["v00", "v04"]
        state["edges"].sort(key=lambda edge: edge["vertices"])
        for index, edge in enumerate(state["edges"]):
            edge["id"] = f"e{index:02d}"

        with self.assertRaisesRegex(ValueError, "edge set does not equal cell incidence"):
            validator.validate_state(state)

    def test_generated_payload_uses_declared_numerical_tolerances(self) -> None:
        """Within-tolerance coordinate and action drift remains interoperable."""
        generated = copy.deepcopy(self.raw)
        generated["states"][0]["vertices"][0]["position"][0] += 1e-15
        generated["actions"][0]["value"] += 1e-14

        validator.compare_generated_scientific_payload(generated, self.raw, self.protocol)

    def test_generated_payload_rejects_out_of_tolerance_coordinates(self) -> None:
        """Coordinates outside the named tolerance fail generated comparison."""
        generated = copy.deepcopy(self.raw)
        generated["states"][0]["vertices"][0]["position"][0] += 1e-10

        with self.assertRaisesRegex(ValueError, "coordinate tolerance"):
            validator.compare_generated_scientific_payload(generated, self.raw, self.protocol)

    def test_generated_payload_keeps_topology_exact(self) -> None:
        """Numerical tolerance never weakens exact incidence comparison."""
        generated = copy.deepcopy(self.raw)
        generated["states"][0]["edges"][0]["type"] = "timelike"

        with self.assertRaisesRegex(ValueError, "exact topology or metadata"):
            validator.compare_generated_scientific_payload(generated, self.raw, self.protocol)

    def test_manifest_records_one_producer_for_every_raw_artifact(self) -> None:
        """A checksum alone cannot conceal missing regeneration provenance."""
        manifest = copy.deepcopy(self.manifest)
        manifest["commands"][0]["artifacts"] = []

        with self.assertRaisesRegex(ValueError, "exactly one producer"):
            validator.validate_manifest(manifest)

    def test_manifest_commands_must_match_the_protocol(self) -> None:
        """Recorded producer argv cannot drift from the retained experiment."""
        manifests = [
            copy.deepcopy(self.manifest),
            validator.load_json(validator.REFERENCE / "manifests" / "v1" / "scaling-macos-arm64.json"),
        ]
        manifests[0]["commands"][1]["command_line"][2] = "-n3"

        with self.assertRaisesRegex(ValueError, "do not match"):
            validator.validate_command_provenance(manifests, self.protocol)

    def test_archival_provenance_rejects_dirty_revisions(self) -> None:
        """Archival validation requires one clean commit across all records."""
        raw = copy.deepcopy(self.raw)
        manifests = [copy.deepcopy(self.manifest)]
        dirty_revision = "a" * 40 + "-dirty"
        raw["implementation"]["revision"] = dirty_revision
        manifests[0]["implementation"]["source_revision"] = dirty_revision

        with (
            mock.patch.object(
                validator,
                "parse_key_value_record",
                return_value={"implementation.revision": dirty_revision},
            ),
            self.assertRaisesRegex(ValueError, "clean Git commit"),
        ):
            validator.validate_clean_provenance(raw, manifests)

    def test_fnv1a64_uses_the_published_offset_basis(self) -> None:
        """The persistence checksum matches the published empty-input vector."""
        self.assertEqual(validator.fnv1a64(b""), "cbf29ce484222325")

    def test_key_value_records_reject_malformed_lines(self) -> None:
        """Every scaling-record line must use the declared key=value syntax."""
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "record.txt"
            path.write_text("record.schema=fixture-v1\nunexpected output\n", encoding="utf-8")

            with self.assertRaisesRegex(ValueError, r"record\.txt:2: expected a key=value record"):
                validator.parse_key_value_record(path)

    def test_key_value_records_reject_duplicate_keys(self) -> None:
        """Conflicting provenance cannot depend on first- or last-key wins."""
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "record.txt"
            path.write_text(
                "implementation.revision=first\nimplementation.revision=second\n",
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, r"record\.txt:2: duplicate key 'implementation\.revision'"):
                validator.parse_key_value_record(path)

    def test_scaling_records_require_every_raw_sample(self) -> None:
        """Each timing series contains the declared number of repetitions."""
        records = [validator.parse_key_value_record(validator.REFERENCE / "raw" / "v1" / f"scaling-threads-{threads}.txt") for threads in (1, 2, 4)]
        records[0]["bulk_insert_ns_samples"] = "1,2,3,4"

        with (
            mock.patch.object(validator, "parse_key_value_record", side_effect=records),
            self.assertRaisesRegex(ValueError, "raw sample count does not match repetitions"),
        ):
            validator.validate_scaling_records()

    def test_proposal_probabilities_are_derived_from_raw_domains(self) -> None:
        """Preserving a wrong Hastings ratio does not conceal scaled inputs."""
        protocol = copy.deepcopy(self.protocol)
        transition = protocol["transitions"][0]
        transition["proposal_probability"] *= 0.5
        transition["reverse_probability"] *= 0.5

        with self.assertRaisesRegex(ValueError, "forward proposal probability"):
            validator.validate_protocol(protocol, self.raw)

    def test_bounded_run_command_must_match_retained_output(self) -> None:
        """A stale timeslice option is rejected against raw provenance."""
        protocol = copy.deepcopy(self.protocol)
        command = protocol["bounded_run"]["command"]
        command[command.index("-t2")] = "-t3"

        with self.assertRaisesRegex(ValueError, "desired timeslices"):
            validator.validate_end_to_end(protocol, self.end_to_end_path)

    def test_bounded_run_f_vector_must_lie_inside_declared_band(self) -> None:
        """The declared randomized band is executable acceptance data."""
        protocol = copy.deepcopy(self.protocol)
        protocol["bounded_run"]["bands"]["randomized_cgal_f_vector"]["maximum"][0] = 5

        with self.assertRaisesRegex(ValueError, "outside its declared band"):
            validator.validate_end_to_end(protocol, self.end_to_end_path)

    def test_current_protocol_satisfies_its_complete_schema(self) -> None:
        """The versioned protocol itself exercises the complete schema path."""
        validator.validate_document(self.protocol, self.fixture_schema, self.fixture_path)


if __name__ == "__main__":
    unittest.main()
