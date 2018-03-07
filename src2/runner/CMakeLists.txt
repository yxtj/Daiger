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
	Runner.h
	Master.cpp
	Worker.cpp
)
add_library(runner
	${HEADERS} ${SOURCES})

add_executable(daiger main.cpp)
target_link_libraries(daiger
		-Wl,-whole-archive application common dbg driver example factory holder network runner serial type_traits util
		-Wl,-no-whole-archive)
target_link_libraries(daiger ${Boost_LIBRARIES})
target_link_libraries(daiger pthread)
