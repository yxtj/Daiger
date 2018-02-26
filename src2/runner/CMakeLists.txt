set(HEADERS
	Option.h
	Worker.h
	Master.h
)
set(SOURCES
	main.cpp
	Option.cpp
	Worker.cpp
	Master.cpp
)
add_library(runner
	${HEADERS} ${SOURCES})
