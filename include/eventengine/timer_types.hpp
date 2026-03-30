#ifndef EVENTENGINE_TIMER_TYPES_HPP
#define EVENTENGINE_TIMER_TYPES_HPP

/// @file timer_types.hpp
/// @brief Enumerations and data structures used by TimerService.

#include <eventengine/duration.hpp>
#include <eventengine/repeat_policy.hpp>
#include <string>
#include <cstdint>

namespace eventengine {
namespace timer {

/// @brief Current status of a managed timer.
enum class TimerStatus {
    Running,   ///< Timer is active and counting.
    Paused,    ///< Timer is paused; remaining time is preserved.
    Expired,   ///< Timer completed all scheduled fires.
    Cancelled  ///< Timer was explicitly cancelled.
};

/// @brief Lifecycle events emitted by TimerService.
enum class TimerEvent {
    Created,     ///< Timer was created.
    Fired,       ///< Timer callback was invoked.
    Paused,      ///< Timer was paused.
    Resumed,     ///< Timer was resumed.
    Cancelled,   ///< Timer was cancelled.
    Expired,     ///< Timer completed its repeat policy.
    Rescheduled  ///< Timer was rescheduled with new parameters.
};

/// @brief Read-only snapshot of a managed timer's state.
struct TimerInfo {
    std::string id;              ///< Unique timer identifier.
    std::string label;           ///< Optional human-readable label.
    TimerStatus status;          ///< Current status.
    Duration timeout;            ///< Initial timeout duration.
    RepeatPolicy repeatPolicy;   ///< Repeat policy.
    uint32_t fireCount;          ///< Number of times the timer has fired.
    Duration remainingTime;      ///< Time remaining until the next fire.
};

} // namespace timer
} // namespace eventengine

#endif // EVENTENGINE_TIMER_TYPES_HPP
