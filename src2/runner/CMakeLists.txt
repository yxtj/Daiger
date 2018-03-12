set(HEADERS
	Option.h
	runner_helpers.h
	Runner.h
	Master.h
	Worker.h
)
set(SOURCES
	Option.cpp
	runner_helpers.cpp
	Runner.cpp
	Master.cpp
	Master_handler.cpp
	Worker.cpp
	Worker_handler.cpp
)
add_library(runner
	${HEADERS} ${SOURCES})
