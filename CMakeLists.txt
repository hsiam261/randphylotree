cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_PREFIX_PATH "lib/yaml-cpp" ${CMAKE_PREFIX_PATH})
find_package(yaml-cpp REQUIRED)

project(generate-random-tree VERSION 1.0)
add_executable(generate-random-tree src/main.cpp)
target_link_libraries(generate-random-tree yaml-cpp)

