cmake_minimum_required(VERSION 3.7)

project(gtmidi)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(gtmidi ${headers} ${sources})

target_link_libraries(gtmidi)