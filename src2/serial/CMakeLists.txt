set(HEADERS
	serialization.h
	serialize_general.hpp
	serializer_basic.hpp
	serializer_iter.hpp
	tuple3_ps.h
)
set(SOURCES

)
add_library(serial
	${HEADERS} ${SOURCES})