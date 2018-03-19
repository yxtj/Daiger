cmake_minimum_required (VERSION 3.0)

project(Daiger)

#include(../cmake/FindProtobuf.cmake)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(CheckFunctionExists)

# -------- Boost --------
find_package(Boost REQUIRED COMPONENTS filesystem system program_options)
include_directories(${Boost_INCLUDE_DIRS}) 

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

find_package(MPI REQUIRED)

#find_program(HAVE_MPICC "mpic++")
#if (NOT HAVE_MPICC) 
if (NOT MPI_CXX_FOUND)
  #message(FATAL_ERROR "Required OpenMPI development libraries and compiler not found")
  message(FATAL_ERROR "Required MPI development libraries and compiler not found")
endif()

#exec_program("mpic++ -showme:compile" OUTPUT_VARIABLE MPI_CXX_COMPILE_FLAGS)
#exec_program("mpic++ -showme:incdirs" OUTPUT_VARIABLE MPI_CXX_INCLUDE_PATH)
#exec_program("mpic++ -showme:link" OUTPUT_VARIABLE MPI_CXX_LINK_FLAGS)
#exec_program("mpic++ -showme:libdirs" OUTPUT_VARIABLE MPI_LIBDIRS)
#exec_program("mpic++ -showme:libs" OUTPUT_VARIABLE MPI_CXX_LIBRARIES)

#string(REPLACE " " ";" MPI_INCDIRS "${MPI_INCDIRS}")
#string(REPLACE " " ";" MPI_LINK_FLAGS "${MPI_LINK_FLAGS}")
#string(REPLACE " " ";" MPI_LIBDIRS "${MPI_LIBDIRS}")
#string(REPLACE " " ";" MPI_LIBS "${MPI_LIBS}")

# -------- compress (LZO) --------
#check_include_file("/usr/include/lzo/lzo1x.h" HAVE_LZO1X_H)
#if (NOT HAVE_LZO1X_H)
#  message(FATAL_ERROR "Required lzo header file not found.")
#endif()

 
set(CXX_DEFAULT "${CMAKE_CXX_FLAGS}")
set(CXX_DEFINE "-DHAVE_LIB_GFLAGS ")
set(CXX_WARN "-Wall -Wno-sign-compare -Wno-unused-function -Wno-unused-result")
 
if ("$ENV{OPROFILE}")
  message(STATUS "Enabling OProfile")
endif()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set( ARCH_BITS 64 )
else()
	set( ARCH_BITS 32 )
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CXX_DEFAULT} -fPIC -fno-omit-frame-pointer ${CXX_DEFINE}")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
#                     ${MAITER_BINARY_DIR}
#                     ${PROTOBUF_INCLUDE_DIRS}
                     ${MPI_CXX_INCLUDE_PATH}
                   )
include_directories(SYSTEM)

set(CMAKE_CXX_FLAGS "${CXX_DEFAULT} -fPIC -fno-omit-frame-pointer -std=c++11 ${CXX_WARN} ${CXX_DEFINE} ${MPI_CXX_COMPILE_FLAGS}")
add_subdirectory (dbg)
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
