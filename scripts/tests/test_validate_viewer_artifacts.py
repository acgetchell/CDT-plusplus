"""Tests for the archival viewer fixture and artifact validator."""

import io
import shutil
import tempfile
import unittest
from contextlib import redirect_stderr
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

    @staticmethod
    def _image_for(manifest: Path) -> Path:
        """Return the copied contract's committed image path."""
        return manifest.parents[3] / "docs" / "images" / "S3-7-27528-I1-R1.png"

    @staticmethod
    def _metadata_for(manifest: Path) -> Path:
        """Return the copied contract's fixture metadata path."""
        return manifest.parents[2] / "fixtures" / "v1" / "S3-7-27528-I1-R1-seed30.off.meta"

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
        metadata = self._metadata_for(manifest)
        contents = metadata.read_text(encoding="utf-8")
        self.assertIn("payload.size=1901391", contents)
        metadata.write_text(contents.replace("payload.size=1901391", "payload.size=invalid", 1), encoding="utf-8")

        with self.assertRaisesRegex(validate_viewer_artifacts.ViewerArtifactError, "'payload.size' must be an unsigned integer"):
            validate_viewer_artifacts.validate(manifest)

    def test_structural_only_accepts_a_noncanonical_png(self) -> None:
        """Structural validation permits a readable-shaped image with another digest."""
        manifest = self._copy_contract()
        image = self._image_for(manifest)
        image.write_bytes(image.read_bytes() + b"\0")

        self.assertEqual(validate_viewer_artifacts.validate(manifest, canonical=False), image.resolve())
        with self.assertRaisesRegex(validate_viewer_artifacts.ViewerArtifactError, "canonical viewer image SHA-256 mismatch"):
            validate_viewer_artifacts.validate(manifest)

    def test_wrong_png_dimensions_are_rejected(self) -> None:
        """The structural policy still requires manifest-declared dimensions."""
        manifest = self._copy_contract()
        image = self._image_for(manifest)
        contents = bytearray(image.read_bytes())
        contents[16:20] = (736).to_bytes(4, byteorder="big")
        image.write_bytes(contents)

        with self.assertRaisesRegex(validate_viewer_artifacts.ViewerArtifactError, "dimensions do not match"):
            validate_viewer_artifacts.validate(manifest, canonical=False)

    def test_undersized_png_is_rejected(self) -> None:
        """A PNG header alone cannot satisfy the minimum artifact size."""
        manifest = self._copy_contract()
        image = self._image_for(manifest)
        image.write_bytes(image.read_bytes()[:24])

        with self.assertRaisesRegex(validate_viewer_artifacts.ViewerArtifactError, "smaller than 20000 bytes"):
            validate_viewer_artifacts.validate(manifest, canonical=False)

    def test_provenance_topology_mismatch_is_rejected(self) -> None:
        """The sidecar topology must agree with the render provenance."""
        manifest = self._copy_contract()
        metadata = self._metadata_for(manifest)
        contents = metadata.read_text(encoding="utf-8")
        metadata.write_text(contents.replace("topology=spherical", "topology=toroidal", 1), encoding="utf-8")

        with self.assertRaisesRegex(validate_viewer_artifacts.ViewerArtifactError, "expected 'spherical', got 'toroidal'"):
            validate_viewer_artifacts.validate(manifest)

    def test_provenance_decimal_mismatches_are_rejected(self) -> None:
        """Radius and spacing use exact decimal-value comparison with the sidecar."""
        for field in ("initial_radius", "foliation_spacing"):
            with self.subTest(field=field):
                manifest = self._copy_contract()
                metadata = self._metadata_for(manifest)
                contents = metadata.read_text(encoding="utf-8")
                metadata.write_text(contents.replace(f"{field}=1", f"{field}=1.0000000000001", 1), encoding="utf-8")

                with self.assertRaisesRegex(validate_viewer_artifacts.ViewerArtifactError, f"{field} does not match"):
                    validate_viewer_artifacts.validate(manifest)

    def test_cli_reports_invalid_utf8_without_a_traceback(self) -> None:
        """Invalid UTF-8 is an ordinary nonzero CLI validation failure."""
        manifest = self._copy_contract()
        manifest.write_bytes(b"\xff")
        stderr = io.StringIO()

        with redirect_stderr(stderr):
            status = validate_viewer_artifacts.main(["--manifest", str(manifest)])

        self.assertEqual(status, 1)
        self.assertIn("Viewer artifact validation failed:", stderr.getvalue())
        self.assertNotIn("Traceback", stderr.getvalue())


if __name__ == "__main__":
    unittest.main()
