cmake_minimum_required(VERSION 3.15)

project(afl)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-O2 -Wall -Werror -Wextra -Wno-unused-variable -Wno-missing-field-initializers)
    if (WIN32)
        add_compile_options(-static -static-libgcc -static-libstdc++)
    endif()
endif()

include_directories(include)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/lib/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
