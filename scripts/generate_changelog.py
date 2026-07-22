"""Generate a deterministic release changelog with git-cliff."""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
import tomllib
from pathlib import Path
from typing import TYPE_CHECKING

import yaml

from scripts import release_check
from scripts.subprocess_utils import ExecutableNotFoundError, run_git_command, run_support_command
from scripts.tag_release import ReleaseTagError, validate_semver

if TYPE_CHECKING:
    from collections.abc import Sequence
    from datetime import date


class ChangelogError(RuntimeError):
    """Report invalid release inputs or malformed git-cliff output."""


def apply_release_date(changelog: str, version: str, release_date: date) -> str:
    """Replace the generated target heading with its metadata-owned date."""
    heading = re.compile(rf"^## \[{re.escape(version)}\](?: - \d{{4}}-\d{{2}}-\d{{2}})?$", re.MULTILINE)
    updated, count = heading.subn(f"## [{version}] - {release_date.isoformat()}", changelog)
    if count != 1:
        message = f"git-cliff output must contain exactly one heading for {version}; found {count}."
        raise ChangelogError(message)
    return updated


def find_previous_stable_tag(root: Path) -> str:
    """Return the newest reachable stable SemVer tag."""
    result = run_git_command(["tag", "--merged", "HEAD", "--sort=-version:refname", "--list"], cwd=root)
    for candidate in result.stdout.splitlines():
        normalized = candidate if candidate.startswith("v") else f"v{candidate}"
        try:
            validate_semver(normalized)
        except ReleaseTagError:
            continue
        version = normalized.removeprefix("v").split("+", maxsplit=1)[0]
        if "-" not in version:
            return candidate
    message = "No reachable stable SemVer tag exists to use as the changelog baseline."
    raise ChangelogError(message)


def head_tag_ignore_pattern(root: Path, requested_tag: str) -> str | None:
    """Return a pattern for older tags on ``HEAD`` that override ``requested_tag``."""
    result = run_git_command(["tag", "--points-at", "HEAD", "--sort=refname", "--list"], cwd=root)
    conflicting_tags = sorted(tag for tag in result.stdout.splitlines() if tag and tag != requested_tag)
    if not conflicting_tags:
        return None
    alternatives = "|".join(re.escape(tag) for tag in conflicting_tags)
    return rf"^(?:{alternatives})$"


def _write_atomic(path: Path, text: str) -> None:
    """Replace ``path`` only after the complete generated text is available."""
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            dir=path.parent,
            prefix=f".{path.name}.",
            delete=False,
        ) as temporary:
            temporary.write(text)
            temporary_path = Path(temporary.name)
        if path.exists():
            temporary_path.chmod(path.stat().st_mode)
        temporary_path.replace(path)
    except OSError:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)
        raise


def generate_changelog(tag: str, *, root: Path | None = None, output: Path | None = None) -> Path:
    """Validate metadata, render git-cliff output, and atomically publish it."""
    validate_semver(tag)
    repository = (root or Path.cwd()).resolve()
    version = tag.removeprefix("v")
    metadata_version, release_date = release_check.check_release_inputs(repository)
    if version != metadata_version:
        message = f"Requested release {version} does not match synchronized metadata version {metadata_version}."
        raise ChangelogError(message)

    configuration = repository / "cliff.toml"
    if not configuration.is_file():
        message = f"{configuration} does not exist."
        raise ChangelogError(message)

    environment = dict(os.environ)
    environment["GIT_CLIFF_OFFLINE"] = "true"
    previous_tag = find_previous_stable_tag(repository)
    arguments = ["--config", str(configuration), "--tag", tag]
    if ignore_pattern := head_tag_ignore_pattern(repository, tag):
        arguments.extend(["--ignore-tags", ignore_pattern])
    arguments.append(f"{previous_tag}..HEAD")
    result = run_support_command(
        "git-cliff",
        arguments,
        cwd=repository,
        environment=environment,
    )
    if not result.stdout.strip():
        message = "git-cliff produced an empty changelog."
        raise ChangelogError(message)

    rendered = f"{apply_release_date(result.stdout, version, release_date).rstrip()}\n"
    destination = output or repository / "CHANGELOG.md"
    _write_atomic(destination, rendered)
    print(f"Generated {destination} for {tag} from {previous_tag} with release date {release_date.isoformat()}.")
    return destination


def _command_error(error: subprocess.CalledProcessError) -> str:
    """Format a failed support command without losing diagnostics."""
    details = (error.stderr or error.stdout or "").strip()
    return f"{error}\n{details}" if details else str(error)


def main(argv: Sequence[str] | None = None) -> int:
    """Run the deterministic changelog generator."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tag", help="SemVer release tag, including the leading v")
    args = parser.parse_args(argv)

    try:
        generate_changelog(args.tag)
    except (
        ChangelogError,
        ExecutableNotFoundError,
        ReleaseTagError,
        release_check.ReleaseCheckError,
        json.JSONDecodeError,
        tomllib.TOMLDecodeError,
        yaml.YAMLError,
        OSError,
        subprocess.TimeoutExpired,
    ) as error:
        print(f"Changelog generation failed: {error}", file=sys.stderr)
        return 1
    except subprocess.CalledProcessError as error:
        print(f"Changelog generation failed: {_command_error(error)}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
