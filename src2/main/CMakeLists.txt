add_executable(daiger main.cpp)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_link_libraries(daiger
		application common driver example factory holder network runner serial type_traits util logging
	)
else()
	target_link_libraries(daiger
		-Wl,-whole-archive application common driver example factory holder network runner serial type_traits util logging
		-Wl,-no-whole-archive
	)
endif()
target_link_libraries(daiger ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${MPI_CXX_LIBRARIES})
