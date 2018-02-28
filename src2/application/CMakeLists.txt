set(HEADERS
	AppBase.h
	IOHandler.h
	Scheduler.h
	Sharder.h
	Terminator.h
)
set(SOURCES
	AppBase.cpp
	IOHandler.cpp
	Scheduler.cpp
	Sharder.cpp
	Terminator.cpp
)
add_library(application
	${HEADERS} ${SOURCES})
