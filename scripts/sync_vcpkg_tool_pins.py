"""Synchronize trusted vcpkg tool pins with the manifest's exact baseline."""

import argparse
import ast
import hashlib
import json
import os
import re
import stat
import sys
import tempfile
import urllib.error
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Callable, Sequence

METADATA_URL = "https://raw.githubusercontent.com/microsoft/vcpkg/{baseline}/scripts/vcpkg-tool-metadata.txt"
WINDOWS_TOOL_URLS = {
    "amd64": "https://github.com/microsoft/vcpkg-tool/releases/download/{release}/vcpkg.exe",
    "arm64": "https://github.com/microsoft/vcpkg-tool/releases/download/{release}/vcpkg-arm64.exe",
}


class PinSyncError(RuntimeError):
    """Report an unusable baseline, metadata file, or tool release."""


@dataclass(frozen=True)
class ToolPins:
    """Describe the trusted Windows assets for one vcpkg tool release."""

    release: str
    windows_sha256: dict[str, str]


def _download(url: str) -> bytes:
    """Download one official vcpkg resource."""
    request = urllib.request.Request(  # noqa: S310 - callers use fixed official HTTPS URL templates.
        url,
        headers={"User-Agent": "CDT-plusplus-vcpkg-pin-sync"},
    )
    try:
        with urllib.request.urlopen(request, timeout=60) as response:  # noqa: S310 - URLs are fixed to official HTTPS hosts.
            return response.read()
    except (OSError, urllib.error.URLError) as error:
        message = f"Unable to download {url}: {error}"
        raise PinSyncError(message) from error


def _read_baseline(manifest: Path) -> str:
    """Read the exact vcpkg registry baseline from the project manifest."""
    try:
        document = json.loads(manifest.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        message = f"Unable to read {manifest}: {error}"
        raise PinSyncError(message) from error
    baseline = document.get("builtin-baseline") if isinstance(document, dict) else None
    if not isinstance(baseline, str) or re.fullmatch(r"[0-9a-f]{40}", baseline) is None:
        message = f"Unable to read a 40-character builtin-baseline from {manifest}."
        raise PinSyncError(message)
    return baseline


def _read_tool_release(metadata: bytes, source: str) -> str:
    """Extract and validate the vcpkg tool release tag from upstream metadata."""
    try:
        lines = metadata.decode("utf-8").splitlines()
    except UnicodeDecodeError as error:
        message = f"Unable to decode vcpkg tool metadata from {source}."
        raise PinSyncError(message) from error
    values = dict(line.partition("=")[::2] for line in lines if "=" in line)
    release = values.get("VCPKG_TOOL_RELEASE_TAG", "")
    if re.fullmatch(r"[0-9]{4}-[0-9]{2}-[0-9]{2}", release) is None:
        message = f"Invalid VCPKG_TOOL_RELEASE_TAG in {source}."
        raise PinSyncError(message)
    return release


def _collect_pins(baseline: str, download: Callable[[str], bytes]) -> ToolPins:
    """Download all inputs and calculate pins without modifying the repository."""
    metadata_url = METADATA_URL.format(baseline=baseline)
    release = _read_tool_release(download(metadata_url), metadata_url)
    windows_sha256 = {architecture: hashlib.sha256(download(url.format(release=release))).hexdigest() for architecture, url in WINDOWS_TOOL_URLS.items()}
    return ToolPins(release, windows_sha256)


def _assignment_nodes(source: str) -> dict[str, ast.Assign]:
    """Locate the two bootstrap assignments that the synchronizer owns."""
    try:
        tree = ast.parse(source)
    except SyntaxError as error:
        message = f"Unable to parse scripts/bootstrap_vcpkg.py: {error}"
        raise PinSyncError(message) from error
    wanted = {"VCPKG_TOOL_RELEASE", "WINDOWS_TOOL_SHA256"}
    found: dict[str, ast.Assign] = {}
    for node in tree.body:
        if not isinstance(node, ast.Assign) or len(node.targets) != 1 or not isinstance(node.targets[0], ast.Name):
            continue
        name = node.targets[0].id
        if name in wanted:
            if name in found:
                message = f"scripts/bootstrap_vcpkg.py defines {name} more than once."
                raise PinSyncError(message)
            found[name] = node
    missing = wanted - found.keys()
    if missing:
        message = f"scripts/bootstrap_vcpkg.py is missing pin assignment(s): {', '.join(sorted(missing))}."
        raise PinSyncError(message)
    return found


def _render_bootstrap(source: str, pins: ToolPins) -> str:
    """Render both trusted pin assignments while preserving unrelated source."""
    line_ending_match = re.search(r"\r\n|\n|\r", source)
    line_ending = line_ending_match.group(0) if line_ending_match is not None else "\n"
    replacements = {
        "VCPKG_TOOL_RELEASE": f'VCPKG_TOOL_RELEASE = "{pins.release}"{line_ending}',
        "WINDOWS_TOOL_SHA256": (
            f"WINDOWS_TOOL_SHA256 = {{{line_ending}"
            f'    "amd64": "{pins.windows_sha256["amd64"]}",{line_ending}'
            f'    "arm64": "{pins.windows_sha256["arm64"]}",{line_ending}'
            f"}}{line_ending}"
        ),
    }
    lines = source.splitlines(keepends=True)
    assignments = _assignment_nodes(source)
    for name, node in sorted(assignments.items(), key=lambda item: item[1].lineno, reverse=True):
        if node.end_lineno is None:
            message = f"Unable to locate the end of {name} in scripts/bootstrap_vcpkg.py."
            raise PinSyncError(message)
        lines[node.lineno - 1 : node.end_lineno] = [replacements[name]]
    return "".join(lines)


def _atomic_write(path: Path, content: str) -> None:
    """Replace one source file atomically while preserving its permissions."""
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as temporary:
            temporary_path = Path(temporary.name)
            temporary.write(content)
            temporary.flush()
            os.fsync(temporary.fileno())
        temporary_path.chmod(stat.S_IMODE(path.stat().st_mode))
        temporary_path.replace(path)
    except OSError as error:
        message = f"Unable to update {path}: {error}"
        raise PinSyncError(message) from error
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def sync_vcpkg_tool_pins(repository_root: Path, *, download: Callable[[str], bytes] = _download) -> ToolPins:
    """Synchronize the bootstrap tool release and hashes for the manifest baseline."""
    baseline = _read_baseline(repository_root / "vcpkg.json")
    pins = _collect_pins(baseline, download)
    bootstrap = repository_root / "scripts" / "bootstrap_vcpkg.py"
    try:
        with bootstrap.open("r", encoding="utf-8", newline="") as source:
            original = source.read()
    except OSError as error:
        message = f"Unable to read {bootstrap}: {error}"
        raise PinSyncError(message) from error
    updated = _render_bootstrap(original, pins)
    if updated != original:
        _atomic_write(bootstrap, updated)
    return pins


def _parse_args(argv: Sequence[str] | None) -> argparse.Namespace:
    """Parse the intentionally argument-free synchronization command."""
    parser = argparse.ArgumentParser(description=__doc__)
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    """Synchronize pins for the repository containing this script."""
    _parse_args(argv)
    repository_root = Path(__file__).resolve().parent.parent
    try:
        pins = sync_vcpkg_tool_pins(repository_root)
    except PinSyncError as error:
        print(error, file=sys.stderr)
        return 1
    print(f"Pinned vcpkg tool {pins.release} for Windows amd64 and arm64.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
