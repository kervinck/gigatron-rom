cmake_minimum_required(VERSION 3.7)

project(gtemuAT67)

set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_BUILD_TYPE "Release")

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

# Pi4 and Pi400
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")
    add_compile_options(-mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8)
endif()

add_subdirectory(tools/gtasm)
add_subdirectory(tools/gtmidi)
add_subdirectory(tools/gtbasic)
add_subdirectory(tools/gt1torom)
add_subdirectory(tools/gtmakerom)
add_subdirectory(tools/gtsplitrom)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

file(GLOB headers *.h)
file(GLOB sources *.cpp)
set(headers ${headers})
set(sources ${sources})

if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_executable(gtemuAT67 WIN32 dirent/dirent.h inih/INIReader.h rs232/rs232.h ${headers} rs232/rs232-win.c ${sources})
else()
    add_compile_options(-Wall -Wextra -pedantic)
    add_executable(gtemuAT67 inih/INIReader.h rs232/rs232.h ${headers} rs232/rs232-linux.c ${sources})
endif()

target_link_libraries(gtemuAT67 ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY})