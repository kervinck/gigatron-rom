
cmake_minimum_required(VERSION 3.16)

project(glcc)
enable_language(C)


# --- Default to RelWithDebInfo

if (NOT "${CMAKE_BUILD_TYPE}")
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()


# --- Only support out-of-directory builds

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "\
Compilation in the source directory is not supported.\
Please create a 'build' directory and run 'cmake SRCDIR' from there.")
endif()


# --- Compute relative path from bindir to srcdir

file(RELATIVE_PATH srcrel "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/foo")
string(REGEX REPLACE "/*foo$" "" srcrel "${srcrel}")


# --- Find Python

find_package(Python3 3.8 REQUIRED QUIET COMPONENTS Interpreter)
set(PYTHON3 "${Python3_EXECUTABLE}" CACHE STRING "Python 3 executable path")
message(STATUS "Using python executable: ${PYTHON3}")


# --- Compile lburg

add_executable(lburg lburg/lburg.c lburg/gram.c)

target_include_directories(lburg PUBLIC lburg)


# --- Run lburg

file(GLOB srcs_md CONFIGURE_DEPENDS
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.md" )

foreach(md ${srcs_md})
  get_filename_component(mdn "${md}" NAME_WLE)
  set(output "${mdn}.c")
  list(APPEND srcs_lburg "${output}")
  add_custom_command(OUTPUT "${output}"
    COMMAND lburg "${srcrel}/src/${md}" "${output}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/src/${md}"
    DEPENDS lburg VERBATIM)
endforeach()


# --- Compile cpp

add_executable(cpp
  cpp/cpp.c cpp/lex.c cpp/nlist.c cpp/tokens.c cpp/macro.c cpp/eval.c
  cpp/include.c cpp/hideset.c cpp/getopt.c cpp/unix.c )

target_include_directories(cpp PUBLIC cpp)


# --- Compile rcc

add_executable(rcc
  src/alloc.c src/bind.c src/dag.c src/decl.c src/enode.c src/error.c
  src/expr.c src/event.c src/init.c src/inits.c src/input.c src/lex.c
  src/list.c src/main.c src/output.c src/prof.c src/profio.c
  src/simp.c src/stmt.c src/string.c src/sym.c src/trace.c src/tree.c
  src/types.c src/null.c src/symbolic.c src/stab.c src/gen.c src/bytecode.c
  ${srcs_lburg} )

target_include_directories(rcc PUBLIC src)

if ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  target_compile_options(rcc PRIVATE "-Wno-psabi")
endif()


# --- Compile lcc

add_executable(lcc
  etc/lcc.c etc/gigatron-lcc.c )

target_compile_definitions(lcc PUBLIC
  "-DTARGET=gigatron")


# --- Compile gtsim

add_executable(gtsim
  gigatron/mapsim/gtsim.c )


# --- Stage binary directory to make glcc usable

set(gigatron_targets
  glcc glink glink.py gt1dump gtprof
  interface.json interface-dev.json roms.json)

foreach(fn ${gigatron_targets})
  set(output "${CMAKE_CURRENT_BINARY_DIR}/${fn}")
  if ("${fn}" STREQUAL gtprof)
     set(fn "mapsim/${fn}")
  endif()
  add_custom_command(OUTPUT "${output}"
    COMMAND "${CMAKE_COMMAND}" "-E" "copy" "${srcrel}/gigatron/${fn}" "${output}"
    MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/gigatron/${fn}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    VERBATIM)
endforeach()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gigatron/glccver.py")
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/gigatron/glccver.py" ver)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/glccver.py" "${ver}")
else()
  set(ver "GLCC-unknown-version")
  execute_process(COMMAND "git" "describe" "--tags"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/glccver.py"
    RESULT_VARIABLE status)
  if ("${status}" STREQUAL "0")
    file(READ "${CMAKE_CURRENT_BINARY_DIR}/glccver.py" ver)
    string(STRIP "${ver}" ver)
  endif()
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/glccver.py" "ver=\"${ver}\"\n")
endif()

if (WIN32)
  foreach(fn glcc.cmd glink.cmd)
    list(APPEND gigatron_targets "${fn}")
    add_custom_command(OUTPUT "${fn}"
      COMMAND "${CMAKE_COMMAND}" "-E" "echo" "@\"${PYTHON3}\" \"%~dp0\\%~n0\" %*" ">" "${fn}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      VERBATIM)
  endforeach()
endif()

add_custom_target(glcc-ready ALL
  COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "${srcrel}/include/gigatron/" "include"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS "cpp" "rcc" "lcc" ${gigatron_targets}
  VERBATIM)

function(copy_target_to_bindir target)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${target}>" "$<TARGET_FILE_NAME:${target}>"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    VERBATIM)
endfunction()

copy_target_to_bindir(cpp)
copy_target_to_bindir(rcc)
copy_target_to_bindir(lcc)
copy_target_to_bindir(gtsim)


# --- Stage map directories

function(add_glcc_map mapname)
  set(map_targets "")
  file(GLOB mapfiles CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/gigatron/"
    "${CMAKE_CURRENT_SOURCE_DIR}/gigatron/${mapname}/*.py")
  foreach(fn ${mapfiles})
    list(APPEND map_targets "${fn}")
    add_custom_command(OUTPUT ${fn}
      COMMAND "${CMAKE_COMMAND}" "-E" "copy" "${srcrel}/gigatron/${fn}" "${fn}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/gigatron/${fn}"
      VERBATIM )
  endforeach()
  add_custom_target("${mapname}" ALL
    DEPENDS ${map_targets} VERBATIM)
endfunction()

file(GLOB gigatron_maps CONFIGURE_DEPENDS
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/gigatron/"
  "${CMAKE_CURRENT_SOURCE_DIR}/gigatron/map*" )
foreach(map ${gigatron_maps})
  add_glcc_map("${map}")
endforeach()


# ---- Compile gigatron libraries

# The previous version of add_glcc_library used to rely on
# add_custom_{command/target} to cross-compile the libraries. This
# works very well with the Makefile generators, but fails with the
# Visual Studio generator because only one of the command is
# executed. Whether this is a cmake bug or a msbuild bug is unclear.
#
# The following version takes a completely different approach.
# It builds a cmake script to cross-compile the library.
# This is less efficient but maybe more reliable.

function(add_glcc_library libname libfile)
  # Parse the CPU x optional argument
  set(cpulist 4 5 6 7)
  list(GET ARGN 0 car)
  if ("${car}" STREQUAL "CPU")
    list(POP_FRONT ARGN car cpulist)
  endif()
  # Prepare the script
  set(srclist "")
  set(objlist "")
  set(objdir "CMakeFiles/${libname}.glccdir")
  set(script "${objdir}/${libname}.cmake")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${objdir}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${script}" "
file(REMOVE \"${libfile}\")
set(glcccmd \".\\\\glcc.cmd\")
if (NOT EXISTS \"\${glcccmd}\")
  set(glcccmd \"./glcc\")
endif()" )
  if ("${CMAKE_GENERATOR}" MATCHES "Makefile")
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/${script}" "
string(ASCII 27 esc)
set(COLORcyan \"\${esc}[36m\")
set(COLORreset \"\${esc}[m\")")
  endif()
  # Iterate over sources
  foreach(fnp ${ARGN})
    file(GLOB fn_list CONFIGURE_DEPENDS
      RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${fnp}")
    foreach(fn ${fn_list})
      get_filename_component(fname "${fn}" NAME_WLE)
      get_filename_component(fnext "${fn}" LAST_EXT)
      list(APPEND srclist "${fn}")
      if ("${fnext}" STREQUAL ".c")
  	  foreach(cpu ${cpulist})
  	    set(obj "${objdir}/${fname}_${cpu}.o")
	    list(APPEND objlist "${obj}")
	    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/${script}" "
if (\"${srcrel}/${fn}\" IS_NEWER_THAN \"${obj}\")
  message(\"\${COLORcyan}Cross-compiling ${obj}\${COLORreset}\")
  execute_process(COMMAND \"\${glcccmd}\" \"-c\" \"-cpu=${cpu}\" \"-o\" \"${obj}\" \"${srcrel}/${fn}\"
                  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
                  RESULT_VARIABLE status)
  if (NOT \"\${status}\" EQUAL 0)
     message(FATAL_ERROR \"Cross-compilation failed: \${status}\")
  endif()
endif()
file(READ \"${obj}\" contents)
file(APPEND \"${libfile}\" \"\${contents}\")" )
	  endforeach()
      elseif ("${fnext}" STREQUAL ".s")
	    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/${script}" "
file(READ \"${srcrel}/${fn}\" contents)
file(APPEND \"${libfile}\" \"\${contents}\")" )
      else()
  	message(FATAL_ERROR "Library file ${fname}${fext} has an unrecognized suffix")
      endif()
    endforeach()
  endforeach()
  # Create target
  add_custom_target(${libname} ALL
    COMMAND "${CMAKE_COMMAND}" } "-P" "${script}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    BYPRODUCTS ${objlist}
    SOURCES ${srclist}
    DEPENDS glcc-ready
    VERBATIM)
endfunction()
  
add_glcc_library("libconx" "mapconx/libconx.a" "gigatron/mapconx/libconx/*.c")
add_glcc_library("libcon_b" "map512k/libcon_b.a" "gigatron/map512k/libcon_b/*.c" "gigatron/map512k/libcon_b/*.s")
add_glcc_library("libcon_n" "map512k/libcon_n.a" "gigatron/map512k/libcon_n/*.c" "gigatron/map512k/libcon_n/*.s")
add_glcc_library("libcon_h" "map512k/libcon_h.a" "gigatron/map512k/libcon_h/*.c" "gigatron/map512k/libcon_h/*.s")
add_glcc_library("libcon1" "map128k/libcon1.a" "gigatron/map128k/libcon1/*.c" "gigatron/map128k/libcon1/*.s")
add_glcc_library("libsim" "mapsim/libsim.a" "gigatron/mapsim/libsim/*.c"  "gigatron/mapsim/libsim/*.s")
add_glcc_library("libc-cpu4" "cpu4/libc.a" CPU 4 "gigatron/libc/*.c" "gigatron/libc/*.s" "gigatron/runtime/*.s")
add_glcc_library("libc-cpu5" "cpu5/libc.a" CPU 5 "gigatron/libc/*.c" "gigatron/libc/*.s" "gigatron/runtime/*.s")
add_glcc_library("libc-cpu6" "cpu6/libc.a" CPU 6 "gigatron/libc/*.c" "gigatron/libc/*.s" "gigatron/runtime/*.s") 
add_glcc_library("libc-cpu7" "cpu7/libc.a" CPU 7 "gigatron/libc/*.c" "gigatron/libc/*.s" "gigatron/runtime/*.s") 


# ---- Installation

set(glcc_install_libdir "lib/gigatron-lcc")
set(glcc_install_bindir "bin")

install(TARGETS cpp rcc lcc gtsim
  RUNTIME DESTINATION "${glcc_install_libdir}")

foreach(fn ${gigatron_targets})
  get_filename_component(ext "${fn}" EXT)
  if ("${ext}" STREQUAL ".json")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${fn}"
      DESTINATION "${glcc_install_libdir}")
  else()
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${fn}"
      DESTINATION "${glcc_install_libdir}")
  endif()
endforeach()

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/glccver.py"
  DESTINATION "${glcc_install_libdir}")

foreach(dn include cpu4 cpu5 cpu6 ${gigatron_maps})
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dn}"
    DESTINATION "${glcc_install_libdir}")
endforeach()

if (WIN32)
  file(RELATIVE_PATH bin_to_lib "/${glcc_install_bindir}" "/${glcc_install_libdir}")
  string(REPLACE "/" "\\\\" cmd_to_lib "${bin_to_lib}")
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
             \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${glcc_install_bindir}\")")
  foreach(pgm glcc glink gtsim gtprof)
    set(pycmd "")
    if (NOT "${pgm}" STREQUAL gtsim)
      set(pycmd "\\\"${PYTHON3}\\\" ")
    endif()
    set(output "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${glcc_install_bindir}/${pgm}.cmd")
    install(CODE "message(STATUS \"Installing: ${output}\")
                  execute_process(COMMAND ${CMAKE_COMMAND} -E echo
                                     \"@${pycmd}\\\"%~dp0\\\\${cmd_to_lib}\\\\%~n0\\\" %*\"
                                  OUTPUT_FILE \"${output}\") ")
  endforeach()
else()
  file(RELATIVE_PATH bin_to_lib "/${glcc_install_bindir}" "/${glcc_install_libdir}")
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
            \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${glcc_install_bindir}\")")
  foreach(pgm glcc glink gtsim gtprof gt1dump)
    set(output "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${glcc_install_bindir}/${pgm}")
    install(CODE "message(STATUS \"Creating symlink: ${output}\")
                  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
                     \"${bin_to_lib}/${pgm}\" \"${output}\" )")
  endforeach()
endif()
