cmake_minimum_required(VERSION 3.7)

project(gtasm)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)

set(headers ../../memory.h ../../loader.h ../../assembler.h ../../expression.h)
set(sources ../../memory.cpp ../../loader.cpp ../../cpu.cpp ../../assembler.cpp ../../expression.cpp gtasm.cpp)

if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(gtasm ${headers} ${sources})

if(MSVC)
    set_target_properties(gtasm PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)
endif()

target_link_libraries(gtasm)
