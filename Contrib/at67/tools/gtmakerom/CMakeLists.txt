cmake_minimum_required(VERSION 3.7)

project(gtmakerom)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)

set(headers ../../cpu.h)
set(sources ../../cpu.cpp gtmakerom.cpp)

if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(gtmakerom ${headers} ${sources})

if(MSVC)
    set_target_properties(gtmakerom PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)
endif()

target_link_libraries(gtmakerom)
