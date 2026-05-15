cmake_minimum_required(VERSION 3.15)
project(mizuna)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(HAKKUN_CORE_CONFIG_FILE ${CMAKE_CURRENT_LIST_DIR}/config/config.cmake)

include_directories(include)
include_directories(lib)
include_directories(lib/hk/hakkun/core/include)

add_library(mizuna)

add_subdirectory(lib/hk/hakkun/core)
add_subdirectory(src)
add_subdirectory(lib/tinygltf)
