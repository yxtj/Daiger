set(HEADERS
	LocalHolder.hpp
	RemoteHolder.hpp
	GlobalHolder.hpp
	GraphHolder.hpp
)
set(SOURCES
	GraphHolder.cpp
)
add_library(holder
	${HEADERS} ${SOURCES})
