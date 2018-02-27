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
