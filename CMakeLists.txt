cmake_minimum_required(VERSION 3.15)

project(afl)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-O2 -Wall -Werror -Wextra -Wno-unused-variable)

include_directories(include)

set(SOURCES lib/afl/bffnt.cpp lib/afl/bfres.cpp lib/afl/bntx.cpp lib/afl/byml/reader.cpp lib/afl/byml/writer.cpp lib/afl/sarc.cpp lib/afl/szs.cpp lib/afl/util.cpp lib/afl/yaz0.cpp)


add_library(${PROJECT_NAME} STATIC ${SOURCES})
