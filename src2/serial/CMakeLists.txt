set(HEADERS
	serialization.h
	serialize_general.hpp
	serializer_basic.hpp
	serializer_iter.hpp
	tuple3_ps.h
)
set(SOURCES
)
add_library(serial INTERFACE)
#	${HEADERS} ${SOURCES})
target_include_directories(serial INTERFACE ./)
