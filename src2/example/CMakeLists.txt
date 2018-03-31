set(HEADERS
    cc.h
    pr.h
    mc.h
    sssp.h
    sswp.h
    example_reg.h
)
set(SOURCES
    cc.cpp
    pr.cpp
    mc.cpp
    sssp.cpp
    sswp.cpp
    example_reg.cpp
)
add_library(example
	${HEADERS} ${SOURCES})
