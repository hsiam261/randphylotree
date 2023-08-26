#!/bin/bash 
set -xe

git submodule update --init --recursive

cd external-libs/yaml-cpp
mkdir -p build
cd build 

if [ $1 == "RELEASE" ]
then
	cmake .. -DCMAKE_INSTALL_PREFIX=../../lib/yaml-cpp -DCMAKE_BUILD_TYPE=Release
	cmake --build .
else
	cmake .. -DCMAKE_INSTALL_PREFIX=../../lib/yaddml-cpp
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

