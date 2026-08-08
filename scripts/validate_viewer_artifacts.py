#!/usr/bin/env python3
"""Validate the versioned CDT++ viewer fixture, manifest, and hero PNG."""

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path
from typing import TYPE_CHECKING, TypeGuard

import jsonschema

if TYPE_CHECKING:
    from collections.abc import Sequence

type JsonObject = dict[str, object]

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211
FNV_MASK = (1 << 64) - 1


class ViewerArtifactError(ValueError):
    """A viewer input or committed artifact violates the v1 contract."""


def _is_object(value: object) -> TypeGuard[JsonObject]:
    """Return whether *value* is a JSON object with string keys."""
    return isinstance(value, dict) and all(isinstance(key, str) for key in value)


def _object(value: object, context: str) -> JsonObject:
    """Return one required JSON object."""
    if not _is_object(value):
        message = f"{context} must be an object"
        raise ViewerArtifactError(message)
    return value


def _string(parent: JsonObject, key: str, context: str) -> str:
    """Return one required nonempty JSON string."""
    value = parent.get(key)
    if not isinstance(value, str) or not value:
        message = f"{context}.{key} must be a nonempty string"
        raise ViewerArtifactError(message)
    return value


def _integer(parent: JsonObject, key: str, context: str) -> int:
    """Return one required JSON integer."""
    value = parent.get(key)
    if not isinstance(value, int) or isinstance(value, bool):
        message = f"{context}.{key} must be an integer"
        raise ViewerArtifactError(message)
    return value


def _reject_json_constant(value: str) -> object:
    """Reject JavaScript constants that RFC 8259 excludes from JSON."""
    message = f"non-standard JSON constant {value!r} is not permitted"
    raise ViewerArtifactError(message)


def _load_object(path: Path) -> JsonObject:
    """Load a JSON object from *path*."""
    return _object(
        json.loads(path.read_text(encoding="utf-8"), parse_constant=_reject_json_constant),
        str(path),
    )


def _sha256(path: Path) -> str:
    """Return the lowercase SHA-256 digest for *path*."""
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while block := stream.read(1024 * 1024):
            digest.update(block)
    return digest.hexdigest()


def _fnv1a64(path: Path) -> str:
    """Return the persistence contract's lowercase FNV-1a digest."""
    digest = FNV_OFFSET
    with path.open("rb") as stream:
        while block := stream.read(1024 * 1024):
            for byte in block:
                digest ^= byte
                digest = (digest * FNV_PRIME) & FNV_MASK
    return f"{digest:016x}"


def _metadata(path: Path) -> dict[str, str]:
    """Parse a CDT++ metadata-v1 sidecar into unique key/value fields."""
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines or lines[0] != "cdt-plusplus-metadata-v1":
        message = f"{path} is not cdt-plusplus-metadata-v1"
        raise ViewerArtifactError(message)
    result: dict[str, str] = {}
    for line_number, line in enumerate(lines[1:], start=2):
        key, separator, value = line.partition("=")
        if not separator or not key or not value:
            message = f"{path}:{line_number} is not a metadata key/value"
            raise ViewerArtifactError(message)
        if key in result:
            message = f"{path}:{line_number} duplicates metadata field {key!r}"
            raise ViewerArtifactError(message)
        result[key] = value
    return result


def _metadata_unsigned_integer(metadata: dict[str, str], key: str, path: Path) -> int:
    """Parse one required ASCII unsigned integer metadata field."""
    value = metadata.get(key)
    message = f"{path} metadata field {key!r} must be an unsigned integer"
    if value is None or not value.isascii() or not value.isdigit():
        raise ViewerArtifactError(message)
    try:
        return int(value)
    except ValueError as error:
        raise ViewerArtifactError(message) from error


def _validate_schema(manifest: JsonObject, manifest_path: Path) -> None:
    """Validate *manifest* against its repository-relative schema."""
    schema_path = (manifest_path.parent / _string(manifest, "$schema", str(manifest_path))).resolve()
    schema = _load_object(schema_path)
    validator_type = jsonschema.validators.validator_for(schema)
    validator_type.check_schema(schema)
    errors = sorted(validator_type(schema).iter_errors(manifest), key=lambda error: list(error.absolute_path))
    if errors:
        details = "; ".join(error.message for error in errors)
        message = f"{manifest_path} does not match {schema_path}: {details}"
        raise ViewerArtifactError(message)


def _validate_fixture(manifest: JsonObject, manifest_path: Path) -> None:
    """Validate the OFF payload, sidecar, digest, and expected topology."""
    fixture = _object(manifest.get("fixture"), "fixture")
    fixture_path = (manifest_path.parent / _string(fixture, "path", "fixture")).resolve()
    metadata_path = (manifest_path.parent / _string(fixture, "metadata_path", "fixture")).resolve()
    if not fixture_path.is_file() or not metadata_path.is_file():
        message = f"viewer fixture pair is missing: {fixture_path}, {metadata_path}"
        raise ViewerArtifactError(message)
    if metadata_path != Path(f"{fixture_path}.meta"):
        message = "fixture.metadata_path must name the payload's .meta sidecar"
        raise ViewerArtifactError(message)
    actual_sha256 = _sha256(fixture_path)
    if actual_sha256 != _string(fixture, "sha256", "fixture"):
        message = f"viewer fixture SHA-256 mismatch: {actual_sha256}"
        raise ViewerArtifactError(message)

    metadata = _metadata(metadata_path)
    if _metadata_unsigned_integer(metadata, "payload.size", metadata_path) != fixture_path.stat().st_size:
        message = "viewer fixture payload.size does not match the OFF file"
        raise ViewerArtifactError(message)
    if metadata.get("payload.fnv1a64") != _fnv1a64(fixture_path):
        message = "viewer fixture payload.fnv1a64 does not match the OFF file"
        raise ViewerArtifactError(message)

    topology = _object(fixture.get("expected_topology"), "fixture.expected_topology")
    topology_fields = {
        "vertices": "actual.vertices",
        "edges": "actual.edges",
        "faces": "actual.faces",
        "simplices": "actual.simplices",
        "minimum_timeslice": "actual.minimum_timeslice",
        "maximum_timeslice": "actual.maximum_timeslice",
    }
    for manifest_field, metadata_field in topology_fields.items():
        expected = _integer(topology, manifest_field, "fixture.expected_topology")
        if metadata.get(metadata_field) != str(expected):
            message = f"{metadata_field} does not match fixture.expected_topology.{manifest_field}"
            raise ViewerArtifactError(message)

    provenance = _object(fixture.get("provenance"), "fixture.provenance")
    provenance_fields = {
        "dimension": "dimension",
        "desired_simplices": "desired.simplices",
        "timeslices": "desired.timeslices",
        "seed": "random.seed",
        "threads": "parallel.max_threads",
    }
    for manifest_field, metadata_field in provenance_fields.items():
        expected = _integer(provenance, manifest_field, "fixture.provenance")
        if metadata.get(metadata_field) != str(expected):
            message = f"{metadata_field} does not match fixture.provenance.{manifest_field}"
            raise ViewerArtifactError(message)


def _png_dimensions(path: Path) -> tuple[int, int]:
    """Return PNG IHDR dimensions after checking its signature and first chunk."""
    with path.open("rb") as stream:
        header = stream.read(24)
    if len(header) != 24 or header[:8] != PNG_SIGNATURE or header[12:16] != b"IHDR":
        message = f"{path} is not a PNG with an initial IHDR chunk"
        raise ViewerArtifactError(message)
    width, height = struct.unpack(">II", header[16:24])
    return width, height


def _validate_image(manifest: JsonObject, manifest_path: Path, image_override: Path | None, *, canonical: bool) -> Path:
    """Validate committed or caller-supplied image structure and canonical digest."""
    render = _object(manifest.get("render"), "render")
    comparison = _object(manifest.get("comparison"), "comparison")
    artifact = (manifest_path.parent / _string(manifest, "artifact", str(manifest_path))).resolve()
    image_path = image_override.resolve() if image_override is not None else artifact
    if not image_path.is_file():
        message = f"viewer image is missing: {image_path}"
        raise ViewerArtifactError(message)
    minimum_bytes = _integer(comparison, "minimum_file_bytes", "comparison")
    if image_path.stat().st_size < minimum_bytes:
        message = f"viewer image is smaller than {minimum_bytes} bytes: {image_path}"
        raise ViewerArtifactError(message)
    expected_dimensions = (_integer(render, "width", "render"), _integer(render, "height", "render"))
    if _png_dimensions(image_path) != expected_dimensions:
        message = f"viewer image dimensions do not match render.width/render.height: {image_path}"
        raise ViewerArtifactError(message)
    if canonical:
        actual_sha256 = _sha256(image_path)
        if actual_sha256 != _string(comparison, "canonical_sha256", "comparison"):
            message = f"canonical viewer image SHA-256 mismatch: {actual_sha256}"
            raise ViewerArtifactError(message)
    return image_path


def validate(manifest_path: Path, image_override: Path | None = None, *, canonical: bool = True) -> Path:
    """Validate one viewer manifest and return its checked image path."""
    manifest_path = manifest_path.resolve()
    manifest = _load_object(manifest_path)
    _validate_schema(manifest, manifest_path)
    _validate_fixture(manifest, manifest_path)
    return _validate_image(manifest, manifest_path, image_override, canonical=canonical)


def _parse_args(argv: Sequence[str]) -> argparse.Namespace:
    """Parse command-line arguments."""
    default_manifest = Path(__file__).resolve().parents[1] / "viewer" / "manifests" / "v1" / "hero.json"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=default_manifest, help="render manifest to validate")
    parser.add_argument("--image", type=Path, help="alternate rendered PNG to check")
    parser.add_argument(
        "--structural-only",
        action="store_true",
        help="skip canonical PNG digest comparison for a noncanonical renderer environment",
    )
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    """Validate viewer artifacts and report the checked image."""
    args = _parse_args(sys.argv[1:] if argv is None else argv)
    try:
        image = validate(args.manifest, args.image, canonical=not args.structural_only)
    except (OSError, ViewerArtifactError, json.JSONDecodeError, jsonschema.SchemaError) as error:
        print(f"Viewer artifact validation failed: {error}", file=sys.stderr)
        return 1
    print(f"Viewer fixture, render manifest, and image are valid: {image}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
