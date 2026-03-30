#ifndef EVENTENGINE_TIMER_SERVICE_HPP
#define EVENTENGINE_TIMER_SERVICE_HPP

/// @file timer_service.hpp
/// @brief High-level managed timer service with lifecycle control.

#include <eventengine/export.hpp>
#include <eventengine/error.hpp>
#include <eventengine/duration.hpp>
#include <eventengine/repeat_policy.hpp>
#include <eventengine/timer_types.hpp>
#include <eventengine/detail/timer.hpp>
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

namespace eventengine {
namespace timer {

/// @brief Managed timer service providing creation, cancellation,
///        pause/resume, rescheduling, and querying of named timers.
///
/// Each timer is identified by a unique string ID. The service enforces
/// a configurable maximum timer count and supports an optional event
/// listener for lifecycle notifications.
class EVENTENGINE_API TimerService {
public:
    /// @brief Callback invoked when a managed timer fires.
    using Callback = std::function<void(const TimerInfo&)>;
    /// @brief Listener invoked on timer lifecycle events (created, fired, cancelled, etc.).
    using EventListener = std::function<void(TimerEvent, const TimerInfo&)>;

    /// @brief Constructs the service on the given loop.
    /// @param loop      The event loop to create timers on.
    /// @param maxTimers  Maximum number of concurrent timers allowed.
    explicit TimerService(Loop& loop, size_t maxTimers = 1024);
    ~TimerService();

    TimerService(const TimerService&) = delete;
    TimerService& operator=(const TimerService&) = delete;

    /// @name Create
    /// @{

    /// @brief Creates and starts a new timer.
    /// @param id      Unique timer identifier.
    /// @param timeout Initial delay before the first fire.
    /// @param cb      Callback invoked on each fire.
    /// @param repeat  Repeat policy (default: one-shot).
    /// @param label   Optional human-readable label.
    /// @return Error if the ID is duplicate or the limit is reached.
    Error setTimer(const std::string& id,
                   Duration timeout,
                   Callback cb,
                   RepeatPolicy repeat = RepeatPolicy::none(),
                   const std::string& label = "");
    /// @brief Creates and starts a new timer, throwing on failure.
    /// @throws Exception if the ID is duplicate or the limit is reached.
    void setTimerOrThrow(const std::string& id,
                         Duration timeout,
                         Callback cb,
                         RepeatPolicy repeat = RepeatPolicy::none(),
                         const std::string& label = "");
    /// @}

    /// @name Cancel
    /// @{

    /// @brief Cancels the timer with the given ID.
    Error cancelTimer(const std::string& id);
    /// @brief Cancels the timer with the given ID, throwing on failure.
    void cancelTimerOrThrow(const std::string& id);
    /// @brief Cancels all active timers.
    void cancelAll();
    /// @}

    /// @name Pause / Resume
    /// @{

    /// @brief Pauses the timer, preserving remaining time.
    Error pauseTimer(const std::string& id);
    /// @brief Pauses the timer, throwing on failure.
    void pauseTimerOrThrow(const std::string& id);
    /// @brief Resumes a paused timer from where it left off.
    Error resumeTimer(const std::string& id);
    /// @brief Resumes a paused timer, throwing on failure.
    void resumeTimerOrThrow(const std::string& id);
    /// @brief Pauses all running timers.
    void pauseAll();
    /// @brief Resumes all paused timers.
    void resumeAll();
    /// @}

    /// @name Reschedule
    /// @{

    /// @brief Reschedules an existing timer with a new timeout and repeat policy.
    Error rescheduleTimer(const std::string& id,
                          Duration timeout,
                          RepeatPolicy repeat = RepeatPolicy::none());
    /// @brief Reschedules an existing timer, throwing on failure.
    void rescheduleTimerOrThrow(const std::string& id,
                                Duration timeout,
                                RepeatPolicy repeat = RepeatPolicy::none());
    /// @}

    /// @name Query
    /// @{

    /// @brief Returns @c true if a timer with the given ID exists.
    bool hasTimer(const std::string& id) const;
    /// @brief Returns the status of the timer.
    TimerStatus status(const std::string& id) const;
    /// @brief Returns a snapshot of the timer's information.
    TimerInfo info(const std::string& id) const;
    /// @brief Returns the remaining time before the next fire.
    Duration remainingTime(const std::string& id) const;
    /// @brief Returns the IDs of all managed timers.
    std::vector<std::string> allTimerIds() const;
    /// @brief Returns the number of currently active timers.
    size_t activeCount() const;
    /// @brief Returns the configured maximum timer count.
    size_t maxTimers() const;
    /// @}

    /// @name Event Listener
    /// @{

    /// @brief Registers a listener for timer lifecycle events.
    void setEventListener(EventListener listener);
    /// @brief Removes the current event listener.
    void clearEventListener();
    /// @}

private:
    struct TimerEntry {
        std::unique_ptr<Timer> timer;
        Callback callback;
        std::string id;
        std::string label;
        Duration timeout;
        RepeatPolicy repeatPolicy;
        TimerStatus status;
        uint32_t fireCount = 0;
        uint64_t remainingMs = 0;
    };

    Loop& loop_;
    size_t maxTimers_;
    std::unordered_map<std::string, TimerEntry> timers_;
    std::vector<std::string> pendingRemoval_;
    std::vector<std::unique_ptr<Timer>> graveyard_;
    EventListener eventListener_;

    void flushPending();
    void emitEvent(TimerEvent event, const TimerEntry& entry);
    void startTimerEntry(TimerEntry& entry);
    TimerInfo buildInfo(const TimerEntry& entry) const;
};

} // namespace timer
} // namespace eventengine

#endif // EVENTENGINE_TIMER_SERVICE_HPP
