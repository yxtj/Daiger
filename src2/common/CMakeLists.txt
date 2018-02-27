set(HEADERS
	def.h
	Node.h
	Kernel.h
	io.h
)
set(SOURCES
	io.cpp
)
add_library(common
	${HEADERS} ${SOURCES})
