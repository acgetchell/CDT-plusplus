from __future__ import absolute_import, division, print_function

# Import comet
from comet_ml import Experiment
from comet_ml import Optimizer

# Run command line programs
from subprocess import Popen
import shlex
from subprocess import check_output as qx

# Create an optimizer for dynamic parameters
optimizer = Optimizer(api_key="dLk4aZE8CUKshNvnZUesTP7QV")
params = """
init_radius real [1, 10] [1.0]
foliation_spacing real [1, 10] [1.0]
"""

optimizer.set_params(params)

# Create an experiment with api key
experiment = Experiment(api_key="dLk4aZE8CUKshNvnZUesTP7QV", project_name="cdt-plusplus", team_name="ucdavis")

import os
import tensorflow as tf
import tensorflow.contrib.eager as tfe

tf.enable_eager_execution()

print('TensorFlow version: {}'.format(tf.VERSION))

hyper_params = {'simplices': 12000, 'foliations': 12}
experiment.log_multiple_params(hyper_params)

command_line = "../build/initialize --s -n" + str(experiment.get_parameter("simplices")) \
               + " -t" + str(experiment.get_parameter("foliations"))
args = shlex.split(command_line)

print(args)

process = Popen(args)

# process = Popen(["../build/initialize", "--s", "-n16000", "-t11"])
(output, err) = process.communicate()
exit_code = process.wait()

print(output)
