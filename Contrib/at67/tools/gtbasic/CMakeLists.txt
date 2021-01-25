cmake_minimum_required(VERSION 3.7)

project(gtbasic)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)

set(headers ../../memory.h ../../loader.h ../../cpu.h ../../assembler.h ../../compiler.h ../../operators.h ../../functions.h ../../keywords.h ../../pragmas.h ../../optimiser.h ../../validater.h ../../linker.h)
set(sources ../../memory.cpp ../../loader.cpp ../../cpu.cpp ../../image.cpp ../../expression.cpp ../../assembler.cpp ../../compiler.cpp ../../operators.cpp ../../functions.cpp ../../keywords.cpp ../../midi.cpp ../../pragmas.cpp ../../optimiser.cpp ../../validater.cpp ../../linker.cpp gtbasic.cpp)

if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(gtbasic ${headers} ${sources})

if(MSVC)
    set_target_properties(gtbasic PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)
endif()

target_link_libraries(gtbasic)
