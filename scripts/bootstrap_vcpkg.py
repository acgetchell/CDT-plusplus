"""Bootstrap and validate the repository-pinned vcpkg checkout."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Callable, Mapping, Sequence

VCPKG_TOOL_RELEASE = "2026-07-13"
OFFICIAL_ORIGINS = frozenset(
    {
        "https://github.com/microsoft/vcpkg",
        "https://github.com/microsoft/vcpkg.git",
    }
)
WINDOWS_TOOL_SHA256 = {
    "amd64": "67958c6a13a35130ff8035bef33097ffe3376a6708577a826cfa41fa592db611",
    "arm64": "8d87ed438db65b0015f624693612cb79d8c35908348c194984c23b63a2da0211",
}


class BootstrapError(RuntimeError):
    """Report an invalid or unusable vcpkg checkout."""


@dataclass(frozen=True)
class ToolSpec:
    """Describe the trusted vcpkg executable for one host platform."""

    executable_name: str
    hash_algorithm: str
    expected_digest: str


def _run(
    command: Sequence[str | Path],
    *,
    cwd: Path | None = None,
    capture_output: bool = False,
) -> subprocess.CompletedProcess[str]:
    """Run a resolved repository-maintenance command without a shell."""
    resolved_command = [str(argument) for argument in command]
    try:
        return subprocess.run(  # noqa: S603 - callers resolve executables or use validated checkout paths.
            resolved_command,
            cwd=cwd,
            check=False,
            capture_output=capture_output,
            text=True,
        )
    except OSError as error:
        message = f"Unable to run {resolved_command[0]}: {error}"
        raise BootstrapError(message) from error


def _require_executable(name: str) -> str:
    """Resolve a required executable or fail with an actionable message."""
    executable = shutil.which(name)
    if executable is None:
        message = f"{name} is required to bootstrap vcpkg."
        raise BootstrapError(message)
    return executable


def _git(git: str, checkout: Path, *arguments: str, capture_output: bool = True) -> subprocess.CompletedProcess[str]:
    """Run Git against a specific checkout."""
    return _run([git, "-C", checkout, *arguments], capture_output=capture_output)


def _git_output(git: str, checkout: Path, *arguments: str) -> str:
    """Return trimmed Git output or raise a bootstrap error."""
    result = _git(git, checkout, *arguments)
    if result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or "unknown Git error"
        message = f"Git failed in {checkout}: {detail}"
        raise BootstrapError(message)
    return result.stdout.strip()


def _read_baseline(manifest: Path) -> str:
    """Read and validate the pinned registry baseline."""
    try:
        document = json.loads(manifest.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        message = f"Unable to read {manifest}: {error}"
        raise BootstrapError(message) from error
    baseline = document.get("builtin-baseline") if isinstance(document, dict) else None
    if not isinstance(baseline, str) or re.fullmatch(r"[0-9a-f]{40}", baseline) is None:
        message = f"Unable to read a 40-character builtin-baseline from {manifest}."
        raise BootstrapError(message)
    return baseline


def _read_metadata(metadata_path: Path) -> dict[str, str]:
    """Parse vcpkg tool metadata into key/value pairs."""
    try:
        lines = metadata_path.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        message = f"Unable to read vcpkg tool metadata at {metadata_path}."
        raise BootstrapError(message) from error
    metadata: dict[str, str] = {}
    for line in lines:
        key, separator, value = line.partition("=")
        if separator:
            metadata[key] = value
    return metadata


def _host_uses_musl() -> bool:
    """Return whether the Linux host reports musl as its C library."""
    ldd = shutil.which("ldd")
    if ldd is None:
        return False
    result = _run([ldd, "--version"], capture_output=True)
    return "musl" in f"{result.stdout}\n{result.stderr}".casefold()


def select_tool_spec(
    metadata: Mapping[str, str],
    *,
    system_name: str | None = None,
    machine_name: str | None = None,
    uses_musl: bool | None = None,
) -> ToolSpec:
    """Select the trusted executable and digest for a host platform."""
    system = (system_name or platform.system()).casefold()
    machine = (machine_name or platform.machine()).casefold()
    if system == "windows":
        architecture = "arm64" if machine in {"aarch64", "arm64"} else "amd64"
        if machine not in {"aarch64", "amd64", "arm64", "x86_64"}:
            message = f"Unsupported Windows architecture for vcpkg: {machine}."
            raise BootstrapError(message)
        return ToolSpec("vcpkg.exe", "sha256", WINDOWS_TOOL_SHA256[architecture])

    metadata_key: str
    if system == "darwin":
        metadata_key = "VCPKG_MACOS_SHA"
    elif system == "linux" and machine in {"aarch64", "arm64"}:
        metadata_key = "VCPKG_GLIBC_ARM64_SHA"
    elif system == "linux" and machine in {"amd64", "x86_64"}:
        metadata_key = "VCPKG_MUSLC_SHA" if (uses_musl if uses_musl is not None else _host_uses_musl()) else "VCPKG_GLIBC_SHA"
    else:
        message = f"Unsupported host for vcpkg: {system}/{machine}."
        raise BootstrapError(message)

    expected_digest = metadata.get(metadata_key, "")
    if re.fullmatch(r"[0-9a-f]{128}", expected_digest) is None:
        message = f"Invalid {metadata_key} value in vcpkg tool metadata."
        raise BootstrapError(message)
    return ToolSpec("vcpkg", "sha512", expected_digest)


def _hash_file(path: Path, algorithm: str) -> str:
    """Hash a file without loading the executable into memory."""
    digest = hashlib.new(algorithm)
    try:
        with path.open("rb") as handle:
            for block in iter(lambda: handle.read(1024 * 1024), b""):
                digest.update(block)
    except OSError as error:
        message = f"Unable to hash trusted vcpkg executable {path}: {error}"
        raise BootstrapError(message) from error
    return digest.hexdigest()


def _read_tool_version(executable: Path) -> str:
    """Run a hash-verified vcpkg executable and return its version output."""
    result = _run([executable, "version"], capture_output=True)
    if result.returncode != 0:
        message = f"Unable to read the vcpkg version from {executable}."
        raise BootstrapError(message)
    return result.stdout.strip()


def _validate_provenance(checkout: Path, git: str) -> None:
    """Require an official origin and an unchanged tracked worktree."""
    origin = _git_output(git, checkout, "remote", "get-url", "origin")
    if origin.casefold() not in OFFICIAL_ORIGINS:
        message = f"Refusing to reuse {checkout} because its origin is not microsoft/vcpkg."
        raise BootstrapError(message)

    diff = _git(git, checkout, "diff", "--quiet", "--no-ext-diff", "HEAD", "--")
    if diff.returncode == 1:
        status = _git_output(git, checkout, "status", "--short", "--untracked-files=no")
        detail = f"\n{status}" if status else ""
        message = f"Refusing to reuse a modified vcpkg checkout at {checkout}.{detail}"
        raise BootstrapError(message)
    if diff.returncode != 0:
        detail = diff.stderr.strip() or diff.stdout.strip() or "unknown Git error"
        message = f"Unable to verify tracked vcpkg files at {checkout}: {detail}"
        raise BootstrapError(message)


def validate_checkout(
    checkout: Path,
    baseline: str,
    *,
    git: str | None = None,
    tool_spec: ToolSpec | None = None,
    version_reader: Callable[[Path], str] = _read_tool_version,
) -> None:
    """Validate checkout provenance, revision, executable digest, and version."""
    git_executable = git or _require_executable("git")
    if not (checkout / ".git").is_dir():
        message = f"No vcpkg Git checkout exists at {checkout}."
        raise BootstrapError(message)
    _validate_provenance(checkout, git_executable)

    actual_commit = _git_output(git_executable, checkout, "rev-parse", "HEAD")
    if actual_commit.casefold() != baseline.casefold():
        message = f"vcpkg checkout {checkout} is at {actual_commit}, not pinned baseline {baseline}."
        raise BootstrapError(message)

    metadata_path = checkout / "scripts" / "vcpkg-tool-metadata.txt"
    metadata = _read_metadata(metadata_path)
    if metadata.get("VCPKG_TOOL_RELEASE_TAG") != VCPKG_TOOL_RELEASE:
        message = f"vcpkg tool metadata at {metadata_path} is not release {VCPKG_TOOL_RELEASE}."
        raise BootstrapError(message)
    selected_tool = tool_spec or select_tool_spec(metadata)
    expected_length = hashlib.new(selected_tool.hash_algorithm).digest_size * 2
    if re.fullmatch(rf"[0-9a-f]{{{expected_length}}}", selected_tool.expected_digest) is None:
        message = f"Invalid trusted {selected_tool.hash_algorithm} digest for {selected_tool.executable_name}."
        raise BootstrapError(message)

    executable = checkout / selected_tool.executable_name
    if not executable.is_file():
        message = f"Trusted vcpkg executable is missing: {executable}."
        raise BootstrapError(message)
    if platform.system() != "Windows" and not os.access(executable, os.X_OK):
        message = f"Trusted vcpkg executable is not executable: {executable}."
        raise BootstrapError(message)
    actual_digest = _hash_file(executable, selected_tool.hash_algorithm)
    if actual_digest != selected_tool.expected_digest:
        message = f"vcpkg executable digest does not match the trusted {selected_tool.hash_algorithm} value: {executable}."
        raise BootstrapError(message)

    expected_version_prefix = f"vcpkg package management program version {VCPKG_TOOL_RELEASE}-"
    version = version_reader(executable)
    if not version.casefold().startswith(expected_version_prefix.casefold()):
        message = f"Unexpected vcpkg tool version from {executable}: {version or '<empty output>'}."
        raise BootstrapError(message)


def _initialize_checkout(checkout: Path, git: str) -> None:
    """Create an empty official vcpkg Git checkout."""
    checkout.mkdir(parents=True, exist_ok=True)
    for arguments in (("init",), ("remote", "add", "origin", "https://github.com/microsoft/vcpkg.git")):
        result = _git(git, checkout, *arguments, capture_output=False)
        if result.returncode != 0:
            message = f"Unable to initialize vcpkg at {checkout}."
            raise BootstrapError(message)


def _update_checkout(checkout: Path, baseline: str, git: str) -> None:
    """Fetch and check out exactly the manifest's pinned baseline."""
    commands = (
        ("fetch", "--depth", "1", "origin", baseline),
        ("checkout", "--detach", baseline),
    )
    for arguments in commands:
        result = _git(git, checkout, *arguments, capture_output=False)
        if result.returncode != 0:
            message = f"Unable to update vcpkg at {checkout} to {baseline}."
            raise BootstrapError(message)


def _bootstrap_tool(checkout: Path, tool_spec: ToolSpec) -> None:
    """Build or download the vcpkg executable using the pinned source checkout."""
    executable = checkout / tool_spec.executable_name
    executable.unlink(missing_ok=True)
    if platform.system() == "Windows":
        command = [os.environ.get("COMSPEC", "cmd.exe"), "/d", "/c", checkout / "bootstrap-vcpkg.bat", "-disableMetrics"]
    else:
        command = [checkout / "bootstrap-vcpkg.sh", "-disableMetrics"]
    result = _run(command, cwd=checkout)
    if result.returncode != 0:
        message = f"Unable to bootstrap vcpkg at {checkout}."
        raise BootstrapError(message)


def bootstrap_vcpkg(repository_root: Path, *, check_only: bool = False) -> Path:
    """Validate or provision the repository-pinned vcpkg checkout."""
    git = _require_executable("git")
    manifest = repository_root / "vcpkg.json"
    baseline = _read_baseline(manifest)
    configured_checkout = os.environ.get("CDT_VCPKG_CACHE_DIR")
    checkout = Path(configured_checkout).resolve() if configured_checkout else (repository_root / ".cache" / "vcpkg").resolve()

    if (checkout / ".git").is_dir():
        _validate_provenance(checkout, git)
        try:
            validate_checkout(checkout, baseline, git=git)
        except BootstrapError:
            if check_only:
                raise
        else:
            print(f"Using vcpkg at {checkout} ({baseline})")
            return checkout
    elif checkout.is_dir() and any(checkout.iterdir()):
        message = f"Refusing to initialize vcpkg in non-empty directory {checkout}."
        raise BootstrapError(message)

    if check_only:
        message = f"VCPKG_ROOT is not a validated official checkout at baseline {baseline}: {checkout}"
        raise BootstrapError(message)

    if not (checkout / ".git").is_dir():
        _initialize_checkout(checkout, git)
    _update_checkout(checkout, baseline, git)
    metadata = _read_metadata(checkout / "scripts" / "vcpkg-tool-metadata.txt")
    tool_spec = select_tool_spec(metadata)
    _bootstrap_tool(checkout, tool_spec)
    validate_checkout(checkout, baseline, git=git, tool_spec=tool_spec)
    print(f"Bootstrapped vcpkg at {checkout} ({baseline})")
    return checkout


def _parse_args(argv: Sequence[str] | None) -> argparse.Namespace:
    """Parse the supported bootstrap mode."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="validate the existing checkout without changing it")
    return parser.parse_args(argv)


def _find_repository_root(start: Path | None = None) -> Path:
    """Find the nearest vcpkg manifest for direct and installed invocations."""
    search_roots = [Path(__file__).resolve().parent.parent, Path.cwd().resolve()] if start is None else [start.resolve()]
    visited: set[Path] = set()
    for search_root in search_roots:
        for candidate in (search_root, *search_root.parents):
            if candidate in visited:
                continue
            visited.add(candidate)
            if (candidate / "vcpkg.json").is_file():
                return candidate
    message = "Unable to find vcpkg.json from the working directory or bootstrap script location."
    raise BootstrapError(message)


def main(argv: Sequence[str] | None = None) -> int:
    """Run the vcpkg bootstrap or validation command."""
    args = _parse_args(argv)
    try:
        repository_root = _find_repository_root()
        bootstrap_vcpkg(repository_root, check_only=args.check)
    except (BootstrapError, OSError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
