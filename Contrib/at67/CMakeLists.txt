cmake_minimum_required(VERSION 3.7)

project(gtemuSDL)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

add_subdirectory(midi)
add_subdirectory(tools/gtasm)
add_subdirectory(tools/gtsplitrom)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

file(GLOB sources *.cpp)
file(GLOB headers *.h)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_executable(gtemuSDL WIN32 dirent/dirent.h inih/INIReader.h ${headers} ${sources})
else()
    add_executable(gtemuSDL inih/INIReader.h ${headers} ${sources})
endif()

target_link_libraries(gtemuSDL ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY})