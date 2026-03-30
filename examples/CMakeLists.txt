if(EVENTENGINE_BUILD_STATIC)
    set(EVENTENGINE_EXAMPLE_LINK_TARGET eventengine-cpp_static)
else()
    set(EVENTENGINE_EXAMPLE_LINK_TARGET eventengine-cpp_shared)
endif()

add_executable(test_version test_version.cpp)
target_link_libraries(test_version PRIVATE ${EVENTENGINE_EXAMPLE_LINK_TARGET})

add_executable(test_timer test_timer.cpp)
target_link_libraries(test_timer PRIVATE ${EVENTENGINE_EXAMPLE_LINK_TARGET})

add_executable(test_thread_pool test_thread_pool.cpp)
target_link_libraries(test_thread_pool PRIVATE ${EVENTENGINE_EXAMPLE_LINK_TARGET})
