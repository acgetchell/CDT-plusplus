"""Tests for the portable initializer optimization support script."""

from __future__ import annotations

import argparse
import unittest
from pathlib import Path

from scripts.optimize_initialize import _initializer_binary, _initializer_command, _parse_args, _parse_initializer_output, _parse_seed


class OptimizeInitializeTests(unittest.TestCase):
    """Verify dependency-free parsing and executable discovery."""

    def test_initializer_binary_uses_windows_suffix(self) -> None:
        """Windows reference builds end in initialize.exe."""
        root = Path("checkout")
        self.assertEqual(
            _initializer_binary(root, "win32"),
            root / "out" / "build" / "reference" / "src" / "initialize.exe",
        )

    def test_initializer_binary_has_no_unix_suffix(self) -> None:
        """Unix reference builds retain the extensionless executable name."""
        root = Path("checkout")
        expected = root / "out" / "build" / "reference" / "src" / "initialize"
        self.assertEqual(_initializer_binary(root, "darwin"), expected)
        self.assertEqual(_initializer_binary(root, "linux"), expected)

    def test_initializer_seed_defaults_to_replay_value(self) -> None:
        """The sweep is reproducible without additional seed configuration."""
        self.assertEqual(_parse_args([]).seed, 92)

    def test_initializer_seed_rejects_values_outside_uint64(self) -> None:
        """Invalid seeds fail before any online experiment is created."""
        with self.assertRaises(argparse.ArgumentTypeError):
            _parse_seed("-1")
        with self.assertRaises(argparse.ArgumentTypeError):
            _parse_seed("18446744073709551616")

    def test_initializer_output_is_parsed(self) -> None:
        """The sweep extracts both the final size and volume profile."""
        output = """Timeslice 1 has 12 spacelike faces.
Timeslice 2 has 24 spacelike faces.
Final number of simplices: 92"""
        self.assertEqual(_parse_initializer_output(output), (92, [(1, 12), (2, 24)]))

    def test_initializer_output_requires_final_simplex_count(self) -> None:
        """The sweep rejects output without the final triangulation size."""
        output = "Timeslice 1 has 12 spacelike faces."
        with self.assertRaisesRegex(RuntimeError, "did not report the final number of simplices"):
            _parse_initializer_output(output)

    def test_initializer_output_requires_volume_profile(self) -> None:
        """The sweep rejects output without any timeslice volumes."""
        output = "Final number of simplices: 92"
        with self.assertRaisesRegex(RuntimeError, "did not contain a timeslice volume profile"):
            _parse_initializer_output(output)

    def test_initializer_output_rejects_malformed_volume_entries(self) -> None:
        """A partial profile is not accepted when one recognized row is malformed."""
        output = """Timeslice 1 has 12 spacelike faces.
Timeslice two has 24 spacelike faces.
Final number of simplices: 92"""
        with self.assertRaisesRegex(RuntimeError, "malformed timeslice volume"):
            _parse_initializer_output(output)

    def test_initializer_command_uses_the_recorded_seed(self) -> None:
        """Every parameter pair forwards its replay seed to initialize."""
        command = _initializer_command(
            Path("initialize"),
            {"simplices": 12000, "foliations": 12, "seed": 92},
            initial_radius=1,
            radial_factor=1.5,
        )
        self.assertEqual(command[-2:], ["--seed", "92"])


if __name__ == "__main__":
    unittest.main()
