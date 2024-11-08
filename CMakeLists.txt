cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_CXX_STANDARD 23)
project(afl)

include_directories(include)

# add_subdirectory("lib/byml")

add_executable(main src/main.cpp lib/bffnt.cpp lib/bntx.cpp lib/byml/reader.cpp lib/byml/writer.cpp lib/sarc.cpp lib/szs.cpp lib/util.cpp lib/yaz0.cpp)
