add_executable(daiger main.cpp)
target_link_libraries(daiger
	application common driver example 
	factory holder network
	runner serial type_traits util logging
)
target_link_libraries(daiger ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${MPI_CXX_LIBRARIES})
