cmake_minimum_required(VERSION 3.7)

project(gtmidi)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

set(headers ../memory.h ../expression.h)
set(sources ../memory.cpp ../expression.cpp gtmidi.cpp)

add_executable(gtmidi ${headers} ${sources})

target_link_libraries(gtmidi)