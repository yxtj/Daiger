set(HEADERS
	common.h
	file.h
	hash.h
	marshal.hpp
	marshalled_map.hpp
	timer.h
)
set(SOURCES
	common.cc 
	file.cc 
	timer.cc
	static-initializers.cc
)
add_library(util2
	${HEADERS} ${SOURCES})