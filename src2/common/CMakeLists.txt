set(HEADERS
	def.h
	def_func.h
	Node.h
	ConfData.h
)
set(SOURCES
	def_func.cpp
)
add_library(common
	${HEADERS} ${SOURCES})
