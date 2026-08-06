"""Shared local-artifact helpers for CDT++ Python workflows."""

import hashlib
import json
import os
import shutil
import tempfile
from contextlib import contextmanager
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Iterator, Mapping

PACKAGE_NAME = "cdt-plusplus-scripts"


class OutputDirectoryExistsError(ValueError):
    """The requested canonical output directory already exists."""


def sha256(path: Path) -> str:
    """Hash one local experiment input or artifact."""
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def artifact_record(path: Path, root: Path) -> dict[str, object]:
    """Describe one canonical artifact by stable relative path and digest."""
    return {
        "bytes": path.stat().st_size,
        "path": path.relative_to(root).as_posix(),
        "sha256": sha256(path),
    }


def write_json(path: Path, payload: Mapping[str, object]) -> None:
    """Write one deterministic canonical local experiment artifact."""
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(f"{json.dumps(payload, allow_nan=False, indent=2, sort_keys=True)}\n", encoding="utf-8")


def staging_directory_prefix(output_directory: Path) -> str:
    """Return the temporary-directory prefix for one staged run."""
    return f".{output_directory.name}.incomplete-"


@contextmanager
def staged_run_directory(output_directory: Path) -> Iterator[Path]:
    """Publish one complete run without mixing it with an older generation."""
    if os.path.lexists(output_directory):
        message = f"Output directory already exists: {output_directory}; choose a new --output-directory."
        raise OutputDirectoryExistsError(message)

    output_directory.parent.mkdir(parents=True, exist_ok=True)
    staging_directory = Path(
        tempfile.mkdtemp(
            dir=output_directory.parent,
            prefix=staging_directory_prefix(output_directory),
        )
    )
    try:
        yield staging_directory
        staging_directory.rename(output_directory)
    except BaseException:
        shutil.rmtree(staging_directory, ignore_errors=True)
        raise
