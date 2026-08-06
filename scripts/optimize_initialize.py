"""Run the local CDT++ initializer parameter sweep."""

import argparse
import hashlib
import re
import shutil
import sys
from dataclasses import dataclass
from importlib.metadata import PackageNotFoundError, version
from pathlib import Path
from subprocess import check_output as qx
from typing import TYPE_CHECKING

from scripts.experiment_artifacts import (
    PACKAGE_NAME,
    OutputDirectoryExistsError,
    artifact_record,
    sha256,
    staged_run_directory,
    write_json,
)

if TYPE_CHECKING:
    from collections.abc import Callable, Mapping, Sequence

MAX_RANDOM_SEED = (1 << 64) - 1
PARAMETER_PAIRS = tuple((initial_radius, spacing) for initial_radius in range(1, 4) for spacing in (1.0, 1.5, 2.0))


@dataclass(frozen=True)
class _SweepServices:
    """Injected process boundary for one local sweep."""

    initializer_runner: Callable[[list[str]], str]


@dataclass(frozen=True)
class _SweepConfig:
    """Inputs needed to reproduce one initializer parameter sweep."""

    initialize_binary: Path
    output_directory: Path
    repository_root: Path
    seed: int


def _parse_seed(value: str) -> int:
    """Parse one unsigned 64-bit initializer seed."""
    try:
        seed = int(value, 10)
    except ValueError as error:
        message = "seed must be an unsigned 64-bit integer"
        raise argparse.ArgumentTypeError(message) from error
    if seed < 0 or seed > MAX_RANDOM_SEED:
        message = "seed must be between 0 and 18446744073709551615"
        raise argparse.ArgumentTypeError(message)
    return seed


def _parse_args(argv: Sequence[str]) -> argparse.Namespace:
    """Parse the dependency-free local sweep command line."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-directory",
        type=Path,
        default=Path("out/experiments/initialize"),
        help="canonical local run-artifact directory (default: out/experiments/initialize)",
    )
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path.cwd(),
        help="CDT++ checkout containing out/build/reference (default: current directory)",
    )
    parser.add_argument(
        "--seed",
        type=_parse_seed,
        default=92,
        help="root initializer seed used for every parameter pair (default: 92)",
    )
    return parser.parse_args(argv)


def _parse_initializer_output(output: str) -> tuple[int, list[tuple[int, int]]]:
    """Extract the final size and volume profile from initializer output."""
    final_simplices = None
    graph: list[tuple[int, int]] = []
    for line in output.splitlines():
        if match := re.fullmatch(r"Final number of simplices: (?P<count>\d+)", line):
            final_simplices = int(match.group("count"))
        elif line.startswith("Timeslice"):
            match = re.fullmatch(r"Timeslice (?P<timeslice>\d+) has (?P<volume>\d+) spacelike faces[.]", line)
            if match is None:
                message = f"Initializer output contained a malformed timeslice volume: {line!r}"
                raise RuntimeError(message)
            graph.append((int(match.group("timeslice")), int(match.group("volume"))))

    if final_simplices is None:
        message = "Initializer output did not report the final number of simplices."
        raise RuntimeError(message)
    if not graph:
        message = "Initializer output did not contain a timeslice volume profile."
        raise RuntimeError(message)
    return final_simplices, graph


def _initializer_binary(repository_root: Path, platform_name: str = sys.platform) -> Path:
    """Return the reference initializer path for the active operating system."""
    executable = "initialize.exe" if platform_name == "win32" else "initialize"
    return repository_root / "out" / "build" / "reference" / "src" / executable


def _initializer_command(
    initialize_binary: Path,
    hyper_params: Mapping[str, int],
    initial_radius: int,
    foliation_spacing: float,
) -> list[str]:
    """Build one replayable initializer invocation."""
    return [
        str(initialize_binary),
        "-s",
        "-n",
        str(hyper_params["simplices"]),
        "-t",
        str(hyper_params["foliations"]),
        "-i",
        str(initial_radius),
        "-f",
        str(foliation_spacing),
        "--seed",
        str(hyper_params["seed"]),
    ]


def _experiment_provenance(repository_root: Path, initialize_binary: Path) -> dict[str, object]:
    """Identify the exact executable, source state, and Python script used."""
    resolved_root = repository_root.resolve()
    resolved_binary = initialize_binary.resolve()
    git = shutil.which("git")
    if git is None:
        message = "Git is required to record initializer source provenance."
        raise RuntimeError(message)
    commit = qx([git, "-C", str(resolved_root), "rev-parse", "HEAD"], text=True).strip()  # noqa: S603
    status = qx(  # noqa: S603
        [git, "-C", str(resolved_root), "status", "--porcelain=v1", "--untracked-files=all"],
        text=True,
    )
    tracked_diff = qx([git, "-C", str(resolved_root), "diff", "--binary", "HEAD"])  # noqa: S603
    try:
        package_version = version(PACKAGE_NAME)
    except PackageNotFoundError:
        package_version = "uninstalled"
    return {
        "initializer": {
            "bytes": resolved_binary.stat().st_size,
            "path": resolved_binary.relative_to(resolved_root).as_posix(),
            "sha256": sha256(resolved_binary),
        },
        "repository": {
            "commit": commit,
            "dirty": bool(status.strip()),
            "tracked_diff_sha256": hashlib.sha256(tracked_diff).hexdigest(),
        },
        "script": {
            "package": PACKAGE_NAME,
            "package_version": package_version,
            "sha256": sha256(Path(__file__)),
        },
    }


def _write_volume_profile(path: Path, graph: Sequence[tuple[int, int]]) -> None:
    """Write a portable, dependency-free table for one volume profile."""
    rows = ["timeslice\tvolume", *(f"{timeslice}\t{volume}" for timeslice, volume in graph)]
    path.write_text(f"{'\n'.join(rows)}\n", encoding="utf-8")


def _run_parameter_sweep(
    initialize_binary: Path,
    seed: int,
    output_directory: Path,
    provenance: Mapping[str, object],
    services: _SweepServices,
) -> None:
    """Run the parameter sweep and retain only canonical local artifacts."""
    with staged_run_directory(output_directory) as staged_output_directory:
        for initial_radius, foliation_spacing in PARAMETER_PAIRS:
            hyper_params = {"simplices": 12000, "foliations": 12, "seed": seed}
            parameters = {
                **hyper_params,
                "initial_radius": initial_radius,
                "foliation_spacing": foliation_spacing,
            }
            command = _initializer_command(
                initialize_binary,
                hyper_params,
                initial_radius=initial_radius,
                foliation_spacing=foliation_spacing,
            )
            print(command)
            run_directory = staged_output_directory / f"radius-{initial_radius}-spacing-{foliation_spacing:g}"
            run_directory.mkdir(parents=True)
            configuration_path = run_directory / "configuration.json"
            write_json(
                configuration_path,
                {
                    "command": command,
                    "parameters": parameters,
                    "provenance": provenance,
                },
            )

            output = services.initializer_runner(command)
            stdout_path = run_directory / "stdout.txt"
            stdout_path.write_text(output, encoding="utf-8")
            final_simplices, graph = _parse_initializer_output(output)
            min_timeslice = min(timeslice for timeslice, _ in graph)
            max_timeslice = max(timeslice for timeslice, _ in graph)
            target_simplices = hyper_params["simplices"]
            score = ((final_simplices - target_simplices) / target_simplices) * 100
            profile_path = run_directory / "volume-profile.tsv"
            _write_volume_profile(profile_path, graph)

            print((final_simplices, min_timeslice, max_timeslice))
            print(f"Initial radius is: {initial_radius}")
            print(f"Foliation spacing is: {foliation_spacing}")
            print()
            write_json(
                run_directory / "run.json",
                {
                    "artifacts": {
                        "configuration": artifact_record(configuration_path, run_directory),
                        "stdout": artifact_record(stdout_path, run_directory),
                        "volume_profile": artifact_record(profile_path, run_directory),
                    },
                    "metrics": {
                        "error_percent": score,
                        "final_simplices": final_simplices,
                        "max_timeslice": max_timeslice,
                        "min_timeslice": min_timeslice,
                    },
                    "parameters": parameters,
                    "provenance": provenance,
                    "volume_profile": [{"timeslice": timeslice, "volume": volume} for timeslice, volume in graph],
                },
            )


def _run_experiments(config: _SweepConfig) -> None:
    """Run the local parameter sweep with the repository initializer."""

    def initializer_runner(command: list[str]) -> str:
        return qx(command, text=True)  # noqa: S603 - The command is built from validated repository inputs.

    provenance = _experiment_provenance(config.repository_root, config.initialize_binary)
    _run_parameter_sweep(
        config.initialize_binary,
        config.seed,
        config.output_directory,
        provenance,
        _SweepServices(initializer_runner=initializer_runner),
    )


def main(argv: Sequence[str] | None = None) -> int:
    """Run the parameter sweep from an installed uv entry point."""
    args = _parse_args(sys.argv[1:] if argv is None else argv)
    repository_root = args.repository_root.resolve()
    output_directory = args.output_directory.resolve()
    initialize_binary = _initializer_binary(repository_root)
    if not initialize_binary.is_file():
        print(f"CDT++ initializer not found at {initialize_binary}; run `just build` first.", file=sys.stderr)
        return 2
    try:
        _run_experiments(
            _SweepConfig(
                initialize_binary=initialize_binary,
                output_directory=output_directory,
                repository_root=repository_root,
                seed=args.seed,
            )
        )
    except OutputDirectoryExistsError as error:
        print(str(error), file=sys.stderr)
        return 2
    print(f"All done with parameter optimization; canonical local artifacts: {output_directory}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
