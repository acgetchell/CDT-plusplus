#!/bin/bash -l
# For use on Slurm
# sbatch -p high -t 60 slurm.sh
module load cmake/3.18.0
module load gcc/9.2.0
cd ..
rm -rf build/
cmake -G Ninja -D CMAKE_BUILD_TYPE=RelWithDebInfo -S . -B build
cmake --build build
#mkdir "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)"
#cp cdt-opt "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)"/
#cd "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)" || exit
#./cdt-opt 2>>errors 1>>output