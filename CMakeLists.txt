cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_CXX_STANDARD 23)
project(afl)

include_directories(include)

set(LIB_FILES lib/bffnt.cpp lib/bntx.cpp lib/byml/reader.cpp lib/byml/writer.cpp lib/sarc.cpp lib/szs.cpp lib/util.cpp lib/yaz0.cpp)
set(SRC_FILES src/main.cpp)
set(TEST_FILES tests/main.cpp)

add_executable(main ${SRC_FILES} ${LIB_FILES})
add_executable(test ${TEST_FILES} ${LIB_FILES})
