# Usage: python optimize-initialize.py
#
from __future__ import absolute_import, division, print_function

# import os
import traceback

# Import Comet.ml
from comet_ml import Experiment
from comet_ml import Optimizer
import comet_ml as cm

# Import TensorFlow
import tensorflow as tf
# import tensorflow.contrib.eager as tfe

# Run command line programs
import shlex
from subprocess import check_output as qx
import re

# Create an optimizer for dynamic parameters
optimizer = Optimizer(api_key="dLk4aZE8CUKshNvnZUesTP7QV")
params = """
init_radius integer [1, 2] [1]
foliation_spacing integer [1, 2] [1]
"""

optimizer.set_params(params)

tf.enable_eager_execution()

try:
    while True:
        # Get a suggestion
        suggestion = optimizer.get_suggestion()

        # Create an experiment with api key
        experiment = Experiment(api_key="dLk4aZE8CUKshNvnZUesTP7QV", project_name="cdt-plusplus", team_name="ucdavis")

        print('TensorFlow version: {}'.format(tf.VERSION))

        hyper_params = {'simplices': 12000, 'foliations': 12}
        experiment.log_multiple_params(hyper_params)
        init_radius = suggestion["init_radius"]
        foliation_spacing = suggestion["foliation_spacing"]

        command_line = "../build/initialize --s -n" + str(experiment.get_parameter("simplices")) \
               + " -t" + str(experiment.get_parameter("foliations")) + " -i" + str(init_radius) \
               + " -f" + str(foliation_spacing)
        args = shlex.split(command_line)

        print(args)

        output = qx(args)

        # Parse output into a list of [simplices, min timeslice, max timeslice]
        result = [0, 0, 0]
        for line in output.splitlines():
            if line.startswith("Final number"):
                # print(line)
                s = re.findall('\d+', line)
                # print(s)
                result[0] = float(s[0])
            elif line.startswith("Minimum timevalue"):
                s = re.findall('\d+', line)
                result[1] = float(s[0])
            elif line.startswith("Maximum timevalue"):
                s = re.findall('\d+', line)
                result[2] = float(s[0])

        print(result)
        print('Initial radius is: {}'.format(init_radius))
        print('Foliation spacing is: {}'.format(foliation_spacing))
        print("")

        # Score model
        score = ((result[0] - experiment.get_parameter("simplices"))/(experiment.get_parameter("simplices")))*100
        suggestion.report_score("%Error", score)
        experiment.log_other("Min Timeslice", result[1])
        experiment.log_other("Max Timeslice", result[2])

except cm.exceptions.NoMoreSuggestionsAvailable as NoMore:
    print("No more suggestions.")

except (TypeError, KeyError):
    pass

finally:
    traceback.print_exc()

print("All done with parameter optimization, look at Comet.ml for results.")
