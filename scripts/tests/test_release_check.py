"""Tests for release metadata validation."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts import release_check


def _write_project(root: Path, *, metadata_version: str = "1.2.3-rc4", release_date: str = "2026-07-20") -> None:
    """Write a minimal synchronized release-metadata fixture."""
    pep440_version = metadata_version.replace("-rc", "rc")
    files = {
        "CMakeLists.txt": ('project(\n  CDT-plusplus\n  VERSION 1.2.3\n  DESCRIPTION "fixture"\n  LANGUAGES CXX)\nset(CDT_VERSION_SUFFIX "-rc4")\n'),
        "vcpkg.json": f'{{"name": "cdt-plusplus", "version": "{metadata_version}"}}\n',
        "pyproject.toml": f'[project]\nname = "cdt-plusplus-scripts"\nversion = "{pep440_version}"\n',
        "uv.lock": (f'version = 1\n\n[[package]]\nname = "cdt-plusplus-scripts"\nversion = "{pep440_version}"\nsource = {{ editable = "." }}\n'),
        "CITATION.cff": (
            "cff-version: 1.2.0\n"
            'message: "Cite this software."\n'
            "type: software\n"
            'title: "CDT++"\n'
            'abstract: "A fixture."\n'
            "authors:\n"
            '  - family-names: "Getchell"\n'
            '    given-names: "Adam"\n'
            f'version: "{metadata_version}"\n'
            f'date-released: "{release_date}"\n'
            'repository-code: "https://example.com/repository"\n'
            'url: "https://example.com"\n'
            'license: "BSD-3-Clause"\n'
        ),
        "docs/Doxyfile": f"PROJECT_NUMBER = {metadata_version}\n",
        "README.md": f"Current release: v{metadata_version}.\n",
        "REFERENCES.md": f"Version {metadata_version}.\n",
        ".github/CONTRIBUTING.md": f"Contributing to v{metadata_version}.\n",
    }
    for relative_path, content in files.items():
        path = root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")


class ReleaseCheckTests(unittest.TestCase):
    """Exercise synchronized and malformed release fixtures."""

    def test_accepts_synchronized_release_metadata(self) -> None:
        """Matching metadata returns the product version and release date."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            _write_project(root)

            version, release_date = release_check.check_release_metadata(root)

        self.assertEqual(version, "1.2.3-rc4")
        self.assertEqual(release_date.isoformat(), "2026-07-20")

    def test_rejects_structured_version_drift(self) -> None:
        """A stale package version fails with its owning file."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            _write_project(root)
            (root / "vcpkg.json").write_text('{"name": "cdt-plusplus", "version": "1.2.3-rc3"}\n', encoding="utf-8")

            with self.assertRaisesRegex(release_check.ReleaseCheckError, "vcpkg.json version=1.2.3-rc3"):
                release_check.check_release_metadata(root)

    def test_rejects_invalid_citation_date(self) -> None:
        """CFF release dates must be real ISO calendar dates."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            _write_project(root, release_date="2026-02-30")

            with self.assertRaisesRegex(release_check.ReleaseCheckError, "ISO calendar date"):
                release_check.check_release_metadata(root)

    def test_rejects_stale_active_documentation(self) -> None:
        """Active documentation may not retain an older RC version."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            _write_project(root)
            (root / "README.md").write_text("Current release: v1.2.3-rc3.\n", encoding="utf-8")

            with self.assertRaisesRegex(release_check.ReleaseCheckError, "stale release-candidate versions"):
                release_check.check_release_metadata(root)


if __name__ == "__main__":
    unittest.main()
