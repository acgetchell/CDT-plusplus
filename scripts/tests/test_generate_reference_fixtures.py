"""Regression tests for complete reference-package regeneration."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts import generate_reference_fixtures as generator


class ReferenceFixtureGenerationTests(unittest.TestCase):
    """Exercise regeneration metadata before any artifacts are published."""

    def test_recorded_timestamp_is_canonical_utc(self) -> None:
        """Equivalent explicit offsets produce one stable manifest spelling."""
        self.assertEqual(
            generator.canonical_timestamp("2026-07-25T01:02:03-07:00"),
            "2026-07-25T08:02:03Z",
        )

    def test_recorded_timestamp_requires_an_offset(self) -> None:
        """An ambiguous local timestamp cannot enter archival provenance."""
        with self.assertRaisesRegex(ValueError, "UTC offset"):
            generator.canonical_timestamp("2026-07-25T08:02:03")

    def test_platform_identity_must_match_manifest_names(self) -> None:
        """A Linux run cannot silently overwrite macOS-arm64 manifests."""
        fixture = {
            "implementation": {
                "operating_system": "Linux",
                "hardware": "x86_64",
            }
        }
        scaling = [
            {
                "build.system": "Linux",
                "build.processor": "x86_64",
            }
        ]

        with self.assertRaisesRegex(RuntimeError, "macOS-arm64-specific"):
            generator.validate_platform_identity(fixture, scaling)

    def test_platform_identity_accepts_the_recorded_host(self) -> None:
        """The macOS-arm64 reference host regenerates without objection."""
        fixture = {
            "implementation": {
                "operating_system": "Darwin",
                "hardware": "arm64",
            }
        }
        scaling = [
            {
                "build.system": "Darwin",
                "build.processor": "arm64",
            }
        ]

        generator.validate_platform_identity(fixture, scaling)

    def test_command_records_must_match_generated_artifacts(self) -> None:
        """Argv arrays cannot be attached to a mismatched command list."""
        with self.assertRaisesRegex(ValueError, "does not match the generated artifacts"):
            generator.record_commands([{"id": "fixture", "artifacts": []}], [])

    def test_producer_paths_must_use_the_canonical_layout(self) -> None:
        """A noncanonical producer is rejected with its option and path."""
        reference_commands = [
            ["build/custom/CDT_reference_fixture"],
            [generator.CANONICAL_PRODUCER_PATHS["--cdt-binary"]],
            [generator.CANONICAL_PRODUCER_PATHS["--initialize-binary"]],
        ]
        scaling_commands = [[generator.CANONICAL_PRODUCER_PATHS["--benchmark-binary"]]]

        with self.assertRaisesRegex(ValueError, r"--fixture-binary.*'build/custom/CDT_reference_fixture'"):
            generator.validate_producer_paths(reference_commands, scaling_commands)

    def test_cmake_version_comes_from_the_configured_builds(self) -> None:
        """Manifest provenance uses the CMake recorded in each build cache."""
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            cmake = root / "cmake"
            cmake.touch()
            cache_paths = (root / "reference-cache.txt", root / "parallel-cache.txt")
            for cache_path in cache_paths:
                cache_path.write_text(f"CMAKE_COMMAND:INTERNAL={cmake}\n", encoding="utf-8")

            with mock.patch.object(
                generator,
                "run_command",
                return_value="cmake version 4.4.0\n",
            ) as run_command:
                self.assertEqual(generator.configured_cmake_version(cache_paths), "4.4.0")

        self.assertEqual(
            run_command.call_args_list,
            [
                mock.call([str(cmake), "--version"], timeout=30),
                mock.call([str(cmake), "--version"], timeout=30),
            ],
        )

    def test_manifest_builders_discard_stale_provenance(self) -> None:
        """Templates cannot carry old verification or unreported tools forward."""
        fixture = generator.load_json(generator.ROOT / "reference" / "raw" / "v1" / "cpp-reference.json")
        scaling = [
            generator.parse_key_value_payload(
                (generator.ROOT / "reference" / "raw" / "v1" / f"scaling-threads-{threads}.txt").read_text(encoding="utf-8"),
                f"scaling-threads-{threads}.txt",
            )
            for threads in (1, 2, 4)
        ]
        metadata = generator.RegenerationMetadata("a" * 40, "2026-07-25T08:02:03Z", "9.9.9")
        reference_commands = [[path] for path in list(generator.CANONICAL_PRODUCER_PATHS.values())[:3]]
        scaling_commands = [[generator.CANONICAL_PRODUCER_PATHS["--benchmark-binary"]] for _threads in (1, 2, 4)]

        with mock.patch.object(generator, "refresh_artifacts"):
            reference_manifest = generator.make_reference_manifest({}, fixture, metadata, reference_commands)
            scaling_manifest = generator.make_scaling_manifest({}, scaling, metadata, scaling_commands)

        schema = generator.load_json(generator.ROOT / "reference" / "schema" / "run-manifest-v1.schema.json")
        for manifest in (reference_manifest, scaling_manifest):
            self.assertNotIn("verification", manifest)
            self.assertEqual(manifest["toolchain"]["cmake"], "9.9.9")
            generator.record_regeneration_verification(manifest, metadata)
            self.assertEqual(manifest["verification"]["producer_regeneration"]["source_revision"], "a" * 40)
            generator.validate_document(manifest, schema, generator.ROOT / "generated-manifest.json")
        self.assertNotIn("mpfr", reference_manifest["toolchain"])


if __name__ == "__main__":
    unittest.main()
