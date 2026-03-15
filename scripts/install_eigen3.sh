#!/bin/bash

# Exit on error
set -e

# Get the path of the script

# Libs prefix
LIB_PREFIX="$(dirname "$(readlink -f "$0")")/../lib"

# Get absolute path of LIB_PREFIX
LIB_PREFIX="$(readlink -f "${LIB_PREFIX}")"

EIGEN3_PREFIX="${LIB_PREFIX}/eigen3"

#  Build Eigen3 from source
wget -c https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
unzip eigen-3.4.0.zip
mv eigen-3.4.0 eigen3_source

# Install Eigen3
mkdir -p eigen3_source/build
cd eigen3_source/build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${EIGEN3_PREFIX} ..
make install -j
cd ../..
rm -rf eigen3_source
rm eigen-3.4.0.zip