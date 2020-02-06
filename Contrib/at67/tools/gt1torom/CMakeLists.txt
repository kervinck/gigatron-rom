cmake_minimum_required(VERSION 3.7)

project(gt1torom)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(sources gt1torom.cpp)

add_executable(gt1torom ${sources})

target_link_libraries(gt1torom)

set_target_properties(gt1torom PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)