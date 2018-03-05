set(HEADERS
	type_trais_dummy.h
	is_container.h
	is_stl_container.h
	is_sequential_container.h
	is_associated_container.h
	is_iterator.h
	is_pair.h
	is_tuple.h
)
set(SOURCES

)
add_library(type_traits
	${HEADERS} ${SOURCES})
