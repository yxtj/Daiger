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
set(_TEMPORTARY_CXX_FLAGS, ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${_TEMPORTARY_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
add_library(network
	${HEADERS} ${SOURCES})
target_link_libraries(network ${MPI_CXX_LIBRARIES})
set(CMAKE_CXX_FLAGS, ${_TEMPORTARY_CXX_FLAGS})

