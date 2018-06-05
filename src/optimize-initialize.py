from __future__ import absolute_import, division, print_function

import os

# Import Comet.ml
from comet_ml import Experiment
from comet_ml import Optimizer

# Import TensorFlow
import tensorflow as tf
import tensorflow.contrib.eager as tfe

# Run command line programs
import shlex
from subprocess import check_output as qx
import re

# Create an optimizer for dynamic parameters
optimizer = Optimizer(api_key="dLk4aZE8CUKshNvnZUesTP7QV")
params = """
init_radius real [1, 10] [1.0]
foliation_spacing real [1, 10] [1.0]
"""

optimizer.set_params(params)

# Create an experiment with api key
experiment = Experiment(api_key="dLk4aZE8CUKshNvnZUesTP7QV", project_name="cdt-plusplus", team_name="ucdavis")


tf.enable_eager_execution()

print('TensorFlow version: {}'.format(tf.VERSION))

hyper_params = {'simplices': 12000, 'foliations': 12}
experiment.log_multiple_params(hyper_params)

command_line = "../build/initialize --s -n" + str(experiment.get_parameter("simplices")) \
               + " -t" + str(experiment.get_parameter("foliations"))
args = shlex.split(command_line)

# print(args)

output = qx(args)

# Parse output for final simplices, which is the label
result = 0
for line in output.splitlines():
    if line.startswith("Final number"):
        # print(line)
        s = re.findall('\d+', line)
        # print(s)
        result = float(s[0])

print(result)
