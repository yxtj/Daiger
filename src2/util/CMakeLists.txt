set(HEADERS
	Timer.h
	Util.h
)
set(SOURCES
	Timer.cpp
	Util.cpp
)
add_library(util
	${HEADERS} ${SOURCES})