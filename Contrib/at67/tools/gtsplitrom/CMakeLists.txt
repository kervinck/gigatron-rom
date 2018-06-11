cmake_minimum_required(VERSION 3.7)

project(gtsplitrom)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)

set(headers ../../cpu.h)
set(sources ../../cpu.cpp gtsplitrom.cpp)

add_executable(gtsplitrom ${headers} ${sources})

target_link_libraries(gtsplitrom)