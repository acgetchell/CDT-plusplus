"""Regression tests for complete reference-package regeneration."""

from __future__ import annotations

import unittest

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


if __name__ == "__main__":
    unittest.main()
