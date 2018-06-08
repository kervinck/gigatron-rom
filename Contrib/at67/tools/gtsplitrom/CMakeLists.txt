cmake_minimum_required(VERSION 3.7)

project(gtsplitrom)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

set(sources gtsplitrom.cpp)

add_executable(gtsplitrom ${sources})

target_link_libraries(gtsplitrom)