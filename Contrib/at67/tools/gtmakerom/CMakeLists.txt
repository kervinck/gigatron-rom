cmake_minimum_required(VERSION 3.7)

project(gtmakerom)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)

set(headers ../../cpu.h)
set(sources ../../cpu.cpp gtmakerom.cpp)

add_executable(gtmakerom ${headers} ${sources})

target_link_libraries(gtmakerom)