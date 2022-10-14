#!/bin/bash -l
# For use on Slurm
# sbatch -p high -t 60 slurm.sh
module load spack/cmake
module load spack/gcc
module load spack/autoconf-archive
cd ..
rm -rf build/
cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -D ENABLE_TESTING:BOOL=TRUE -S . -B build
cmake --build build
cd build || exit
ctest --output-on-failure -j2
#mkdir "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)"
#cp cdt-opt "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)"/
#cd "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)" || exit
#./cdt-opt 2>>errors 1>>output