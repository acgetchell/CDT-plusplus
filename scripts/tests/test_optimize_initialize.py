"""Tests for the local initializer optimization support script."""

import argparse
import hashlib
import json
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import Mock, patch

from scripts.experiment_artifacts import staging_directory_prefix
from scripts.optimize_initialize import (
    PARAMETER_PAIRS,
    _experiment_provenance,
    _initializer_binary,
    _initializer_command,
    _parse_args,
    _parse_initializer_output,
    _parse_seed,
    _run_parameter_sweep,
    _SweepServices,
)


class OptimizeInitializeTests(unittest.TestCase):
    """Verify local parsing, execution, artifact, and provenance contracts."""

    def test_initializer_binary_uses_platform_suffix(self) -> None:
        """Only Windows reference builds append .exe."""
        root = Path("checkout")
        self.assertEqual(
            _initializer_binary(root, "win32"),
            root / "out" / "build" / "reference" / "src" / "initialize.exe",
        )
        expected = root / "out" / "build" / "reference" / "src" / "initialize"
        self.assertEqual(_initializer_binary(root, "darwin"), expected)
        self.assertEqual(_initializer_binary(root, "linux"), expected)

    def test_initializer_seed_defaults_to_replay_value(self) -> None:
        """The sweep is reproducible without hosted configuration."""
        self.assertEqual(_parse_args([]).seed, 92)

    def test_initializer_seed_rejects_values_outside_uint64(self) -> None:
        """Invalid seeds fail before an initializer process starts."""
        for value in ("-1", "18446744073709551616", "invalid"):
            with self.subTest(value=value), self.assertRaises(argparse.ArgumentTypeError):
                _parse_seed(value)

    def test_initializer_output_is_parsed(self) -> None:
        """The sweep extracts both final size and volume profile."""
        output = """Timeslice 1 has 12 spacelike faces.
Timeslice 2 has 24 spacelike faces.
Final number of simplices: 92"""
        self.assertEqual(_parse_initializer_output(output), (92, [(1, 12), (2, 24)]))

    def test_initializer_output_rejects_missing_or_malformed_fields(self) -> None:
        """Incomplete stdout cannot become a canonical run record."""
        cases = (
            ("Timeslice 1 has 12 spacelike faces.", "did not report"),
            ("Final number of simplices: 92", "did not contain"),
            ("Timeslice two has 12 spacelike faces.\nFinal number of simplices: 92", "malformed"),
        )
        for output, message in cases:
            with self.subTest(output=output), self.assertRaisesRegex(RuntimeError, message):
                _parse_initializer_output(output)

    def test_initializer_command_uses_the_recorded_seed(self) -> None:
        """Every parameter pair forwards its replay seed."""
        command = _initializer_command(
            Path("initialize"),
            {"simplices": 12000, "foliations": 12, "seed": 92},
            initial_radius=1,
            foliation_spacing=1.5,
        )
        self.assertEqual(
            command,
            ["initialize", "-s", "-n", "12000", "-t", "12", "-i", "1", "-f", "1.5", "--seed", "92"],
        )

    def test_parameter_sweep_records_dependency_free_local_artifacts(self) -> None:
        """Every pair retains JSON, raw stdout, and a portable table."""
        runner = Mock(
            return_value="""Timeslice 1 has 12 spacelike faces.
Timeslice 2 has 24 spacelike faces.
Final number of simplices: 12000"""
        )
        provenance = {"initializer": {"sha256": "initializer-digest"}}
        with TemporaryDirectory() as temporary_directory, patch("builtins.print"):
            output_directory = Path(temporary_directory) / "run"
            _run_parameter_sweep(
                Path("initialize"),
                92,
                output_directory,
                provenance,
                _SweepServices(initializer_runner=runner),
            )

            run_directory = output_directory / "radius-1-spacing-1"
            run_record = json.loads((run_directory / "run.json").read_text(encoding="utf-8"))
            self.assertEqual(run_record["provenance"], provenance)
            self.assertEqual((run_directory / "volume-profile.tsv").read_text(encoding="utf-8"), "timeslice\tvolume\n1\t12\n2\t24\n")
            for name in ("configuration", "stdout", "volume_profile"):
                with self.subTest(artifact=name):
                    record = run_record["artifacts"][name]
                    artifact = run_directory / record["path"]
                    self.assertEqual(record["bytes"], artifact.stat().st_size)
                    self.assertEqual(record["sha256"], hashlib.sha256(artifact.read_bytes()).hexdigest())
            run_directories = {path for path in output_directory.glob("radius-*-spacing-*") if path.is_dir()}
            self.assertEqual(len(run_directories), len(PARAMETER_PAIRS))

    def test_parameter_sweep_failure_removes_partial_artifacts(self) -> None:
        """A failed initializer cannot publish an incomplete sweep."""
        with TemporaryDirectory() as temporary_directory, patch("builtins.print"):
            root = Path(temporary_directory)
            output_directory = root / "run"
            services = _SweepServices(initializer_runner=Mock(side_effect=RuntimeError("initializer failed")))
            with self.assertRaisesRegex(RuntimeError, "initializer failed"):
                _run_parameter_sweep(Path("initialize"), 92, output_directory, {}, services)

            self.assertFalse(output_directory.exists())
            self.assertEqual(list(root.glob(f"{staging_directory_prefix(output_directory)}*")), [])

    def test_parameter_sweep_preserves_an_existing_run(self) -> None:
        """A replay must use a new output path instead of mixing generations."""
        with TemporaryDirectory() as temporary_directory:
            output_directory = Path(temporary_directory) / "run"
            output_directory.mkdir()
            sentinel = output_directory / "run.json"
            sentinel.write_text("previous\n", encoding="utf-8")
            runner = Mock()
            services = _SweepServices(initializer_runner=runner)
            with self.assertRaisesRegex(ValueError, "already exists"):
                _run_parameter_sweep(Path("initialize"), 92, output_directory, {}, services)

            self.assertEqual(sentinel.read_text(encoding="utf-8"), "previous\n")
            runner.assert_not_called()

    def test_experiment_provenance_hashes_binary_and_source_state(self) -> None:
        """The local record identifies the executable and dirty source state."""
        with TemporaryDirectory() as temporary_directory:
            repository_root = Path(temporary_directory)
            initialize_binary = repository_root / "out" / "build" / "reference" / "src" / "initialize"
            initialize_binary.parent.mkdir(parents=True)
            initialize_binary.write_bytes(b"exact executable")
            git_binary = repository_root / "git"
            git_binary.write_bytes(b"git executable")

            with (
                patch("scripts.optimize_initialize.shutil.which", return_value=str(git_binary)),
                patch("scripts.optimize_initialize.qx", side_effect=["abc123\n", " M README.md\n", b"tracked diff"]),
            ):
                provenance = _experiment_provenance(repository_root, initialize_binary)

            self.assertEqual(
                provenance["initializer"],
                {
                    "bytes": len(b"exact executable"),
                    "path": "out/build/reference/src/initialize",
                    "sha256": hashlib.sha256(b"exact executable").hexdigest(),
                },
            )
            self.assertEqual(
                provenance["repository"],
                {
                    "commit": "abc123",
                    "dirty": True,
                    "tracked_diff_sha256": hashlib.sha256(b"tracked diff").hexdigest(),
                },
            )


if __name__ == "__main__":
    unittest.main()
