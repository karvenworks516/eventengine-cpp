set(EVENTENGINE_SOURCES
    # Core
    core/error.cpp
    core/loop.cpp

    # Timer
    timer/core/timer.cpp

    # Thread pool
    threadpool/core/thread_pool_work.cpp
    threadpool/service/thread_pool_service.cpp

    # Timer service
    timer/service/timer_service.cpp
)

set(EVENTENGINE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
set(EVENTENGINE_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)

# Helper function to configure a library target
function(configure_eventengine_cpp_target TARGET_NAME)
    target_include_directories(${TARGET_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${EVENTENGINE_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${EVENTENGINE_GENERATED_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(${TARGET_NAME} PUBLIC uv_a)
    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME eventengine-cpp
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endfunction()

# Static library
if(EVENTENGINE_BUILD_STATIC)
    add_library(eventengine-cpp_static STATIC ${EVENTENGINE_SOURCES})
    configure_eventengine_cpp_target(eventengine-cpp_static)
    add_library(eventengine-cpp::static ALIAS eventengine-cpp_static)
endif()

# Shared library
if(EVENTENGINE_BUILD_SHARED)
    add_library(eventengine-cpp_shared SHARED ${EVENTENGINE_SOURCES})
    configure_eventengine_cpp_target(eventengine-cpp_shared)
    target_compile_definitions(eventengine-cpp_shared
        PRIVATE EVENTENGINE_BUILDING_SHARED
        INTERFACE EVENTENGINE_USING_SHARED
    )
    set_target_properties(eventengine-cpp_shared PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
    add_library(eventengine-cpp::shared ALIAS eventengine-cpp_shared)
endif()
