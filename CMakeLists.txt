cmake_minimum_required(VERSION 3.15)
project(afl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_library(afl)

add_subdirectory(lib/afl)
add_subdirectory(lib/tinygltf)
