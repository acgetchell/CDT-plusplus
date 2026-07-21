"""Tests for cross-platform vcpkg checkout validation."""

from __future__ import annotations

import hashlib
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path
from typing import override
from unittest import mock

from scripts import bootstrap_vcpkg


class BootstrapVcpkgTests(unittest.TestCase):
    """Exercise repository provenance and tool-integrity validation."""

    git: str

    @override
    def setUp(self) -> None:
        """Require Git for repository-backed fixtures."""
        git = shutil.which("git")
        if git is None:
            self.skipTest("git is required")
        self.git = git

    def _git(self, checkout: Path, *arguments: str) -> str:
        """Run Git in a test checkout and return stdout."""
        result = subprocess.run(  # noqa: S603 - self.git is resolved with shutil.which.
            [self.git, "-C", str(checkout), *arguments],
            check=True,
            capture_output=True,
            text=True,
        )
        return result.stdout.strip()

    def _make_checkout(self, root: Path) -> tuple[str, bootstrap_vcpkg.ToolSpec]:
        """Create a minimal official checkout with a hash-verifiable tool."""
        self._git(root, "init")
        self._git(root, "config", "user.email", "test@example.com")
        self._git(root, "config", "user.name", "CDT++ tests")
        metadata = root / "scripts" / "vcpkg-tool-metadata.txt"
        metadata.parent.mkdir(parents=True)
        metadata.write_text(f"VCPKG_TOOL_RELEASE_TAG={bootstrap_vcpkg.VCPKG_TOOL_RELEASE}\n", encoding="utf-8")
        tracked = root / "tracked.txt"
        tracked.write_text("tracked\n", encoding="utf-8")
        self._git(root, "add", "scripts/vcpkg-tool-metadata.txt", "tracked.txt")
        self._git(root, "commit", "-m", "fixture")
        self._git(root, "remote", "add", "origin", "https://github.com/microsoft/vcpkg.git")

        tool_content = b"fixture vcpkg tool"
        tool = root / "vcpkg-test"
        tool.write_bytes(tool_content)
        tool.chmod(0o755)
        tool_spec = bootstrap_vcpkg.ToolSpec("vcpkg-test", "sha256", hashlib.sha256(tool_content).hexdigest())
        return self._git(root, "rev-parse", "HEAD"), tool_spec

    @staticmethod
    def _valid_version(_executable: Path) -> str:
        """Return version output accepted by the validator."""
        return f"vcpkg package management program version {bootstrap_vcpkg.VCPKG_TOOL_RELEASE}-fixture"

    def test_accepts_official_clean_checkout_with_untracked_metadata(self) -> None:
        """Action-owned untracked metadata does not invalidate tracked sources."""
        with tempfile.TemporaryDirectory() as temp_dir:
            checkout = Path(temp_dir)
            baseline, tool_spec = self._make_checkout(checkout)
            (checkout / "vcpkgLastBuiltCommitId").write_text(baseline, encoding="utf-8")

            bootstrap_vcpkg.validate_checkout(
                checkout,
                baseline,
                git=self.git,
                tool_spec=tool_spec,
                version_reader=self._valid_version,
            )

    def test_rejects_tracked_modification_with_path(self) -> None:
        """Tracked changes fail and identify the modified file."""
        with tempfile.TemporaryDirectory() as temp_dir:
            checkout = Path(temp_dir)
            baseline, tool_spec = self._make_checkout(checkout)
            (checkout / "tracked.txt").write_text("modified\n", encoding="utf-8")

            with self.assertRaisesRegex(bootstrap_vcpkg.BootstrapError, r"modified[\s\S]*tracked\.txt"):
                bootstrap_vcpkg.validate_checkout(
                    checkout,
                    baseline,
                    git=self.git,
                    tool_spec=tool_spec,
                    version_reader=self._valid_version,
                )

    def test_rejects_nonofficial_origin(self) -> None:
        """A checkout from another repository is rejected."""
        with tempfile.TemporaryDirectory() as temp_dir:
            checkout = Path(temp_dir)
            baseline, tool_spec = self._make_checkout(checkout)
            self._git(checkout, "remote", "set-url", "origin", "https://example.com/vcpkg.git")

            with self.assertRaisesRegex(bootstrap_vcpkg.BootstrapError, "origin is not microsoft/vcpkg"):
                bootstrap_vcpkg.validate_checkout(
                    checkout,
                    baseline,
                    git=self.git,
                    tool_spec=tool_spec,
                    version_reader=self._valid_version,
                )

    def test_rejects_wrong_baseline(self) -> None:
        """A clean checkout at another commit is rejected."""
        with tempfile.TemporaryDirectory() as temp_dir:
            checkout = Path(temp_dir)
            _baseline, tool_spec = self._make_checkout(checkout)

            with self.assertRaisesRegex(bootstrap_vcpkg.BootstrapError, "not pinned baseline"):
                bootstrap_vcpkg.validate_checkout(
                    checkout,
                    "0" * 40,
                    git=self.git,
                    tool_spec=tool_spec,
                    version_reader=self._valid_version,
                )

    def test_rejects_tool_digest_mismatch(self) -> None:
        """An executable that differs from its trusted digest is rejected."""
        with tempfile.TemporaryDirectory() as temp_dir:
            checkout = Path(temp_dir)
            baseline, tool_spec = self._make_checkout(checkout)
            invalid_spec = bootstrap_vcpkg.ToolSpec(tool_spec.executable_name, tool_spec.hash_algorithm, "0" * 64)

            with self.assertRaisesRegex(bootstrap_vcpkg.BootstrapError, "digest does not match"):
                bootstrap_vcpkg.validate_checkout(
                    checkout,
                    baseline,
                    git=self.git,
                    tool_spec=invalid_spec,
                    version_reader=self._valid_version,
                )

    def test_selects_windows_tool_hashes_by_architecture(self) -> None:
        """Windows x64 and ARM64 select their pinned release assets."""
        x64 = bootstrap_vcpkg.select_tool_spec({}, system_name="Windows", machine_name="AMD64")
        arm64 = bootstrap_vcpkg.select_tool_spec({}, system_name="Windows", machine_name="ARM64")

        self.assertEqual(x64.executable_name, "vcpkg.exe")
        self.assertEqual(x64.expected_digest, bootstrap_vcpkg.WINDOWS_TOOL_SHA256["amd64"])
        self.assertEqual(arm64.expected_digest, bootstrap_vcpkg.WINDOWS_TOOL_SHA256["arm64"])

    def test_finds_repository_root_from_nested_working_directory(self) -> None:
        """An installed entry point locates the checkout independently of its module path."""
        with tempfile.TemporaryDirectory() as temp_dir:
            repository_root = Path(temp_dir)
            nested_directory = repository_root / "nested" / "directory"
            nested_directory.mkdir(parents=True)
            (repository_root / "vcpkg.json").write_text("{}\n", encoding="utf-8")

            self.assertEqual(
                bootstrap_vcpkg._find_repository_root(nested_directory),  # noqa: SLF001 - focused repository-discovery contract.
                repository_root.resolve(),
            )

    def test_empty_cache_override_uses_repository_cache(self) -> None:
        """An empty cache environment value behaves like an unset override."""
        with (
            tempfile.TemporaryDirectory() as temp_dir,
            mock.patch.dict(
                bootstrap_vcpkg.os.environ,
                {"CDT_VCPKG_CACHE_DIR": ""},
            ),
        ):
            repository_root = Path(temp_dir)
            (repository_root / "vcpkg.json").write_text(
                '{"builtin-baseline": "0000000000000000000000000000000000000000"}\n',
                encoding="utf-8",
            )

            with self.assertRaises(bootstrap_vcpkg.BootstrapError) as raised:
                bootstrap_vcpkg.bootstrap_vcpkg(repository_root, check_only=True)
            expected_cache = (repository_root / ".cache" / "vcpkg").resolve()
            self.assertIn(str(expected_cache), str(raised.exception))


if __name__ == "__main__":
    unittest.main()
