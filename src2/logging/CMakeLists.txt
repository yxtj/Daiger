set(HEADERS
	easylogging++.h
	logging.h
)
set(SOURCES
	easylogging++.cc
	logging.cpp
)
add_library(logging
	${HEADERS} ${SOURCES})
target_link_libraries(logging pthread)
