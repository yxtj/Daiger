set(HEADERS
	RPCInfo.h
	Task.h
	NetworkThread.h
	NetworkImplMPI.h
)
set(SOURCES
	Task.cpp
	NetworkThread.cpp
	NetworkImplMPI.cpp
)
add_library(network
	${HEADERS} ${SOURCES})
target_include_directories(network PRIVATE ${MPI_CXX_INCLUDE_PATH})

set_target_properties(network PROPERTIES
    LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
target_link_libraries(network ${MPI_CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

