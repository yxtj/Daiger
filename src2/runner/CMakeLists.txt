set(HEADERS
	Option.h
	IdTranslate.h
	Worker.h
	Master.h
)
set(SOURCES
	main.cpp
	Option.cpp
	IdTranslate.cpp
	Worker.cpp
	Master.cpp
)
add_library(runner
	${HEADERS} ${SOURCES})
