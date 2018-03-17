set(HEADERS
    cc.h
    pr.h
    sssp.h
    example_reg.h
)
set(SOURCES
    cc.cpp
    pr.cpp
    sssp.cpp
    example_reg.cpp
)
add_library(example
	${HEADERS} ${SOURCES})
