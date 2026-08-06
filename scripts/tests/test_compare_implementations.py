"""Tests for the local CDT++/Rust comparison harness."""

import copy
import sys
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import Any, cast
from unittest.mock import patch

from scripts.compare_implementations import (
    DEFAULT_FIXTURE_SCHEMA,
    DEFAULT_MANIFEST_SCHEMA,
    DEFAULT_PROTOCOL,
    DEFAULT_REFERENCE_MANIFEST,
    DEFAULT_REFERENCE_RESULT,
    DEFAULT_RESULT_SCHEMA,
    ComparisonRecorder,
    ProcessSpec,
    _compare_results,
    _expanded_arguments,
    reanalyze_comparison,
    run_comparison,
)
from scripts.experiment_artifacts import OutputDirectoryExistsError, staging_directory_prefix, write_json
from scripts.validate_reference_fixtures import load_json

ROOT = Path(__file__).resolve().parents[2]
REFERENCE_RESULT = ROOT / "reference" / "raw" / "v1" / "cpp-reference.json"


def _sources() -> dict[str, Path]:
    """Return the committed #94 comparison inputs."""
    return {
        "fixture_schema": DEFAULT_FIXTURE_SCHEMA,
        "manifest_schema": DEFAULT_MANIFEST_SCHEMA,
        "protocol": DEFAULT_PROTOCOL,
        "reference_manifest": DEFAULT_REFERENCE_MANIFEST,
        "reference_result": DEFAULT_REFERENCE_RESULT,
        "result_schema": DEFAULT_RESULT_SCHEMA,
    }


def _emitter(name: str, mutation: str = "") -> ProcessSpec:
    """Build a dependency-free structured-result producer command."""
    code = (
        "import json; from pathlib import Path; "
        f"payload=json.loads(Path({str(REFERENCE_RESULT)!r}).read_text()); "
        f"payload['implementation']['name']={name!r}; "
        f"{mutation}"
        "print(json.dumps(payload, allow_nan=False))"
    )
    return ProcessSpec(name.lower(), sys.executable, ("-c", code), ROOT)


def _failures(summary: dict[str, object]) -> list[dict[str, Any]]:
    """Narrow the stable summary failure array for test assertions."""
    return cast("list[dict[str, Any]]", summary["failures"])


class ComparisonHarnessTests(unittest.TestCase):
    """Verify command, artifact, classification, and replay contracts."""

    def test_command_arguments_expand_only_bundle_inputs(self) -> None:
        """Both producers can receive the same copied protocol and schemas."""
        bundle = Path("comparison")
        expanded = _expanded_arguments(("--fixture", "{protocol}", "--schema={result_schema}", "literal"), bundle)
        self.assertEqual(expanded[0], "--fixture")
        self.assertEqual(Path(expanded[1]).name, "protocol.json")
        self.assertTrue(expanded[2].endswith("inputs/result-v1.schema.json"))
        self.assertEqual(expanded[3], "literal")

    def test_run_retains_complete_artifacts_and_reanalysis_is_deterministic(self) -> None:
        """A stored bundle reproduces its summary without launching a producer."""
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            summary = run_comparison(
                bundle,
                _emitter("cpp"),
                _emitter("rust"),
                _sources(),
                10,
            )

            self.assertEqual(summary["status"], "passed")
            for relative in (
                "inputs/protocol.json",
                "inputs/cpp-reference.json",
                "inputs/result-v1.schema.json",
                "raw/cpp/process.json",
                "raw/cpp/stdout.txt",
                "raw/cpp/stderr.txt",
                "raw/rust/process.json",
                "raw/rust/stdout.txt",
                "raw/rust/stderr.txt",
                "manifest.json",
                "summary.json",
            ):
                with self.subTest(artifact=relative):
                    self.assertTrue((bundle / relative).is_file())

            first_summary = (bundle / "summary.json").read_bytes()
            reproduced = reanalyze_comparison(bundle)
            self.assertEqual(reproduced, summary)
            self.assertEqual((bundle / "summary.json").read_bytes(), first_summary)

            manifest = load_json(bundle / "manifest.json")
            self.assertIn("analysis_ns", manifest["timing"])
            self.assertEqual(set(manifest["timing"]["process_ns"]), {"cpp", "rust"})
            self.assertNotIn("COMET_API_KEY", manifest["processes"]["rust"]["environment"])

    def test_exact_failure_names_fixture_rule_values_and_raw_artifact(self) -> None:
        """Exact topology failures contain every field required for investigation."""
        mutation = "payload['states'][0]['cells'][0]['type']='2-2'; "
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            summary = run_comparison(
                bundle,
                _emitter("cpp"),
                _emitter("rust", mutation),
                _sources(),
                10,
            )

        self.assertEqual(summary["status"], "failed")
        failure = next(item for item in _failures(summary) if item["field"] == "cells[0]")
        self.assertEqual(failure["implementation"], "rust")
        self.assertEqual(failure["fixture"], "state:causal-simplex")
        self.assertEqual(failure["rule"], {"kind": "exact"})
        self.assertEqual(cast("dict[str, Any]", failure["expected"])["type"], "3-1")
        self.assertEqual(cast("dict[str, Any]", failure["observed"])["type"], "2-2")
        self.assertEqual(failure["raw_artifact"], "raw/rust/stdout.txt")

    def test_live_cpp_result_is_anchored_to_the_committed_reference(self) -> None:
        """Matching live producers cannot hide drift from the canonical #94 result."""
        mutation = "payload['states'][0]['cells'][0]['type']='2-2'; "
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            summary = run_comparison(
                bundle,
                _emitter("cpp", mutation),
                _emitter("rust", mutation),
                _sources(),
                10,
            )

        failures = _failures(summary)
        self.assertEqual(summary["status"], "failed")
        self.assertTrue(any(item["implementation"] == "cpp" and item["field"] == "cells[0]" for item in failures))
        self.assertFalse(any(item["implementation"] == "rust" and item["field"] == "cells[0]" for item in failures))

    def test_live_cpp_transitions_are_anchored_to_the_protocol(self) -> None:
        """Matching live transition drift cannot hide from the committed #94 oracle."""
        transitions = (
            f"payload['schema']='cdt-comparison-result-v1'; payload['transitions']=json.loads(Path({str(DEFAULT_PROTOCOL)!r}).read_text())['transitions']; "
            "payload['transitions'][0]['action_delta']+=0.5; "
        )
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            summary = run_comparison(
                bundle,
                _emitter("cpp", transitions),
                _emitter("rust", transitions),
                _sources(),
                10,
            )

        failures = _failures(summary)
        self.assertEqual(summary["status"], "failed")
        self.assertTrue(
            any(item["implementation"] == "cpp" and item["fixture"] == "transition:move-23-accepted" and item["field"] == "action_delta" for item in failures)
        )
        self.assertFalse(any(item["implementation"] == "rust" and item["field"] == "action_delta" for item in failures))

    def test_named_tolerance_accepts_roundoff_and_rejects_a_material_delta(self) -> None:
        """Numerical comparisons use the protocol quantity, not a global percent."""
        cpp = load_json(REFERENCE_RESULT)
        protocol = load_json(DEFAULT_PROTOCOL)
        for delta, expected_failures in ((1e-15, 0), (1e-3, 1)):
            with self.subTest(delta=delta):
                rust = copy.deepcopy(cpp)
                rust["actions"][0]["value"] += delta
                recorder = ComparisonRecorder()
                _compare_results(cpp, rust, protocol, recorder)
                failures = [item for item in recorder.failures if item["field"] == "value"]
                self.assertEqual(len(failures), expected_failures)
                if failures:
                    self.assertEqual(cast("dict[str, Any]", failures[0]["rule"])["name"], "regge_action_closed_form")

    def test_language_neutral_transition_observations_use_protocol_rules(self) -> None:
        """Full transition records receive exact, action, and probability checks."""
        transitions = (
            f"payload['schema']='cdt-comparison-result-v1'; payload['transitions']=json.loads(Path({str(DEFAULT_PROTOCOL)!r}).read_text())['transitions']; "
        )
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            summary = run_comparison(
                bundle,
                _emitter("cpp", transitions),
                _emitter("rust", transitions),
                _sources(),
                10,
            )

        self.assertEqual(summary["status"], "passed")
        self.assertEqual(summary["transition_coverage"], "observed")
        counts = cast("dict[str, int]", summary["counts"])
        self.assertGreater(counts["numerical"], 3)

    def test_invalid_structured_result_is_reported_with_retained_stdout(self) -> None:
        """Schema failure reports identify the producer and canonical raw output."""
        invalid = ProcessSpec("rust", sys.executable, ("-c", "print('{}')"), ROOT)
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            summary = run_comparison(bundle, _emitter("cpp"), invalid, _sources(), 10)
            self.assertEqual((bundle / "raw" / "rust" / "stdout.txt").read_text(encoding="utf-8"), "{}\n")

        failure = _failures(summary)[0]
        self.assertEqual(failure["implementation"], "rust")
        self.assertEqual(failure["fixture"], "result")
        self.assertEqual(failure["field"], "schema")
        self.assertEqual(failure["rule"], {"kind": "schema"})
        self.assertEqual(failure["raw_artifact"], "raw/rust/stdout.txt")

    def test_nonzero_exit_is_reported_and_does_not_discard_stderr(self) -> None:
        """Producer failures retain exit status and diagnostic streams."""
        failing = ProcessSpec(
            "rust",
            sys.executable,
            ("-c", "import sys; print('rust failed', file=sys.stderr); raise SystemExit(7)"),
            ROOT,
        )
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            summary = run_comparison(bundle, _emitter("cpp"), failing, _sources(), 10)
            process = load_json(bundle / "raw" / "rust" / "process.json")
            stderr = (bundle / "raw" / "rust" / "stderr.txt").read_text(encoding="utf-8")

        self.assertEqual(summary["status"], "failed")
        self.assertEqual(process["exit_status"], 7)
        self.assertEqual(stderr, "rust failed\n")
        self.assertEqual(_failures(summary)[0]["field"], "exit_status")

    def test_reanalysis_requires_the_complete_canonical_artifact_inventory(self) -> None:
        """Removing manifest records cannot bypass retained-artifact verification."""
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            run_comparison(bundle, _emitter("cpp"), _emitter("rust"), _sources(), 10)
            manifest_path = bundle / "manifest.json"
            manifest = load_json(manifest_path)
            manifest["artifacts"] = []
            write_json(manifest_path, manifest)
            (bundle / "raw" / "rust" / "stderr.txt").unlink()

            with self.assertRaisesRegex(ValueError, "artifact inventory is incomplete"):
                reanalyze_comparison(bundle)

    def test_reanalysis_rejects_artifact_paths_outside_the_bundle(self) -> None:
        """A modified manifest cannot make reanalysis inspect arbitrary paths."""
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            run_comparison(bundle, _emitter("cpp"), _emitter("rust"), _sources(), 10)
            manifest_path = bundle / "manifest.json"
            manifest = load_json(manifest_path)
            artifacts = cast("list[dict[str, object]]", manifest["artifacts"])
            artifacts[0]["path"] = "../outside.txt"
            write_json(manifest_path, manifest)

            with self.assertRaisesRegex(ValueError, "not a canonical relative path"):
                reanalyze_comparison(bundle)

    def test_internal_failure_does_not_publish_a_partial_bundle(self) -> None:
        """A failed analysis cleans staging and leaves the requested path retryable."""
        with TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            bundle = root / "comparison"
            with (
                patch("scripts.compare_implementations.analyze_bundle", side_effect=OSError("analysis failed")),
                self.assertRaisesRegex(OSError, "analysis failed"),
            ):
                run_comparison(bundle, _emitter("cpp"), _emitter("rust"), _sources(), 10)

            self.assertFalse(bundle.exists())
            self.assertEqual(list(root.glob(f"{staging_directory_prefix(bundle)}*")), [])
            summary = run_comparison(bundle, _emitter("cpp"), _emitter("rust"), _sources(), 10)
            self.assertEqual(summary["status"], "passed")

    def test_existing_bundle_is_never_overwritten(self) -> None:
        """Every canonical comparison run must use a fresh output path."""
        with TemporaryDirectory() as temporary_directory:
            bundle = Path(temporary_directory) / "comparison"
            bundle.mkdir()
            sentinel = bundle / "summary.json"
            sentinel.write_text("previous\n", encoding="utf-8")

            with self.assertRaisesRegex(OutputDirectoryExistsError, "already exists"):
                run_comparison(bundle, _emitter("cpp"), _emitter("rust"), _sources(), 10)

            self.assertEqual(sentinel.read_text(encoding="utf-8"), "previous\n")

    def test_invalid_protocol_fails_before_creating_a_bundle_or_process(self) -> None:
        """Schema-invalid shared inputs cannot be forwarded to either producer."""
        with TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            invalid_protocol = root / "protocol.json"
            invalid_protocol.write_text("{}\n", encoding="utf-8")
            sources = _sources()
            sources["protocol"] = invalid_protocol
            bundle = root / "comparison"

            with self.assertRaisesRegex(ValueError, "schema violation"):
                run_comparison(bundle, _emitter("cpp"), _emitter("rust"), sources, 10)

            self.assertFalse(bundle.exists())


if __name__ == "__main__":
    unittest.main()
