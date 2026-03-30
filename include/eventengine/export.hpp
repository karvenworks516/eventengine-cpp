#ifndef EVENTENGINE_EXPORT_HPP
#define EVENTENGINE_EXPORT_HPP

/// @file export.hpp
/// @brief Symbol visibility macros for shared/static library builds.

#if defined(_WIN32)
  #if defined(EVENTENGINE_BUILDING_SHARED)
    #define EVENTENGINE_API __declspec(dllexport)
  #elif defined(EVENTENGINE_USING_SHARED)
    #define EVENTENGINE_API __declspec(dllimport)
  #else
    #define EVENTENGINE_API
  #endif
#elif !defined(DOXYGEN_SHOULD_SKIP_THIS)
  #if defined(EVENTENGINE_BUILDING_SHARED)
    #define EVENTENGINE_API __attribute__((visibility("default")))
  #else
    #define EVENTENGINE_API
  #endif
#else
  #define EVENTENGINE_API
#endif

#endif // EVENTENGINE_EXPORT_HPP
