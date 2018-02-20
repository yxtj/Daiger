set(HEADERS
	type_trais_dummy.h
	is_container.h
	is_stl_container.h
	is_sequential_container.h
	is_associated_container.h
	is_iterator.h
	is_pair.h
	is_tuple.h

	Timer.h
	Util.h
)
set(SOURCES
	Timer.cpp
	Util.cpp
)
add_library(util
	${HEADERS} ${SOURCES})