#ifndef EVENTENGINE_FWD_HPP
#define EVENTENGINE_FWD_HPP

/// @file fwd.hpp
/// @brief Forward declarations for all public eventengine types.

#include <cstdint>

namespace eventengine {

// Core
class Error;
class Exception;
class Loop;

template <typename Derived, typename UvHandle>
class Handle;

// Timer
class Duration;
class RepeatPolicy;
class Timer;
class TimerService;

enum class TimerStatus;
enum class TimerEvent;
struct TimerInfo;

enum class Service;
class ServiceConfig;

} // namespace eventengine

#endif // EVENTENGINE_FWD_HPP
