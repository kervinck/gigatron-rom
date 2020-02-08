cmake_minimum_required(VERSION 3.7)

project(gtmidi)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(headers ../../cpu.h ../../memory.h ../../expression.h)
set(sources ../../expression.cpp gtmidi.cpp)

add_executable(gtmidi ${headers} ${sources})

target_link_libraries(gtmidi)

set_target_properties(gtmidi PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)