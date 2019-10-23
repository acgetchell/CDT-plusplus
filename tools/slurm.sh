#!/bin/bash -l
# For use on Slurm
# sbatch -p high -t 60 slurm.sh
module load cmake gcc boost cgal tbb
cd ..
rm -rf build/
mkdir build
cd build || exit
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$HOME"/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
mkdir "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)"
cp cdt-opt "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)"/
cd "$HOME"/data/"$(date +%Y-%m-%d.%R%Z)" || exit
./cdt-opt 2>>errors 1>>output