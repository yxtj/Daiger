add_executable(daiger main.cpp)
target_link_libraries(daiger
		-Wl,-whole-archive application common dbg driver example factory holder network runner serial type_traits util
		${MPI_LINK_FLAGS}
		-Wl,-no-whole-archive)
target_link_libraries(daiger ${Boost_LIBRARIES})
target_link_libraries(daiger pthread)
