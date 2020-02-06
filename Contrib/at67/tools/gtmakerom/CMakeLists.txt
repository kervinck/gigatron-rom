cmake_minimum_required(VERSION 3.7)

project(gtmakerom)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(headers ../../cpu.h)
set(sources ../../cpu.cpp gtmakerom.cpp)

add_executable(gtmakerom ${headers} ${sources})

target_link_libraries(gtmakerom)

set_target_properties(gtmakerom PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)