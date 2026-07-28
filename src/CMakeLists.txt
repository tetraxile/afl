if (MSVC)
    message(FATAL_ERROR "Compiling with MSVC is not supported")
endif()

add_compile_options(mizuna PUBLIC -g -O2 -Wall -Werror -Wextra -Wno-unused-variable -Wno-missing-field-initializers)
if (WIN32)
    add_compile_options(mizuna PUBLIC -static -static-libgcc -static-libstdc++)
endif()

add_subdirectory(bfres)
add_subdirectory(byml)
add_subdirectory(sarc)
add_subdirectory(msbp)
add_subdirectory(msbt)

target_sources(mizuna
    PRIVATE
        bffnt.cpp
        bntx.cpp
        util.cpp
        yaz0.cpp
        lms.cpp
)

