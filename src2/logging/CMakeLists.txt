set(HEADERS
	easylogging++.h
)
set(SOURCES
	easylogging++.cc
)
add_library(log
	${HEADERS} ${SOURCES})
target_link_libraries(log pthread)
