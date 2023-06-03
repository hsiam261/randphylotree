#!/bin/bash 

mkdir -p lib/yaml-cpp 

cd external-libs/yaml-cpp
mkdir -p build
cd build 

cmake .. -DCMAKE_INSTALL_PREFIX=../../lib/yaml-cpp
cmake --build . --target install

cd ../../../ 
cmake .
make

