#ifndef EVENTENGINE_TIMER_HPP
#define EVENTENGINE_TIMER_HPP

/// @file timer.hpp
/// @brief Low-level one-shot and repeating timer handle.

#include <eventengine/export.hpp>
#include <eventengine/handle.hpp>
#include <functional>
#include <cstdint>

namespace eventengine {

/// @brief RAII wrapper around @c uv_timer_t.
///
/// Supports one-shot and repeating timers. For managed timer
/// lifecycle (pause, resume, reschedule), see TimerService.
class EVENTENGINE_API Timer : public Handle<Timer, uv_timer_t> {
public:
    /// @brief Timer callback signature.
    using Callback = std::function<void()>;

    /// @brief Creates a timer on the given loop.
    /// @param loop The event loop this timer belongs to.
    /// @throws Exception if initialisation fails.
    explicit Timer(Loop& loop);

    /// @brief Starts the timer.
    /// @param cb         Callback invoked on each fire.
    /// @param timeout_ms Initial delay in milliseconds.
    /// @param repeat_ms  Repeat interval (0 = one-shot).
    /// @return Error on failure.
    Error start(Callback cb, uint64_t timeout_ms, uint64_t repeat_ms = 0);

    /// @brief Starts the timer, throwing on failure.
    /// @param cb         Callback invoked on each fire.
    /// @param timeout_ms Initial delay in milliseconds.
    /// @param repeat_ms  Repeat interval (0 = one-shot).
    /// @throws Exception on failure.
    void startOrThrow(Callback cb, uint64_t timeout_ms, uint64_t repeat_ms = 0);

    /// @brief Stops the timer.
    Error stop();
    /// @brief Stops the timer, throwing on failure.
    void stopOrThrow();

    /// @brief Restarts a repeating timer (uses the current repeat interval).
    Error again();

    /// @brief Sets the repeat interval in milliseconds.
    void setRepeat(uint64_t repeat_ms);

    /// @brief Returns the current repeat interval in milliseconds.
    uint64_t getRepeat() const;

    /// @brief Returns milliseconds until the next fire, or 0 if not active.
    uint64_t getDueIn() const;

private:
    Callback callback_;
    static void onTimer(uv_timer_t* handle);
};

} // namespace eventengine

#endif // EVENTENGINE_TIMER_HPP
