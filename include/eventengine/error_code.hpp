#ifndef EVENTENGINE_ERROR_CODE_HPP
#define EVENTENGINE_ERROR_CODE_HPP

/// @file error_code.hpp
/// @brief All error codes for eventengine, grouped by service namespace.
///
/// Each service defines its own error codes in a non-overlapping range.
/// Core errors use -1 to -99, timer uses -100 to -199,
/// threadpool uses -200 to -299. Future services continue the pattern.

namespace eventengine {

/// @brief Core error codes (range: -1 to -99).
enum CoreError {
    EE_SUCCESS       =    0, ///< No error.
    EE_UNKNOWN       =   -1, ///< Unknown or unmapped internal error.
    EE_INIT_FAILED   =   -2, ///< Initialization failed (loop, handle).
};

/// @brief Returns the name string for a CoreError.
const char* errorCodeName(CoreError code);
/// @brief Returns the message string for a CoreError.
const char* errorCodeMessage(CoreError code);

namespace timer {

/// @brief Timer service error codes (range: -100 to -199).
enum TimerError {
    EE_TIMER_SUCCESS         =    0, ///< No error.
    EE_TIMER_INVALID_ID      = -100, ///< Empty or invalid timer ID.
    EE_TIMER_ALREADY_EXISTS  = -101, ///< Timer with this ID already exists.
    EE_TIMER_NOT_FOUND       = -102, ///< Timer ID not found.
    EE_TIMER_ALREADY_PAUSED  = -103, ///< Timer is already paused.
    EE_TIMER_NOT_PAUSED      = -104, ///< Timer is not paused (cannot resume).
    EE_TIMER_LIMIT_REACHED   = -105, ///< Maximum timer count exceeded.
};

/// @brief Returns the name string for a TimerError.
const char* errorCodeName(TimerError code);
/// @brief Returns the message string for a TimerError.
const char* errorCodeMessage(TimerError code);

} // namespace timer

namespace threadpool {

/// @brief Thread pool service error codes (range: -200 to -299).
enum ThreadPoolError {
    EE_POOL_SUCCESS          =    0, ///< No error.
    EE_POOL_INVALID_NAME     = -200, ///< Empty or invalid task name.
    EE_POOL_ALREADY_EXISTS   = -201, ///< Task with this name already exists.
    EE_POOL_NOT_FOUND        = -202, ///< Task name not found.
    EE_POOL_NOT_CANCELLABLE  = -203, ///< Task already running or completed (cannot cancel).
    EE_POOL_CANCELLED        = -204, ///< Task was cancelled.
    EE_POOL_QUEUE_FAILED     = -205, ///< Failed to queue work (loop closing or internal error).
};

/// @brief Returns the name string for a ThreadPoolError.
const char* errorCodeName(ThreadPoolError code);
/// @brief Returns the message string for a ThreadPoolError.
const char* errorCodeMessage(ThreadPoolError code);

} // namespace threadpool

} // namespace eventengine

#endif // EVENTENGINE_ERROR_CODE_HPP
