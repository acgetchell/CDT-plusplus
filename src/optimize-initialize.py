# Usage: python optimize-initialize.py
#
from __future__ import absolute_import, division, print_function

import os
import traceback

# Import Comet.ml
from comet_ml import Experiment
# from comet_ml import Optimizer
import comet_ml as cm

# Import TensorFlow
# import tensorflow as tf
# import tensorflow.contrib.eager as tfe
import matplotlib.pyplot as plt
import numpy as np

# Run command line programs
import shlex
from subprocess import check_output as qx
import re

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

try:
    # while True:
        # Get a suggestion
        # suggestion = optimizer.get_suggestion()
    for parameter_pair in parameters:

        # Create an experiment with api key
        experiment = Experiment(api_key=os.environ['COMET_API_KEY'], project_name="cdt-plusplus", team_name="ucdavis")

        # print('TensorFlow version: {}'.format(tf.VERSION))

        hyper_params = {'simplices': 12000, 'foliations': 11}
        experiment.log_multiple_params(hyper_params)
        # init_radius = suggestion["initial_radius"]
        init_radius = parameter_pair[0]
        # radial_factor = suggestion["foliation_spacing"]
        radial_factor = parameter_pair[1]

        command_line = "../build/initialize --s -n" + str(experiment.get_parameter("simplices")) \
            + " -t" + str(experiment.get_parameter("foliations")) + " -i" + str(init_radius) \
            + " -f" + str(radial_factor)
        args = shlex.split(command_line)

        print(args)

        output = qx(args)

        # Parse output into a list of [simplices, min timeslice, max timeslice]
        result = [0, 0, 0]
        graph = []
        for line in output.splitlines():
            if line.startswith("Minimum timevalue"):
                s = re.findall('\d+', line)
                result[1] = float(s[0])
            elif line.startswith("Maximum timevalue"):
                s = re.findall('\d+', line)
                result[2] = float(s[0])
            elif line.startswith("Final number"):
                # print(line)
                s = re.findall('\d+', line)
                # print(s)
                result[0] = float(s[0])
            elif line.startswith("Timeslice"):
                t = re.findall('\d+', line)
                graph.append(t)

        print(result)
        print('Initial radius is: {}'.format(init_radius))
        print('Radial factor is: {}'.format(radial_factor))
        for element in graph:
            print("Timeslice {} has {} spacelike faces.".format(element[0], element[1]))
        print("")

        # Score model
        score = ((result[0] - experiment.get_parameter("simplices"))/(experiment.get_parameter("simplices")))*100

        # Report results
        experiment.log_metric("Error %", score)
        experiment.log_other("Min Timeslice", result[1])
        experiment.log_other("Max Timeslice", result[2])

        # Graph volume profile
        timeslice = []
        volume = []
        for element in graph:
            timeslice.append(int(element[0]))
            volume.append(int(element[1]))
        plt.plot(timeslice, volume)
        plt.xlabel('Timeslice')
        plt.ylabel('Volume (spacelike faces)')
        plt.title('Volume Profile')
        plt.grid(True)
        experiment.log_figure(figure_name="Volume per Timeslice", figure=plt)
        plt.clf()


except cm.exceptions.NoMoreSuggestionsAvailable as NoMore:
    print("No more suggestions.")

except (TypeError, KeyError):
    pass

finally:
    traceback.print_exc()

print("All done with parameter optimization, look at Comet.ml for results.")
