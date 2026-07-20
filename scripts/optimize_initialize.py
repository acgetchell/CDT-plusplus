"""Run the CDT++ initializer parameter optimization experiment."""

from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path
from subprocess import check_output as qx
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Sequence


def _parse_args(argv: Sequence[str]) -> argparse.Namespace:
    """Parse command-line arguments without loading experiment dependencies."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path.cwd(),
        help="CDT++ checkout containing out/build/reference (default: current directory)",
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
            if match:
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


def _run_experiments(initialize_binary: Path, api_key: str) -> None:
    """Run the historical Comet parameter sweep."""
    import comet_ml as cm  # noqa: PLC0415
    import matplotlib.pyplot as plt  # noqa: PLC0415
    import numpy as np  # noqa: PLC0415
    from comet_ml import Experiment  # noqa: PLC0415

    parameters = [(initial_radius, spacing) for initial_radius in range(1, 4) for spacing in np.arange(1, 2.5, 0.5)]

    try:
        for parameter_pair in parameters:
            experiment = Experiment(api_key=api_key, project_name="cdt-plusplus")
            hyper_params = {"simplices": 12000, "foliations": 12}
            experiment.log_multiple_params(hyper_params)
            init_radius = parameter_pair[0]
            radial_factor = parameter_pair[1]

            command = [
                str(initialize_binary),
                "-s",
                "-n",
                str(hyper_params["simplices"]),
                "-t",
                str(hyper_params["foliations"]),
                "-i",
                str(init_radius),
                "-f",
                str(radial_factor),
            ]
            print(command)

            # The executable and numeric parameters are repository-controlled.
            output = qx(command, text=True)  # noqa: S603

            final_simplices, graph = _parse_initializer_output(output)

            min_timeslice = min(timeslice for timeslice, _ in graph)
            max_timeslice = max(timeslice for timeslice, _ in graph)
            result = (final_simplices, min_timeslice, max_timeslice)

            print(result)
            print(f"Initial radius is: {init_radius}")
            print(f"Radial factor is: {radial_factor}")
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
            plt.plot(timeslices, volumes)
            plt.xlabel("Timeslice")
            plt.ylabel("Volume (spacelike faces)")
            plt.title("Volume Profile")
            plt.grid(visible=True)
            experiment.log_figure(figure_name="Volume per Timeslice", figure=plt)
            plt.clf()
            experiment.end()
    except cm.exceptions.NoMoreSuggestionsAvailable:
        print("No more suggestions.")


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
        _run_experiments(initialize_binary, api_key)
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
