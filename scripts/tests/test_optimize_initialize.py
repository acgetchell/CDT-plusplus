"""Tests for the portable initializer optimization support script."""

from __future__ import annotations

import unittest
from pathlib import Path

from scripts.optimize_initialize import _initializer_binary, _parse_initializer_output


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


if __name__ == "__main__":
    unittest.main()
