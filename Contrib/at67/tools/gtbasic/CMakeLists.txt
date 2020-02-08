cmake_minimum_required(VERSION 3.7)

project(gtbasic)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})

add_definitions(-DSTAND_ALONE)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()    

set(headers ../../memory.h ../../loader.h ../../cpu.h ../../assembler.h ../../compiler.h ../../operators.h ../../keywords.h ../../optimiser.h ../../validater.h ../../linker.h)
set(sources ../../memory.cpp ../../loader.cpp ../../cpu.cpp ../../image.cpp ../../expression.cpp ../../assembler.cpp ../../compiler.cpp ../../operators.cpp ../../keywords.cpp ../../optimiser.cpp ../../validater.cpp ../../linker.cpp gtbasic.cpp)

add_executable(gtbasic ${headers} ${sources})

target_link_libraries(gtbasic)

set_target_properties(gtbasic PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ..)