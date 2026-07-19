#!/bin/bash -l
# For use on Slurm
# sbatch -p high -t 60 slurm.sh
# Or, interactively,
# srun -p med2 -t 1-00 --mem=100G --ntasks 12 --pty /bin/bash -il
set -eo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/build"

module load cmake/3.28.1
module load gcc/13.2.0
module load autoconf-archive/2022.02.11
rm -rf -- "${build_dir}"
cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -D ENABLE_TESTING:BOOL=TRUE -S "${repo_root}" -B "${build_dir}"
cmake --build "${build_dir}"
ctest --test-dir "${build_dir}" --no-tests=error --output-on-failure -j2
#mkdir "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)"
#cp cdt-opt "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)"/
#cd "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)" || exit
#./cdt-opt 2>>errors 1>>output
