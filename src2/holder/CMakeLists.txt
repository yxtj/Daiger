set(HEADERS
	LocalHolder.hpp
	RemoteHolder.hpp
	GlobalHolder.hpp
	GraphContainer.h
)
set(SOURCES
	GraphContainer.cpp
)
add_library(holder
	${HEADERS} ${SOURCES})
