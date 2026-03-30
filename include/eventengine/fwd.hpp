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

enum class Service;
class ServiceConfig;

// Timer (public)
namespace timer {
    class Duration;
    class RepeatPolicy;
    class TimerService;
    enum class TimerStatus;
    enum class TimerEvent;
    struct TimerInfo;
} // namespace timer

// Thread pool (public)
namespace threadpool {
    class ThreadPoolService;
    enum class TaskStatus;
    struct TaskInfo;
} // namespace threadpool

} // namespace eventengine

#endif // EVENTENGINE_FWD_HPP
