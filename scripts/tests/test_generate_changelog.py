"""Tests for deterministic changelog generation."""

from __future__ import annotations

import subprocess
import tempfile
import unittest
from datetime import date
from pathlib import Path
from unittest import mock

from scripts import generate_changelog


class GenerateChangelogTests(unittest.TestCase):
    """Exercise release-date normalization and atomic output publication."""

    def test_applies_explicit_release_date_to_target_heading(self) -> None:
        """Metadata, rather than the process clock, owns the release date."""
        changelog = "# Changelog\n\n## [1.0.0-rc1] - 2099-01-01\n\n- Fixed.\n"

        updated = generate_changelog.apply_release_date(changelog, "1.0.0-rc1", date(2026, 7, 21))

        self.assertIn("## [1.0.0-rc1] - 2026-07-21", updated)
        self.assertNotIn("2099-01-01", updated)

    def test_rejects_missing_target_heading(self) -> None:
        """Malformed tool output cannot replace the prior artifact."""
        with self.assertRaisesRegex(generate_changelog.ChangelogError, "found 0"):
            generate_changelog.apply_release_date("# Changelog\n", "1.0.0-rc1", date(2026, 7, 21))

    @mock.patch("scripts.generate_changelog.run_support_command")
    @mock.patch("scripts.generate_changelog.run_git_command")
    @mock.patch("scripts.generate_changelog.release_check.check_release_inputs")
    def test_malformed_output_preserves_prior_changelog(
        self,
        metadata: mock.Mock,
        git: mock.Mock,
        command: mock.Mock,
    ) -> None:
        """Validation completes before the tracked artifact is replaced."""
        metadata.return_value = ("1.0.0-rc1", date(2026, 7, 21))
        git.side_effect = [
            subprocess.CompletedProcess(args=["git"], returncode=0, stdout="0.1.8\n", stderr=""),
            subprocess.CompletedProcess(args=["git"], returncode=0, stdout="", stderr=""),
        ]
        command.return_value = subprocess.CompletedProcess(
            args=["git-cliff"],
            returncode=0,
            stdout="# Changelog without the requested heading\n",
            stderr="",
        )
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "cliff.toml").write_text("[changelog]\n", encoding="utf-8")
            output = root / "CHANGELOG.md"
            output.write_text("prior valid artifact\n", encoding="utf-8")

            with self.assertRaises(generate_changelog.ChangelogError):
                generate_changelog.generate_changelog("v1.0.0-rc1", root=root, output=output)

            self.assertEqual(output.read_text(encoding="utf-8"), "prior valid artifact\n")

    @mock.patch("scripts.generate_changelog.run_support_command")
    @mock.patch("scripts.generate_changelog.run_git_command")
    @mock.patch("scripts.generate_changelog.release_check.check_release_inputs")
    def test_generates_validated_changelog_atomically(
        self,
        metadata: mock.Mock,
        git: mock.Mock,
        command: mock.Mock,
    ) -> None:
        """A complete validated rendering replaces the requested output."""
        metadata.return_value = ("1.0.0-rc1", date(2026, 7, 21))
        command.return_value = subprocess.CompletedProcess(
            args=["git-cliff"],
            returncode=0,
            stdout="# Changelog\n\n## [1.0.0-rc1] - 2099-01-01\n\n- Fixed.\n\n",
            stderr="",
        )
        git.side_effect = [
            subprocess.CompletedProcess(args=["git"], returncode=0, stdout="v0.9.0-rc1\n0.1.8\n", stderr=""),
            subprocess.CompletedProcess(args=["git"], returncode=0, stdout="", stderr=""),
        ]
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "cliff.toml").write_text("[changelog]\n", encoding="utf-8")
            output = root / "CHANGELOG.md"
            output.write_text("prior valid artifact\n", encoding="utf-8")

            generated = generate_changelog.generate_changelog("v1.0.0-rc1", root=root, output=output)

            text = generated.read_text(encoding="utf-8")

        self.assertEqual(text, "# Changelog\n\n## [1.0.0-rc1] - 2026-07-21\n\n- Fixed.\n")
        environment = command.call_args.kwargs["environment"]
        self.assertEqual(environment["GIT_CLIFF_OFFLINE"], "true")
        self.assertEqual(command.call_args.args[1][-1], "0.1.8..HEAD")

    @mock.patch("scripts.generate_changelog.run_support_command")
    @mock.patch("scripts.generate_changelog.run_git_command")
    @mock.patch("scripts.generate_changelog.release_check.check_release_inputs")
    def test_ignores_prior_release_tag_on_head(
        self,
        metadata: mock.Mock,
        git: mock.Mock,
        command: mock.Mock,
    ) -> None:
        """A prior RC on HEAD cannot override the requested synthetic release."""
        metadata.return_value = ("1.0.0-rc2", date(2026, 7, 22))
        git.side_effect = [
            subprocess.CompletedProcess(args=["git"], returncode=0, stdout="v1.0.0-rc1\n0.1.8\n", stderr=""),
            subprocess.CompletedProcess(args=["git"], returncode=0, stdout="v1.0.0-rc1\nv1.0.0-rc2\n", stderr=""),
        ]
        command.return_value = subprocess.CompletedProcess(
            args=["git-cliff"],
            returncode=0,
            stdout="# Changelog\n\n## [1.0.0-rc2] - 2099-01-01\n\n- Fixed.\n",
            stderr="",
        )
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "cliff.toml").write_text("[changelog]\n", encoding="utf-8")

            generated = generate_changelog.generate_changelog("v1.0.0-rc2", root=root, output=root / "CHANGELOG.md")

            text = generated.read_text(encoding="utf-8")

        self.assertIn("## [1.0.0-rc2] - 2026-07-22", text)
        self.assertEqual(
            command.call_args.args[1],
            [
                "--config",
                str(root.resolve() / "cliff.toml"),
                "--tag",
                "v1.0.0-rc2",
                "--ignore-tags",
                r"^(?:v1\.0\.0\-rc1)$",
                "0.1.8..HEAD",
            ],
        )

    @mock.patch("scripts.generate_changelog.release_check.check_release_inputs")
    def test_rejects_tag_metadata_mismatch_before_running_git_cliff(self, metadata: mock.Mock) -> None:
        """The requested release cannot diverge from structured metadata."""
        metadata.return_value = ("1.0.0", date(2026, 7, 21))
        with tempfile.TemporaryDirectory() as temp_dir, self.assertRaisesRegex(generate_changelog.ChangelogError, "does not match"):
            generate_changelog.generate_changelog("v1.0.0-rc1", root=Path(temp_dir))

    @mock.patch("scripts.generate_changelog.run_git_command")
    def test_previous_tag_ignores_prereleases_and_non_semver(self, git: mock.Mock) -> None:
        """Release candidates compare against the newest stable release."""
        git.return_value = subprocess.CompletedProcess(
            args=["git"],
            returncode=0,
            stdout="nightly\nv1.0.0-rc1\n0.1.8\n",
            stderr="",
        )

        previous = generate_changelog.find_previous_stable_tag(Path())

        self.assertEqual(previous, "0.1.8")


if __name__ == "__main__":
    unittest.main()
