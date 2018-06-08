cmake_minimum_required(VERSION 3.7)

project(gtasm)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)

set(headers ../../cpu.h ../../loader.h ../../assembler.h ../../expression.h )
set(sources ../../cpu.cpp ../../loader.cpp ../../assembler.cpp ../../expression.cpp gtasm.cpp)

add_executable(gtasm ${headers} ${sources})

target_link_libraries(gtasm)