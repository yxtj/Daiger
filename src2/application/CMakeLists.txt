set(HEADERS
	IOHandler.h
	Sharder.h
	Terminator.h
)
set(SOURCES
	IOHandler.cpp
	Sharder.cpp
	Terminator.cpp
)
add_library(application
	${HEADERS} ${SOURCES})
