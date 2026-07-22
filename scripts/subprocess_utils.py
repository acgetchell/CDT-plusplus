"""Secure subprocess helpers for repository support scripts."""

from __future__ import annotations

import shutil
import subprocess
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Mapping, Sequence
    from pathlib import Path

DEFAULT_TIMEOUT_SECONDS = 30.0


class ExecutableNotFoundError(RuntimeError):
    """Report that a required support-tool executable is unavailable."""


def _resolve_executable(command: str) -> str:
    """Resolve one executable from ``PATH`` or fail with an actionable error."""
    executable = shutil.which(command)
    if executable is None:
        message = f"Required executable {command!r} was not found in PATH."
        raise ExecutableNotFoundError(message)
    return executable


def run_git_command(
    arguments: Sequence[str],
    *,
    cwd: Path,
    input_text: str | None = None,
) -> subprocess.CompletedProcess[str]:
    """Run Git without a shell using deterministic text-mode I/O."""
    return run_support_command(
        "git",
        arguments,
        cwd=cwd,
        input_text=input_text,
    )


def run_support_command(
    command: str,
    arguments: Sequence[str],
    *,
    cwd: Path,
    environment: Mapping[str, str] | None = None,
    input_text: str | None = None,
) -> subprocess.CompletedProcess[str]:
    """Run one resolved support command without a shell."""
    executable = _resolve_executable(command)
    return subprocess.run(  # noqa: S603 - executable is resolved explicitly and arguments are never passed through a shell.
        [executable, *arguments],
        cwd=cwd,
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
        env=environment,
        input=input_text,
        timeout=DEFAULT_TIMEOUT_SECONDS,
    )
