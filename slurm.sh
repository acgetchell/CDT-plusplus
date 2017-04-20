#!/bin/bash -l
# For use on Slurm
# sbatch -p high -t 60 slurm.sh
module load cmake gcc boost cgal tbb
rm -rf build/
mkdir build && cd build
cmake -G Ninja -DGMOCK_TESTS:BOOL=OFF -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
mkdir $HOME/data/`date +%Y-%m-%d.%R%Z`
cp cdt-opt $HOME/data/`date +%Y-%m-%d.%R%Z`/
cd $HOME/data/`date +%Y-%m-%d.%R%Z`
./cdt-opt 2>>errors 1>>output

