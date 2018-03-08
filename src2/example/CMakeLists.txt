set(HEADERS
    cc.h
    example_reg.h
)
set(SOURCES
    cc.cpp
    example_reg.cpp
)
add_library(example
	${HEADERS} ${SOURCES})
