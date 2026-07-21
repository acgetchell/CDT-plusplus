"""Run the CDT++ initializer parameter optimization experiment."""

from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path
from subprocess import check_output as qx
from typing import TYPE_CHECKING, Protocol

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

    def clf(self) -> object:
        """Clear the current figure."""
        ...


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


def _run_parameter_sweep(
    initialize_binary: Path,
    seed: int,
    experiment_factory: Callable[[], _Experiment],
    initializer_runner: Callable[[list[str]], str],
    plotter: _Plotter,
) -> None:
    """Run the parameter sweep through injected initializer and Comet boundaries."""
    for initial_radius, foliation_spacing in PARAMETER_PAIRS:
        experiment = experiment_factory()
        try:
            hyper_params = {"simplices": 12000, "foliations": 12, "seed": seed}
            experiment.log_parameters(
                {
                    **hyper_params,
                    "initial_radius": initial_radius,
                    "foliation_spacing": foliation_spacing,
                }
            )

            command = _initializer_command(
                initialize_binary,
                hyper_params,
                initial_radius=initial_radius,
                foliation_spacing=foliation_spacing,
            )
            print(command)

            output = initializer_runner(command)
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
            experiment.log_metric("Error %", score)
            experiment.log_other("Min Timeslice", result[1])
            experiment.log_other("Max Timeslice", result[2])

            timeslices = [timeslice for timeslice, _ in graph]
            volumes = [volume for _, volume in graph]
            plotter.plot(timeslices, volumes)
            plotter.xlabel("Timeslice")
            plotter.ylabel("Volume (spacelike faces)")
            plotter.title("Volume Profile")
            plotter.grid(visible=True)
            experiment.log_figure(figure_name="Volume per Timeslice", figure=plotter)
            plotter.clf()
        finally:
            experiment.end()


def _run_experiments(initialize_binary: Path, api_key: str, seed: int) -> None:
    """Run the historical Comet parameter sweep."""
    import matplotlib.pyplot as plt  # noqa: PLC0415
    from comet_ml import Experiment  # noqa: PLC0415

    def experiment_factory() -> _Experiment:
        return Experiment(api_key=api_key, project_name="cdt-plusplus")

    def initializer_runner(command: list[str]) -> str:
        # The executable and numeric parameters are repository-controlled.
        return qx(command, text=True)  # noqa: S603

    _run_parameter_sweep(initialize_binary, seed, experiment_factory, initializer_runner, plt)


def main(argv: Sequence[str] | None = None) -> int:
    """Run the parameter sweep from an installed uv entry point."""
    args = _parse_args(sys.argv[1:] if argv is None else argv)
    repository_root = args.repository_root.resolve()
    initialize_binary = _initializer_binary(repository_root)
    if not initialize_binary.is_file():
        print(f"CDT++ initializer not found at {initialize_binary}; run `just build` first.", file=sys.stderr)
        return 2

    api_key = os.environ.get("COMET_API_KEY")
    if not api_key:
        print("COMET_API_KEY is required for an online Comet experiment.", file=sys.stderr)
        return 2

    try:
        _run_experiments(initialize_binary, api_key, args.seed)
    except ModuleNotFoundError as error:
        print(
            f"Missing experiment dependency {error.name!r}; run with `uv run --group experiments cdt-optimize-initialize`.",
            file=sys.stderr,
        )
        return 2

    print("All done with parameter optimization; results are available in Comet.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
