set(HEADERS
	FactoryTemplate.h
	ArgumentSeparatorFactory.h
	IOHandlerFactory.h
	SchedulerFactory.h
	SharderFactory.h
	TerminatorFactory.h
)
set(SOURCES
	ArgumentSeparatorFactory.cpp
	IOHandlerFactory.cpp
	SchedulerFactory.cpp
	SharderFactory.cpp
	TerminatorFactory.cpp
)
add_library(application
	${HEADERS} ${SOURCES})
