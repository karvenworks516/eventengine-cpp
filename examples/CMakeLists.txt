if(EVENTENGINE_BUILD_STATIC)
    set(EVENTENGINE_EXAMPLE_LINK_TARGET eventengine-cpp_static)
else()
    set(EVENTENGINE_EXAMPLE_LINK_TARGET eventengine-cpp_shared)
endif()

add_executable(testVersion testVersion.cpp)
target_link_libraries(testVersion PRIVATE ${EVENTENGINE_EXAMPLE_LINK_TARGET})

add_executable(testTimer testTimer.cpp)
target_link_libraries(testTimer PRIVATE ${EVENTENGINE_EXAMPLE_LINK_TARGET})
