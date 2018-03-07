set(HEADERS
	LocalHolder.hpp
	RemoteHolder.hpp
)
set(SOURCES

)
add_library(holder INTERFACE)
#	${HEADERS} ${SOURCES})
target_include_directories(holder INTERFACE ./)
