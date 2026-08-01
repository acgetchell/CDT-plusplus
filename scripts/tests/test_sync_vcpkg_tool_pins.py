"""Tests for atomic vcpkg tool-pin synchronization."""

from __future__ import annotations

import hashlib
import tempfile
import unittest
from pathlib import Path

from scripts import sync_vcpkg_tool_pins


class SyncVcpkgToolPinsTests(unittest.TestCase):
    """Exercise exact-baseline fetching and failure-safe source updates."""

    baseline = "1" * 40
    release = "2026-07-27"
    original_bootstrap = (
        f'VCPKG_TOOL_RELEASE = "2026-07-13"\nUNCHANGED = "preserve me"\nWINDOWS_TOOL_SHA256 = {{\n    "amd64": "{"2" * 64}",\n    "arm64": "{"3" * 64}",\n}}\n'
    )

    def _make_repository(self, root: Path) -> Path:
        """Create the manifest and bootstrap source owned by the synchronizer."""
        (root / "vcpkg.json").write_text(f'{{"builtin-baseline": "{self.baseline}"}}\n', encoding="utf-8")
        scripts = root / "scripts"
        scripts.mkdir()
        bootstrap = scripts / "bootstrap_vcpkg.py"
        bootstrap.write_text(self.original_bootstrap, encoding="utf-8")
        return bootstrap

    def test_syncs_release_and_both_hashes_from_exact_baseline(self) -> None:
        """The manifest commit selects metadata and both official Windows assets."""
        amd64 = b"amd64 executable"
        arm64 = b"arm64 executable"
        requested: list[str] = []

        def download(url: str) -> bytes:
            requested.append(url)
            if url == sync_vcpkg_tool_pins.METADATA_URL.format(baseline=self.baseline):
                return f"VCPKG_TOOL_RELEASE_TAG={self.release}\n".encode()
            if url.endswith("/vcpkg-arm64.exe"):
                return arm64
            if url.endswith("/vcpkg.exe"):
                return amd64
            message = f"Unexpected URL: {url}"
            raise AssertionError(message)

        with tempfile.TemporaryDirectory() as temp_dir:
            repository_root = Path(temp_dir)
            bootstrap = self._make_repository(repository_root)

            pins = sync_vcpkg_tool_pins.sync_vcpkg_tool_pins(repository_root, download=download)

            updated = bootstrap.read_text(encoding="utf-8")

        self.assertEqual(pins.release, self.release)
        self.assertEqual(
            requested,
            [
                sync_vcpkg_tool_pins.METADATA_URL.format(baseline=self.baseline),
                sync_vcpkg_tool_pins.WINDOWS_TOOL_URLS["amd64"].format(release=self.release),
                sync_vcpkg_tool_pins.WINDOWS_TOOL_URLS["arm64"].format(release=self.release),
            ],
        )
        self.assertIn(f'VCPKG_TOOL_RELEASE = "{self.release}"', updated)
        self.assertIn(hashlib.sha256(amd64).hexdigest(), updated)
        self.assertIn(hashlib.sha256(arm64).hexdigest(), updated)
        self.assertIn('UNCHANGED = "preserve me"', updated)

    def test_download_failure_preserves_existing_pins(self) -> None:
        """A partial asset download cannot publish a partial source update."""
        calls = 0

        def download(_url: str) -> bytes:
            nonlocal calls
            calls += 1
            if calls == 1:
                return f"VCPKG_TOOL_RELEASE_TAG={self.release}\n".encode()
            if calls == 2:
                return b"amd64 executable"
            message = "simulated arm64 download failure"
            raise sync_vcpkg_tool_pins.PinSyncError(message)

        with tempfile.TemporaryDirectory() as temp_dir:
            repository_root = Path(temp_dir)
            bootstrap = self._make_repository(repository_root)

            with self.assertRaisesRegex(sync_vcpkg_tool_pins.PinSyncError, "arm64 download failure"):
                sync_vcpkg_tool_pins.sync_vcpkg_tool_pins(repository_root, download=download)

            self.assertEqual(bootstrap.read_text(encoding="utf-8"), self.original_bootstrap)

    def test_invalid_metadata_preserves_existing_pins(self) -> None:
        """Malformed upstream metadata is rejected before any source update."""
        with tempfile.TemporaryDirectory() as temp_dir:
            repository_root = Path(temp_dir)
            bootstrap = self._make_repository(repository_root)

            with self.assertRaisesRegex(sync_vcpkg_tool_pins.PinSyncError, "Invalid VCPKG_TOOL_RELEASE_TAG"):
                sync_vcpkg_tool_pins.sync_vcpkg_tool_pins(
                    repository_root,
                    download=lambda _url: b"VCPKG_TOOL_RELEASE_TAG=not-a-release\n",
                )

            self.assertEqual(bootstrap.read_text(encoding="utf-8"), self.original_bootstrap)


if __name__ == "__main__":
    unittest.main()
