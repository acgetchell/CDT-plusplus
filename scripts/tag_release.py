"""Create a CDT++ annotated release tag from ``CHANGELOG.md``."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path
from typing import TYPE_CHECKING

from scripts.subprocess_utils import ExecutableNotFoundError, run_git_command

if TYPE_CHECKING:
    from collections.abc import Sequence

GITHUB_TAG_ANNOTATION_LIMIT = 125_000
REPOSITORY_URL = "https://github.com/acgetchell/CDT-plusplus"

_ALPHANUMERIC_IDENTIFIER = r"(?:(?=[0-9A-Za-z-]*[A-Za-z-])[0-9A-Za-z-]+)"
_SEMVER_RE = re.compile(
    r"^v"
    r"(0|[1-9]\d*)\."
    r"(0|[1-9]\d*)\."
    r"(0|[1-9]\d*)"
    rf"(?:-(?:(?:0|[1-9]\d*)|{_ALPHANUMERIC_IDENTIFIER})"
    rf"(?:\.(?:(?:0|[1-9]\d*)|{_ALPHANUMERIC_IDENTIFIER}))*"
    r")?"
    r"(?:\+[0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*)?$"
)


class ReleaseTagError(RuntimeError):
    """Report a release-tag precondition or changelog failure."""


def validate_semver(tag: str) -> None:
    """Require a complete SemVer tag with the repository's leading ``v``."""
    if _SEMVER_RE.fullmatch(tag) is None:
        message = f"Tag must use SemVer with a leading 'v' (for example, v1.0.0 or v1.0.0-rc1); got {tag!r}."
        raise ReleaseTagError(message)


def _version_header(version: str) -> re.Pattern[str]:
    """Build the accepted level-two changelog heading for ``version``."""
    return re.compile(rf"^##\s+\[?v?{re.escape(version)}\]?(?:$|\s|\()")


def extract_changelog_section(changelog: Path, version: str) -> str:
    """Extract the nonempty body of one release section from a changelog."""
    header = _version_header(version)
    section: list[str] = []
    collecting = False

    for line in changelog.read_text(encoding="utf-8").splitlines():
        if line.startswith("## "):
            if collecting:
                break
            collecting = header.match(line) is not None
            continue
        if collecting:
            section.append(line)

    body = "\n".join(section).strip()
    if not collecting or not body:
        message = f"{changelog} has no nonempty section for {version}; expected a heading such as '## [{version}] - YYYY-MM-DD'."
        raise ReleaseTagError(message)
    return body


def _tag_exists(root: Path, tag: str) -> bool:
    """Return whether the local repository already contains ``tag``."""
    try:
        run_git_command(["rev-parse", "-q", "--verify", f"refs/tags/{tag}"], cwd=root)
    except subprocess.CalledProcessError:
        return False
    return True


def _require_clean_worktree(root: Path) -> None:
    """Reject tagging from a worktree with tracked or untracked changes."""
    status = run_git_command(["status", "--porcelain"], cwd=root).stdout.strip()
    if status:
        message = "The worktree must be clean before creating a release tag."
        raise ReleaseTagError(message)


def _tag_message(tag: str, section: str) -> str:
    """Return the changelog body or a stable reference when it is too large."""
    if len(section.encode("utf-8")) <= GITHUB_TAG_ANNOTATION_LIMIT:
        return section
    return (
        f"Version {tag.removeprefix('v')}\n\nThis release section exceeds GitHub's tag-annotation size limit. See <{REPOSITORY_URL}/blob/{tag}/CHANGELOG.md>.\n"
    )


def _print_preview(tag: str, message: str, *, dry_run: bool) -> None:
    """Print a bounded tag-message preview and the intended effect."""
    lines = message.splitlines()
    preview = lines[:20]
    print(f"Tag message preview for {tag} ({len(message.encode('utf-8')):,} bytes):")
    print("----------------------------------------")
    print("\n".join(preview))
    if len(lines) > len(preview):
        print("... (preview truncated)")
    print("----------------------------------------")
    action = "Would create" if dry_run else "Creating"
    print(f"{action} annotated tag {tag}.")


def create_tag(tag: str, *, root: Path | None = None, dry_run: bool = False) -> None:
    """Validate release inputs and optionally create the annotated Git tag."""
    validate_semver(tag)
    repository = (root or Path.cwd()).resolve()
    changelog = repository / "CHANGELOG.md"
    if not changelog.is_file():
        message = f"{changelog} does not exist. Generate the release changelog first."
        raise ReleaseTagError(message)
    if _tag_exists(repository, tag):
        message = f"Tag {tag!r} already exists; release tags must not be replaced."
        raise ReleaseTagError(message)

    section = extract_changelog_section(changelog, tag.removeprefix("v"))
    message = _tag_message(tag, section)
    _require_clean_worktree(repository)
    _print_preview(tag, message, dry_run=dry_run)

    if dry_run:
        return

    run_git_command(
        ["tag", "-a", tag, "-F", "-", "--cleanup=verbatim"],
        cwd=repository,
        input_text=message,
    )
    print(f"Created annotated tag {tag}.")


def _command_error(error: subprocess.CalledProcessError) -> str:
    """Format one failed subprocess without discarding its diagnostics."""
    details = (error.stderr or error.stdout or "").strip()
    return f"{error}\n{details}" if details else str(error)


def main(argv: Sequence[str] | None = None) -> int:
    """Run the release-tag command-line interface."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tag", help="SemVer release tag, including the leading v")
    parser.add_argument("--dry-run", action="store_true", help="validate and preview the tag without creating it")
    args = parser.parse_args(argv)

    try:
        create_tag(args.tag, dry_run=args.dry_run)
    except (ReleaseTagError, ExecutableNotFoundError, OSError, subprocess.TimeoutExpired) as error:
        print(f"Release tag failed: {error}", file=sys.stderr)
        return 1
    except subprocess.CalledProcessError as error:
        print(f"Release tag failed: {_command_error(error)}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
