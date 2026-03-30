cmake_minimum_required(VERSION 3.16)
project(eventengine-cpp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EVENTENGINE_BUILD_SHARED  "Build shared library" ON)
option(EVENTENGINE_BUILD_STATIC  "Build static library" ON)
option(EVENTENGINE_BUILD_TESTS   "Build unit tests"     ON)
option(EVENTENGINE_BUILD_EXAMPLES "Build examples"      ON)

# --- libuv (bundled) ---
set(LIBUV_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/libuv)

# Generate version.hpp from template
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/eventengine/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/eventengine/version.hpp
    @ONLY
)

add_subdirectory(src)

if(EVENTENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(EVENTENGINE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# --- Doxygen (optional) ---
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()
