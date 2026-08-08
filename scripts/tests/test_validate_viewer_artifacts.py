"""Tests for the archival viewer fixture and artifact validator."""

import shutil
import tempfile
import unittest
from pathlib import Path

from scripts import validate_viewer_artifacts

REPO_ROOT = Path(__file__).resolve().parents[2]


class ViewerArtifactValidationTests(unittest.TestCase):
    """Exercise the tracked v1 viewer contract as one integrated fixture."""

    def _copy_contract(self) -> Path:
        """Copy the versioned viewer contract into an isolated temporary tree."""
        temporary_root = Path(self.enterContext(tempfile.TemporaryDirectory()))
        shutil.copytree(REPO_ROOT / "viewer", temporary_root / "viewer")
        image = temporary_root / "docs" / "images" / "S3-7-27528-I1-R1.png"
        image.parent.mkdir(parents=True)
        shutil.copy2(REPO_ROOT / "docs" / "images" / image.name, image)
        return temporary_root / "viewer" / "manifests" / "v1" / "hero.json"

    def test_repository_viewer_artifacts_validate(self) -> None:
        """The schema, OFF pair, topology, provenance, and hero digest agree."""
        manifest = REPO_ROOT / "viewer" / "manifests" / "v1" / "hero.json"

        image = validate_viewer_artifacts.validate(manifest)

        self.assertEqual(image, REPO_ROOT / "docs" / "images" / "S3-7-27528-I1-R1.png")

    def test_nonfinite_manifest_number_is_rejected(self) -> None:
        """NaN must not pass Python validation when Qt rejects the same JSON."""
        manifest = self._copy_contract()
        contents = manifest.read_text(encoding="utf-8")
        self.assertIn('"oversampling": 1.0', contents)
        manifest.write_text(contents.replace('"oversampling": 1.0', '"oversampling": NaN', 1), encoding="utf-8")

        with self.assertRaisesRegex(validate_viewer_artifacts.ViewerArtifactError, "non-standard JSON constant 'NaN'"):
            validate_viewer_artifacts.validate(manifest)

    def test_malformed_payload_size_is_rejected_without_leaking_value_error(self) -> None:
        """Malformed sidecar integers must produce the validator's stable error."""
        manifest = self._copy_contract()
        metadata = manifest.parents[2] / "fixtures" / "v1" / "S3-7-27528-I1-R1-seed30.off.meta"
        contents = metadata.read_text(encoding="utf-8")
        self.assertIn("payload.size=1901391", contents)
        metadata.write_text(contents.replace("payload.size=1901391", "payload.size=invalid", 1), encoding="utf-8")

        with self.assertRaisesRegex(validate_viewer_artifacts.ViewerArtifactError, "'payload.size' must be an unsigned integer"):
            validate_viewer_artifacts.validate(manifest)


if __name__ == "__main__":
    unittest.main()
