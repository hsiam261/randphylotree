#!/bin/bash 
set -xe

git submodule update --init --recursive

LIB_ROOT=$(pwd)/lib
mkdir -p $LIB_ROOT

cd external-libs/yaml-cpp
mkdir -p build
cd build 

# install yaml-cpp locally in ${LIB_ROOT}/yaml-cpp directory
# ${LIB_ROOT}/yaml-cpp must be in CMAKE_PREFIX_LIST
# for build to work
if [ $1 == "RELEASE" ]
then
	cmake .. -DCMAKE_INSTALL_PREFIX=${LIB_ROOT}/yaml-cpp -DCMAKE_BUILD_TYPE=Release
	cmake --build .
else
	cmake .. -DCMAKE_INSTALL_PREFIX=${LIB_ROOT}/yaml-cpp
	cmake --build . --target install
fi

cd ../../../ 

if [ $1 == "RELEASE" ]
then
	# Generate build files for the "Release" configuration
	mkdir -p release
	cd release
	cmake -DCMAKE_BUILD_TYPE=Release ..
	# Build the project for the "Release" configuration
	make
else
	cmake .
	make
fi

