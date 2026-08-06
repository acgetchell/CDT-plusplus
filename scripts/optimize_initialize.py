"""Run the CDT++ initializer parameter optimization experiment."""

import argparse
import hashlib
import os
import re
import shutil
import sys
from dataclasses import dataclass
from importlib.metadata import PackageNotFoundError, version
from pathlib import Path
from subprocess import check_output as qx
from typing import TYPE_CHECKING, Literal, Protocol, cast

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


class _Experiment(Protocol):
    """Comet operations used by the historical parameter sweep."""

    def log_parameters(self, parameters: Mapping[str, int | float]) -> object:
        """Record the inputs used by one initializer run."""
        ...

    def log_metric(self, name: str, value: float) -> object:
        """Record a numeric result."""
        ...

    def log_other(self, name: str, value: int) -> object:
        """Record non-metric result metadata."""
        ...

    def log_figure(self, *, figure_name: str, figure: object) -> object:
        """Record the volume profile plot."""
        ...

    def end(self) -> object:
        """Close the online experiment."""
        ...


class _Plotter(Protocol):
    """Matplotlib operations used by the historical parameter sweep."""

    def plot(self, x_values: list[int], y_values: list[int]) -> object:
        """Plot the volume profile."""
        ...

    def xlabel(self, label: str) -> object:
        """Label the horizontal axis."""
        ...

    def ylabel(self, label: str) -> object:
        """Label the vertical axis."""
        ...

    def title(self, label: str) -> object:
        """Set the plot title."""
        ...

    def grid(self, *, visible: bool) -> object:
        """Configure the plot grid."""
        ...

    def savefig(self, filename: Path) -> object:
        """Save the canonical local volume-profile figure."""
        ...

    def clf(self) -> object:
        """Clear the current figure."""
        ...


@dataclass(frozen=True)
class _SweepServices:
    """Injected hosted, process, and plotting boundaries for one sweep."""

    experiment_factory: Callable[[Path], _Experiment | None]
    initializer_runner: Callable[[list[str]], str]
    plotter: _Plotter


@dataclass(frozen=True)
class _SweepConfig:
    """Inputs needed to reproduce one initializer parameter sweep."""

    api_key: str | None
    comet_mode: Literal["disabled", "offline", "online"]
    initialize_binary: Path
    output_directory: Path
    repository_root: Path
    seed: int


def _parse_seed(value: str) -> int:
    """Parse one unsigned 64-bit seed before creating online experiments."""
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
    """Parse command-line arguments without loading experiment dependencies."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--comet",
        choices=("disabled", "offline", "online"),
        default="disabled",
        help="optional Comet mirror; local artifacts are always canonical (default: disabled)",
    )
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


def _initializer_binary(repository_root: Path, platform: str = sys.platform) -> Path:
    """Return the reference initializer path for the active operating system."""
    executable = "initialize.exe" if platform == "win32" else "initialize"
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


def _mirror_to_comet(action: str, operation: Callable[..., object], /, *args: object, **kwargs: object) -> bool:
    """Attempt one optional Comet operation without invalidating local output."""
    try:
        operation(*args, **kwargs)
    except Exception as error:  # noqa: BLE001 - The optional mirror cannot invalidate canonical local artifacts.
        print(f"Comet mirror failed while {action}: {error}", file=sys.stderr)
        return False
    return True


def _run_parameter_sweep(
    initialize_binary: Path,
    seed: int,
    output_directory: Path,
    provenance: Mapping[str, object],
    services: _SweepServices,
) -> bool:
    """Run the parameter sweep through injected initializer and Comet boundaries."""
    mirror_succeeded = True
    with staged_run_directory(output_directory) as staged_output_directory:
        for initial_radius, foliation_spacing in PARAMETER_PAIRS:
            try:
                experiment = services.experiment_factory(staged_output_directory)
            except Exception as error:  # noqa: BLE001 - The optional mirror cannot invalidate canonical local artifacts.
                print(f"Comet mirror failed while starting the experiment: {error}", file=sys.stderr)
                mirror_succeeded = False
                experiment = None
            try:
                hyper_params = {"simplices": 12000, "foliations": 12, "seed": seed}
                parameters = {
                    **hyper_params,
                    "initial_radius": initial_radius,
                    "foliation_spacing": foliation_spacing,
                }
                if experiment is not None:
                    mirror_succeeded &= _mirror_to_comet("logging parameters", experiment.log_parameters, parameters)

                command = _initializer_command(
                    initialize_binary,
                    hyper_params,
                    initial_radius=initial_radius,
                    foliation_spacing=foliation_spacing,
                )
                print(command)
                run_directory = staged_output_directory / f"radius-{initial_radius}-spacing-{foliation_spacing:g}"
                run_directory.mkdir(parents=True, exist_ok=True)
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
                result = (final_simplices, min_timeslice, max_timeslice)

                print(result)
                print(f"Initial radius is: {initial_radius}")
                print(f"Foliation spacing is: {foliation_spacing}")
                for timeslice, volume in graph:
                    print(f"Timeslice {timeslice} has {volume} spacelike faces.")
                print()

                target_simplices = hyper_params["simplices"]
                score = ((final_simplices - target_simplices) / target_simplices) * 100
                if experiment is not None:
                    mirror_succeeded &= _mirror_to_comet("logging the error metric", experiment.log_metric, "Error %", score)
                    mirror_succeeded &= _mirror_to_comet("logging the minimum timeslice", experiment.log_other, "Min Timeslice", result[1])
                    mirror_succeeded &= _mirror_to_comet("logging the maximum timeslice", experiment.log_other, "Max Timeslice", result[2])

                timeslices = [timeslice for timeslice, _ in graph]
                volumes = [volume for _, volume in graph]
                figure_path = run_directory / "volume-profile.png"
                mirror_succeeded &= _write_volume_profile(timeslices, volumes, figure_path, services, experiment)

                write_json(
                    run_directory / "run.json",
                    {
                        "artifacts": {
                            "configuration": artifact_record(configuration_path, run_directory),
                            "figure": artifact_record(figure_path, run_directory),
                            "stdout": artifact_record(stdout_path, run_directory),
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
            finally:
                if experiment is not None:
                    mirror_succeeded &= _mirror_to_comet("ending the experiment", experiment.end)
    return mirror_succeeded


def _write_volume_profile(
    timeslices: list[int],
    volumes: list[int],
    figure_path: Path,
    services: _SweepServices,
    experiment: _Experiment | None,
) -> bool:
    """Write and optionally mirror one parameter pair's volume profile."""
    mirror_succeeded = True
    try:
        services.plotter.plot(timeslices, volumes)
        services.plotter.xlabel("Timeslice")
        services.plotter.ylabel("Volume (spacelike faces)")
        services.plotter.title("Volume Profile")
        services.plotter.grid(visible=True)
        services.plotter.savefig(figure_path)
        if experiment is not None:
            mirror_succeeded = _mirror_to_comet(
                "logging the volume profile",
                experiment.log_figure,
                figure_name="Volume per Timeslice",
                figure=services.plotter,
            )
    finally:
        services.plotter.clf()
    return mirror_succeeded


def _run_experiments(config: _SweepConfig) -> bool:
    """Run the local-first parameter sweep with an optional Comet mirror."""
    import matplotlib.pyplot as plt  # noqa: PLC0415

    if config.comet_mode != "disabled":
        import comet_ml  # noqa: PLC0415

    def experiment_factory(artifact_directory: Path) -> _Experiment | None:
        if config.comet_mode == "disabled":
            return None
        if config.comet_mode == "online":
            experiment_config = comet_ml.ExperimentConfig(
                log_env_details=False,
                log_env_network=False,
                log_git_metadata=False,
                log_git_patch=False,
                name="cdt-optimize-initialize",
            )
            return cast(
                "_Experiment",
                comet_ml.start(api_key=config.api_key, experiment_config=experiment_config, mode="create", project_name="cdt-plusplus"),
            )
        offline_directory = artifact_directory / "comet"
        offline_directory.mkdir(parents=True, exist_ok=True)
        experiment_config = comet_ml.ExperimentConfig(
            log_env_details=False,
            log_env_network=False,
            log_git_metadata=False,
            log_git_patch=False,
            name="cdt-optimize-initialize",
            offline_directory=str(offline_directory),
        )
        return cast(
            "_Experiment",
            comet_ml.start(
                experiment_config=experiment_config,
                mode="create",
                online=False,
                project_name="cdt-plusplus",
            ),
        )

    def initializer_runner(command: list[str]) -> str:
        # The executable and numeric parameters are repository-controlled.
        return qx(command, text=True)  # noqa: S603

    services = _SweepServices(experiment_factory=experiment_factory, initializer_runner=initializer_runner, plotter=cast("_Plotter", plt))
    provenance = _experiment_provenance(config.repository_root, config.initialize_binary)
    return _run_parameter_sweep(config.initialize_binary, config.seed, config.output_directory, provenance, services)


def main(argv: Sequence[str] | None = None) -> int:
    """Run the parameter sweep from an installed uv entry point."""
    args = _parse_args(sys.argv[1:] if argv is None else argv)
    repository_root = args.repository_root.resolve()
    output_directory = args.output_directory.resolve()
    initialize_binary = _initializer_binary(repository_root)
    if not initialize_binary.is_file():
        print(f"CDT++ initializer not found at {initialize_binary}; run `just build` first.", file=sys.stderr)
        return 2

    api_key = os.environ.get("COMET_API_KEY")
    if args.comet == "online" and not api_key:
        print("COMET_API_KEY is required when --comet online is selected.", file=sys.stderr)
        return 2

    try:
        config = _SweepConfig(
            api_key=api_key,
            comet_mode=args.comet,
            initialize_binary=initialize_binary,
            output_directory=output_directory,
            repository_root=repository_root,
            seed=args.seed,
        )
        mirror_succeeded = _run_experiments(config)
    except ModuleNotFoundError as error:
        print(
            f"Missing experiment dependency {error.name!r}; run `just python-sync-experiments`, then retry with `uv run --no-sync cdt-optimize-initialize`.",
            file=sys.stderr,
        )
        return 2
    except OutputDirectoryExistsError as error:
        print(str(error), file=sys.stderr)
        return 2

    print(f"All done with parameter optimization; canonical local artifacts: {output_directory}")
    if args.comet != "disabled":
        if mirror_succeeded:
            print("The run was also mirrored to Comet.")
        else:
            print("Comet mirroring was incomplete; canonical local artifacts were retained.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
