cmake_minimum_required(VERSION 3.15)
project(afl)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Werror=return-type -Wall -O2")

include_directories(include)

set(LIB_FILES lib/bffnt.cpp lib/bntx.cpp lib/byml/reader.cpp lib/byml/writer.cpp lib/sarc.cpp lib/szs.cpp lib/util.cpp lib/yaz0.cpp)
set(SRC_FILES src/main.cpp)
set(TEST_FILES tests/main.cpp)

add_executable(main ${SRC_FILES} ${LIB_FILES})
add_executable(test ${TEST_FILES} ${LIB_FILES})
