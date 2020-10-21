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

use_cxx11()

# -------- Boost --------
find_package(Boost REQUIRED COMPONENTS filesystem system program_options)
include_directories(${Boost_INCLUDE_DIRS}) 
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")

# -------- Protobuf --------
#find_package(Protobuf REQUIRED)

#include_directories(${Protobuf_INCLUDE_DIRS})
#include_directories(${CMAKE_CURRENT_BINARY_DIR})

#if(CMAKE_VERSION VERSION_GREATER 3.8.0)
#	set(PROTOBUF_LIBS "protobuf::libprotobuf")
#else()
#	set(PROTOBUF_LIBS "protobuf")
#endif()

#message(STATUS "Protobuf include dir: ${Protobuf_INCLUDE_DIRS}")
#message(STATUS "Protobuf complier: ${Protobuf_PROTOC_EXECUTABLE}")

# -------- MPI --------
#set(MPI_EXECUTABLE_SUFFIX ".openmpi")
find_package(MPI REQUIRED)
message(STATUS "MPI: ${MPI_CXX_FOUND} ${MPIEXEC}")

include_directories(${MPI_CXX_INCLUDE_PATH})
# ${MPI_CXX_LINK_FLAGS}     Linking flags for MPI programs
# ${MPI_CXX_LIBRARIES}      All libraries to link MPI programs against
link_directories(${MPI_LIBDIRS})

message(STATUS "mpi-include-path ${MPI_CXX_INCLUDE_PATH}")
message(STATUS "mpi-compile-flags ${MPI_CXX_COMPILE_FLAGS}")
message(STATUS "mpi-link-flags ${MPI_CXX_LINK_FLAGS}")
message(STATUS "mpi-libs ${MPI_CXX_LIBRARIES}")

# -------- thread --------
find_package (Threads)
message(STATUS "Thread lib: ${CMAKE_THREAD_LIBS_INIT}")
# use ${CMAKE_THREAD_LISBS_INIT} for targets need thread support.

# -------- logging --------
if(USE_GLOG)
    # use glog
    find_package(Gflags)
    find_package(Glog REQUIRED)
    if(${Glog_FOUND})
	    set(GLOG_LIBS "glog::glog")
    else()
	    find_library(GLOG_LIBS "glog")
    endif()
    message(STATUS "gflags: ${Gflags_FOUND}, ${Gflags_INCLUDE_DIRS}, ${Gflags_INCLUDES}, ${Gflags_LIBARIES}, ${Gflags_LIBS}")
    message(STATUS "glog: ${Glog_FOUND}, ${Glog_INCLUDE_DIRS}, ${Glog_INCLUDES}, ${Glog_LIBARIES}, ${Glog_LIBS}, ${GLOG_LIBS}")
    message(STATUS "Logging lib: glog")
else()
    # use easyloggingpp
    set(CXX_DEFINE "-DUSE_ELPP -DELPP_THREAD_SAFE -DELPP_STL_LOGGING -DELPP_LOG_UNORDERED_MAP")
    message(STATUS "Logging lib: easyloggingpp")
endif()

# -------- main --------

message(STATUS "We are on a ${CMAKE_SYSTEM_NAME} system")
message(STATUS "The host processor is ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "Source dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "Binary dir: ${PROJECT_BINARY_DIR}")

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
#                     ${PROTOBUF_INCLUDE_DIRS}
                     ${MPI_CXX_INCLUDE_PATH}
                   )
include_directories(SYSTEM)

set(CXX_DEFAULT "${CMAKE_CXX_FLAGS}")
#set(CXX_WARN "-Wall -Wno-sign-compare -Wno-unused-function -Wno-unused-variable -Wno-unused-result")
set(CMAKE_CXX_FLAGS "${CXX_DEFAULT} ${CXX_WARN} ${CXX_DEFINE} ${CMAKE_THREAD_LIBS_INIT}")

add_subdirectory (src2)
