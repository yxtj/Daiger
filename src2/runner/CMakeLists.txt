set(HEADERS
	Option.h
	runner_helpers.h
	Runner.h
	Master.h
	Worker.h
)
set(SOURCES
	main.cpp
	Option.cpp
	runner_helpers.cpp
	Runner.h
	Master.cpp
	Worker.cpp
)
add_library(runner
	${HEADERS} ${SOURCES})
