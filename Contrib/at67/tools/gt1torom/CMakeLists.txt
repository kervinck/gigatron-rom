cmake_minimum_required(VERSION 3.7)

project(gt1torom)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)

set(sources gt1torom.cpp)

if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(gt1torom ${sources})

if(MSVC)
    set_target_properties(gt1torom PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)
endif()

target_link_libraries(gt1torom)
