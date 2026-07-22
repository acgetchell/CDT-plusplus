#!/usr/bin/env python3
"""Validate CDT++ release metadata and version synchronization."""

from __future__ import annotations

import argparse
import json
import re
import sys
import tomllib
from datetime import date
from pathlib import Path
from typing import TYPE_CHECKING, TypeGuard

import yaml

if TYPE_CHECKING:
    from collections.abc import Sequence

type ParsedObject = dict[str, object]

SEMVER_RE = re.compile(r"[0-9]+[.][0-9]+[.][0-9]+(?:-rc[0-9]+)?")
PEP440_RE = re.compile(r"(?P<base>[0-9]+[.][0-9]+[.][0-9]+)(?:rc(?P<rc>[0-9]+))?")
RC_REFERENCE_RE = re.compile(r"(?<![0-9A-Za-z])v?(?P<version>[0-9]+[.][0-9]+[.][0-9]+-rc[0-9]+)(?![0-9A-Za-z])")
ACTIVE_RELEASE_DOCS = (Path("README.md"), Path("REFERENCES.md"), Path(".github/CONTRIBUTING.md"))


class ReleaseCheckError(ValueError):
    """A release metadata file is malformed or inconsistent."""


def _is_parsed_object(value: object) -> TypeGuard[ParsedObject]:
    """Return whether *value* is a mapping with string keys."""
    return isinstance(value, dict) and all(isinstance(key, str) for key in value)


def _require_object(value: object, context: str) -> ParsedObject:
    """Return *value* as a parsed mapping or fail with context."""
    if not _is_parsed_object(value):
        message = f"{context} must be a mapping"
        raise ReleaseCheckError(message)
    return value


def _require_string(data: ParsedObject, key: str, context: str) -> str:
    """Return one required nonempty string field."""
    value = data.get(key)
    if not isinstance(value, str) or not value.strip():
        message = f"{context} must contain a nonempty {key!r} string"
        raise ReleaseCheckError(message)
    return value


def _read_toml(path: Path) -> ParsedObject:
    """Read one TOML document as a string-keyed mapping."""
    return _require_object(tomllib.loads(path.read_text(encoding="utf-8")), str(path))


def _single_match(path: Path, pattern: re.Pattern[str], description: str) -> str:
    """Return the only named ``value`` match in *path*."""
    matches = [match.group("value") for match in pattern.finditer(path.read_text(encoding="utf-8"))]
    if len(matches) != 1:
        message = f"{path} must contain exactly one {description}; found {len(matches)}"
        raise ReleaseCheckError(message)
    return matches[0]


def _cmake_version(root: Path) -> str:
    """Return the canonical product version declared by CMake."""
    path = root / "CMakeLists.txt"
    project_version = _single_match(
        path,
        re.compile(r"project[(][^)]*?\bVERSION\s+(?P<value>[0-9]+[.][0-9]+[.][0-9]+)\b", re.DOTALL),
        "project VERSION",
    )
    suffix = _single_match(
        path,
        re.compile(r'^set[(]CDT_VERSION_SUFFIX\s+"(?P<value>[^"]*)"[)]\s*$', re.MULTILINE),
        "CDT_VERSION_SUFFIX",
    )
    version = f"{project_version}{suffix}"
    if SEMVER_RE.fullmatch(version) is None:
        message = f"{path} declares unsupported release version {version!r}"
        raise ReleaseCheckError(message)
    return version


def _pep440_to_product_version(version: str, context: str) -> str:
    """Convert the repository's PEP 440 spelling to its product spelling."""
    match = PEP440_RE.fullmatch(version)
    if match is None:
        message = f"{context} declares unsupported PEP 440 version {version!r}"
        raise ReleaseCheckError(message)
    rc = match.group("rc")
    return f"{match.group('base')}-rc{rc}" if rc is not None else match.group("base")


def _pyproject_metadata(root: Path) -> tuple[str, str]:
    """Return the Python support project name and product version."""
    path = root / "pyproject.toml"
    project = _require_object(_read_toml(path).get("project"), f"{path} [project]")
    name = _require_string(project, "name", f"{path} [project]")
    version = _require_string(project, "version", f"{path} [project]")
    return name, _pep440_to_product_version(version, f"{path} [project]")


def _uv_lock_version(root: Path, project_name: str) -> str:
    """Return the locked version of the local Python support project."""
    path = root / "uv.lock"
    packages = _read_toml(path).get("package")
    if not isinstance(packages, list):
        message = f"{path} must contain [[package]] entries"
        raise ReleaseCheckError(message)
    matches: list[ParsedObject] = []
    for index, package in enumerate(packages, start=1):
        parsed = _require_object(package, f"{path} [[package]] entry {index}")
        source = parsed.get("source")
        if parsed.get("name") == project_name and _is_parsed_object(source) and source.get("editable") == ".":
            matches.append(parsed)
    if len(matches) != 1:
        message = f"{path} must contain exactly one editable package named {project_name!r}; found {len(matches)}"
        raise ReleaseCheckError(message)
    version = _require_string(matches[0], "version", f"{path} package {project_name!r}")
    return _pep440_to_product_version(version, f"{path} package {project_name!r}")


def _citation_metadata(root: Path) -> tuple[str, date]:
    """Validate required CFF software fields and return version/date."""
    path = root / "CITATION.cff"
    citation = _require_object(yaml.safe_load(path.read_text(encoding="utf-8")), str(path))
    if _require_string(citation, "cff-version", str(path)) != "1.2.0":
        message = f"{path} must use cff-version 1.2.0"
        raise ReleaseCheckError(message)
    if _require_string(citation, "type", str(path)) != "software":
        message = f"{path} must describe software"
        raise ReleaseCheckError(message)
    for key in ("message", "title", "abstract", "repository-code", "url", "license"):
        _require_string(citation, key, str(path))
    authors = citation.get("authors")
    if not isinstance(authors, list) or not authors:
        message = f"{path} must contain at least one author"
        raise ReleaseCheckError(message)
    for index, author in enumerate(authors, start=1):
        parsed_author = _require_object(author, f"{path} author {index}")
        _require_string(parsed_author, "family-names", f"{path} author {index}")
        _require_string(parsed_author, "given-names", f"{path} author {index}")
    version = _require_string(citation, "version", str(path))
    raw_date = _require_string(citation, "date-released", str(path))
    try:
        release_date = date.fromisoformat(raw_date)
    except ValueError as error:
        message = f"{path} date-released must be an ISO calendar date: {raw_date!r}"
        raise ReleaseCheckError(message) from error
    return version, release_date


def _release_versions(root: Path) -> tuple[dict[str, str], date]:
    """Collect every structured release version."""
    vcpkg_path = root / "vcpkg.json"
    vcpkg = _require_object(json.loads(vcpkg_path.read_text(encoding="utf-8")), str(vcpkg_path))
    pyproject_name, pyproject_version = _pyproject_metadata(root)
    citation_version, release_date = _citation_metadata(root)
    versions = {
        "CMakeLists.txt product version": _cmake_version(root),
        "vcpkg.json version": _require_string(vcpkg, "version", str(vcpkg_path)),
        "pyproject.toml version": pyproject_version,
        "uv.lock local package version": _uv_lock_version(root, pyproject_name),
        "docs/Doxyfile PROJECT_NUMBER": _single_match(
            root / "docs/Doxyfile",
            re.compile(r"^PROJECT_NUMBER\s*=\s*(?P<value>\S+)\s*$", re.MULTILINE),
            "PROJECT_NUMBER",
        ),
        "CITATION.cff version": citation_version,
    }
    return versions, release_date


def _check_active_release_docs(root: Path, expected: str) -> None:
    """Reject stale release-candidate references in active documentation."""
    for relative_path in ACTIVE_RELEASE_DOCS:
        path = root / relative_path
        references = [match.group("version") for match in RC_REFERENCE_RE.finditer(path.read_text(encoding="utf-8"))]
        stale = sorted({version for version in references if version != expected})
        if stale:
            message = f"{path} contains stale release-candidate versions: {', '.join(stale)}; expected {expected}"
            raise ReleaseCheckError(message)
        if "-rc" in expected and expected not in references:
            message = f"{path} must reference the current release candidate {expected}"
            raise ReleaseCheckError(message)


def _check_changelog_release(root: Path, expected: str, release_date: date) -> None:
    """Require one changelog heading matching release metadata."""
    path = root / "CHANGELOG.md"
    heading_re = re.compile(rf"^## \[{re.escape(expected)}\] - (?P<date>\d{{4}}-\d{{2}}-\d{{2}})$", re.MULTILINE)
    matches = list(heading_re.finditer(path.read_text(encoding="utf-8")))
    if len(matches) != 1:
        message = f"{path} must contain exactly one release heading for {expected}; found {len(matches)}"
        raise ReleaseCheckError(message)
    raw_date = matches[0].group("date")
    try:
        changelog_date = date.fromisoformat(raw_date)
    except ValueError as error:
        message = f"{path} release date must be an ISO calendar date: {raw_date!r}"
        raise ReleaseCheckError(message) from error
    if changelog_date != release_date:
        message = f"{path} release date {changelog_date.isoformat()} does not match CITATION.cff {release_date.isoformat()}"
        raise ReleaseCheckError(message)


def _check_release_metadata(root: Path, *, include_changelog: bool) -> tuple[str, date]:
    """Validate structured metadata and optionally its generated changelog."""
    versions, release_date = _release_versions(root)
    expected = versions["CMakeLists.txt product version"]
    mismatches = {source: version for source, version in versions.items() if version != expected}
    if mismatches:
        details = "; ".join(f"{source}={version}" for source, version in mismatches.items())
        message = f"release metadata must match {expected} from CMakeLists.txt: {details}"
        raise ReleaseCheckError(message)
    _check_active_release_docs(root, expected)
    if include_changelog:
        _check_changelog_release(root, expected, release_date)
    return expected, release_date


def check_release_inputs(root: Path) -> tuple[str, date]:
    """Validate release inputs before regenerating ``CHANGELOG.md``."""
    return _check_release_metadata(root, include_changelog=False)


def check_release_metadata(root: Path) -> tuple[str, date]:
    """Validate release metadata and its generated changelog."""
    return _check_release_metadata(root, include_changelog=True)


def _parse_args(argv: Sequence[str]) -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", nargs="?", type=Path, default=Path.cwd(), help="repository root (default: current directory)")
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    """Validate release metadata and report the synchronized version."""
    args = _parse_args(sys.argv[1:] if argv is None else argv)
    try:
        version, release_date = check_release_metadata(args.root.resolve())
    except (OSError, ReleaseCheckError, json.JSONDecodeError, tomllib.TOMLDecodeError, yaml.YAMLError) as error:
        print(f"Release metadata validation failed: {error}", file=sys.stderr)
        return 1
    print(f"Release metadata is synchronized at {version} with release date {release_date.isoformat()}.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
