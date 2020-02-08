cmake_minimum_required(VERSION 3.7)

project(gtasm)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(headers ../../memory.h ../../loader.h ../../assembler.h ../../expression.h)
set(sources ../../memory.cpp ../../loader.cpp ../../cpu.cpp ../../assembler.cpp ../../expression.cpp gtasm.cpp)

add_executable(gtasm ${headers} ${sources})

target_link_libraries(gtasm)

set_target_properties(gtasm PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)