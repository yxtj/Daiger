cmake_minimum_required (VERSION 3.1)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 14)
  endif ()
endmacro(use_cxx11)

project(Daiger CXX)

#include(../cmake/FindProtobuf.cmake)

#include(CheckIncludeFile)
#include(CheckIncludeFileCXX)
#include(CheckLibraryExists)
#include(CheckFunctionExists)

use_cxx11()

# -------- Boost --------
find_package(Boost REQUIRED COMPONENTS filesystem system program_options)
include_directories(${Boost_INCLUDE_DIRS}) 
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")

# -------- Python interface --------
#find_package(PythonLibs)
#find_package(SWIG)

# -------- Protobuf --------
#find_package(HAVE_PROTOBUF protobuf)
#find_program(HAVE_PROTOC "protoc")
#if (NOT HAVE_PROTOBUF OR NOT HAVE_PROTOC)
#  message(FATAL_ERROR "Required protocol buffer headers and compiler not found.")
#else()
#  set(PROTOBUF_LIBS protobuf)
#endif()

# -------- MPI --------

set(MPI_EXECUTABLE_SUFFIX ".openmpi")
find_package(MPI REQUIRED)
message(STATUS "MPI: ${MPI_CXX_FOUND} ${MPIEXEC}")
#include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})
# ${MPI_CXX_LINK_FLAGS}     Linking flags for MPI programs
# ${MPI_CXX_LIBRARIES}      All libraries to link MPI programs against
link_directories(${MPI_LIBDIRS})

message(STATUS "mpi-include-path ${MPI_CXX_INCLUDE_PATH}")
message(STATUS "mpi-compile-flags ${MPI_CXX_COMPILE_FLAGS}")
message(STATUS "mpi-link-flags ${MPI_CXX_LINK_FLAGS}")
message(STATUS "mpi-libs ${MPI_CXX_LIBRARIES}")

#exec_program("mpic++ -showme:compile" OUTPUT_VARIABLE MPI_CXX_COMPILE_FLAGS)
#exec_program("mpic++ -showme:incdirs" OUTPUT_VARIABLE MPI_CXX_INCLUDE_PATH)
#exec_program("mpic++ -showme:link" OUTPUT_VARIABLE MPI_CXX_LINK_FLAGS)
#exec_program("mpic++ -showme:libdirs" OUTPUT_VARIABLE MPI_LIBDIRS)
#exec_program("mpic++ -showme:libs" OUTPUT_VARIABLE MPI_CXX_LIBRARIES)

# -------- thread --------

find_package (Threads)
message(STATUS "Thread lib: ${CMAKE_THREAD_LIBS_INIT}")
# use ${CMAKE_THREAD_LISBS_INIT} for targets need thread support.

# -------- main --------

message(STATUS "We are on a ${CMAKE_SYSTEM_NAME} system")
message(STATUS "The host processor is ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "Source dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "Binary dir: ${PROJECT_BINARY_DIR}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
#                     ${PROTOBUF_INCLUDE_DIRS}
                     ${MPI_CXX_INCLUDE_PATH}
                   )
include_directories(SYSTEM)

set(CXX_DEFAULT "${CMAKE_CXX_FLAGS}")
set(CXX_DEFINE "-DELPP_THREAD_SAFE -DELPP_STL_LOGGING -DELPP_LOG_UNORDERED_MAP")
#set(CXX_WARN "-Wall -Wno-sign-compare -Wno-unused-function -Wno-unused-variable -Wno-unused-result")
set(CMAKE_CXX_FLAGS "${CXX_DEFAULT} ${CXX_WARN} ${CXX_DEFINE} ${CMAKE_THREAD_LIBS_INIT}")

#add_subdirectory (dbg)
add_subdirectory (logging)
add_subdirectory (util)
add_subdirectory (common)
add_subdirectory (network)
#add_subdirectory (msg)
add_subdirectory (driver)
add_subdirectory (holder)
add_subdirectory (application)
add_subdirectory (factory)
add_subdirectory (type_traits)
add_subdirectory (serial)
add_subdirectory (example)
add_subdirectory (runner)
add_subdirectory (main)
