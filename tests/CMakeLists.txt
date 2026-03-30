include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

set(EVENTENGINE_TEST_SOURCES
    test_timer_service.cpp
    test_thread_pool_service.cpp
)

# Pick the link target — prefer static
if(EVENTENGINE_BUILD_STATIC)
    set(EVENTENGINE_TEST_LINK_TARGET eventengine-cpp_static)
else()
    set(EVENTENGINE_TEST_LINK_TARGET eventengine-cpp_shared)
endif()

foreach(TEST_SOURCE ${EVENTENGINE_TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${EVENTENGINE_TEST_LINK_TARGET} GTest::gtest_main)
    gtest_discover_tests(${TEST_NAME})
endforeach()
