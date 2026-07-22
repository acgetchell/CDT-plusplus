"""Tests for deterministic release-tag creation."""

from __future__ import annotations

import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path
from typing import override

from scripts import tag_release


class TagReleaseTests(unittest.TestCase):
    """Exercise changelog parsing and isolated Git tag creation."""

    git: str

    @override
    def setUp(self) -> None:
        """Require Git for repository-backed fixtures."""
        git = shutil.which("git")
        if git is None:
            self.skipTest("git is required")
        self.git = git

    def _git(self, root: Path, *arguments: str) -> str:
        """Run Git in a temporary fixture repository."""
        result = subprocess.run(  # noqa: S603 - self.git is resolved with shutil.which.
            [self.git, "-C", str(root), *arguments],
            check=True,
            capture_output=True,
            text=True,
            encoding="utf-8",
        )
        return result.stdout.strip()

    def _make_repository(self, root: Path, *, release_entry: str = "- Preserve causal invariants.") -> None:
        """Create a clean repository containing two changelog sections."""
        self._git(root, "init")
        self._git(root, "config", "user.email", "test@example.com")
        self._git(root, "config", "user.name", "CDT++ tests")
        (root / "CHANGELOG.md").write_text(
            f"# Changelog\n\n## [1.0.0-rc1] - 2026-07-21\n\n### Fixed\n\n{release_entry}\n\n## [0.1.8] - 2020-01-01\n\n### Changed\n\n- Historical release.\n",
            encoding="utf-8",
        )
        self._git(root, "add", "CHANGELOG.md")
        self._git(root, "commit", "-m", "fixture")

    def test_accepts_stable_and_release_candidate_semver(self) -> None:
        """Stable and prerelease tags use the same validation path."""
        for tag in ("v1.0.0", "v1.0.0-rc1", "v1.0.0-rc.2"):
            with self.subTest(tag=tag):
                tag_release.validate_semver(tag)

    def test_rejects_missing_prefix_and_incomplete_versions(self) -> None:
        """Ambiguous tag spellings fail before Git is invoked."""
        for tag in ("1.0.0", "v1.0", "v01.0.0"):
            with self.subTest(tag=tag), self.assertRaises(tag_release.ReleaseTagError):
                tag_release.validate_semver(tag)

    def test_extracts_only_the_requested_changelog_body(self) -> None:
        """The next release heading terminates the selected tag message."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._make_repository(root)

            section = tag_release.extract_changelog_section(root / "CHANGELOG.md", "1.0.0-rc1")

        self.assertEqual(section, "### Fixed\n\n- Preserve causal invariants.")

    def test_dry_run_preserves_repository_refs(self) -> None:
        """Preflight validates and previews without creating the tag."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._make_repository(root)

            tag_release.create_tag("v1.0.0-rc1", root=root, dry_run=True)

            self.assertEqual(self._git(root, "tag", "--list"), "")

    def test_creates_annotated_tag_from_changelog_section(self) -> None:
        """The annotation contains the selected release body verbatim."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._make_repository(root)

            tag_release.create_tag("v1.0.0-rc1", root=root)

            annotation = self._git(root, "tag", "--list", "--format=%(contents)", "v1.0.0-rc1")

        self.assertEqual(annotation, "### Fixed\n\n- Preserve causal invariants.")

    def test_rejects_dirty_worktree(self) -> None:
        """A tag cannot silently omit uncommitted release changes."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._make_repository(root)
            (root / "untracked.txt").write_text("not released\n", encoding="utf-8")

            with self.assertRaisesRegex(tag_release.ReleaseTagError, "worktree must be clean"):
                tag_release.create_tag("v1.0.0-rc1", root=root)

    def test_rejects_existing_tag(self) -> None:
        """Published identities are never replaced by the helper."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._make_repository(root)
            tag_release.create_tag("v1.0.0-rc1", root=root)

            with self.assertRaisesRegex(tag_release.ReleaseTagError, "already exists"):
                tag_release.create_tag("v1.0.0-rc1", root=root)

    def test_large_section_uses_stable_changelog_reference(self) -> None:
        """Oversized annotations remain below GitHub's limit and actionable."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._make_repository(root, release_entry="x" * (tag_release.GITHUB_TAG_ANNOTATION_LIMIT + 1))

            tag_release.create_tag("v1.0.0-rc1", root=root)

            annotation = self._git(root, "tag", "--list", "--format=%(contents)", "v1.0.0-rc1")

        self.assertLess(len(annotation.encode("utf-8")), tag_release.GITHUB_TAG_ANNOTATION_LIMIT)
        self.assertIn("/blob/v1.0.0-rc1/CHANGELOG.md", annotation)


if __name__ == "__main__":
    unittest.main()
