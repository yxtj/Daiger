set(HEADERS
	RPCInfo.h
	Task.h
	NetworkThread.h
	NetworkImplMPI.h
)
set(SOURCES
	Task.cc
	NetworkThread.cc
	NetworkImplMPI.cc
)
add_library(network
	${HEADERS} ${SOURCES})
