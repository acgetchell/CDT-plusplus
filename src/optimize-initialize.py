# Causal Dynamical Triangulations in C++ using CGAL
#
# Copyright © 2018 Adam Getchell
#
# A program that optimizes spacetime generation parameters

# @file optimize-initialize.py
# @brief Optimize spacetime generation
# @author Adam Getchell

# Usage: python optimize-initialize.py
#

"""Run the legacy CDT++ parameter optimization experiment."""

import os
import re
from pathlib import Path
from subprocess import check_output as qx

# from comet_ml import Optimizer
import comet_ml as cm

# Import TensorFlow
# import tensorflow as tf
# import tensorflow.contrib.eager as tfe
import matplotlib.pyplot as plt
import numpy as np

# Import Comet.ml
from comet_ml import Experiment

# Create an optimizer for dynamic parameters
# optimizer = Optimizer(api_key=os.environ['COMET_API_KEY'])
# params = """
# initial_radius integer [1, 2] [1]
# foliation_spacing integer [1, 2] [1]
# """
#
# optimizer.set_params(params)

# tf.enable_eager_execution()

# Create parameters to vary
parameters = [(initial_radius, spacing) for initial_radius in range(1, 4) for spacing in np.arange(1, 2.5, 0.5)]

repository_root = Path(__file__).resolve().parents[1]
initialize_binary = repository_root / "out/build/reference/src/initialize"
if not initialize_binary.is_file():
    missing_binary_message = f"CDT++ initializer not found at {initialize_binary}; run `just build` first."
    raise SystemExit(missing_binary_message)

api_key = os.environ.get("COMET_API_KEY")
if not api_key:
    missing_api_key_message = "COMET_API_KEY is required for an online Comet experiment."
    raise SystemExit(missing_api_key_message)

try:
    # while True:
    # Get a suggestion
    # suggestion = optimizer.get_suggestion()
    for parameter_pair in parameters:
        # Create an experiment with api key
        experiment = Experiment(api_key=api_key, project_name="cdt-plusplus")

        # print('TensorFlow version: {}'.format(tf.VERSION))

        hyper_params = {"simplices": 12000, "foliations": 12}
        experiment.log_multiple_params(hyper_params)
        # init_radius = suggestion["initial_radius"]
        init_radius = parameter_pair[0]
        # radial_factor = suggestion["foliation_spacing"]
        radial_factor = parameter_pair[1]

        args = [
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

        print(args)

        # The argument vector is assembled entirely from repository-controlled
        # executable and numeric experiment parameters.
        output = qx(args, text=True)  # noqa: S603

        final_simplices = None
        graph: list[tuple[int, int]] = []
        for line in output.splitlines():
            if match := re.fullmatch(r"Final number of simplices: (?P<count>\d+)", line):
                final_simplices = int(match.group("count"))
            elif line.startswith("Timeslice"):
                match = re.fullmatch(r"Timeslice (?P<timeslice>\d+) has (?P<volume>\d+) spacelike faces\.", line)
                if match:
                    graph.append((int(match.group("timeslice")), int(match.group("volume"))))

        if final_simplices is None:
            missing_simplices_message = "Initializer output did not report the final number of simplices."
            raise RuntimeError(missing_simplices_message)
        if not graph:
            missing_profile_message = "Initializer output did not contain a timeslice volume profile."
            raise RuntimeError(missing_profile_message)

        min_timeslice = min(timeslice for timeslice, _ in graph)
        max_timeslice = max(timeslice for timeslice, _ in graph)
        result = (final_simplices, min_timeslice, max_timeslice)

        print(result)
        print(f"Initial radius is: {init_radius}")
        print(f"Radial factor is: {radial_factor}")
        for timeslice, volume in graph:
            print(f"Timeslice {timeslice} has {volume} spacelike faces.")
        print()

        # Score model
        target_simplices = hyper_params["simplices"]
        score = ((final_simplices - target_simplices) / target_simplices) * 100

        # Report results
        experiment.log_metric("Error %", score)
        experiment.log_other("Min Timeslice", result[1])
        experiment.log_other("Max Timeslice", result[2])

        # Graph volume profile
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

print("All done with parameter optimization, look at Comet.ml for results.")
