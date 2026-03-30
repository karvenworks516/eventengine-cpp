#ifndef EVENTENGINE_THREAD_POOL_SERVICE_HPP
#define EVENTENGINE_THREAD_POOL_SERVICE_HPP

/// @file thread_pool_service.hpp
/// @brief High-level thread pool service with named tasks, timeout, and cancellation.

#include <eventengine/export.hpp>
#include <eventengine/error.hpp>
#include <eventengine/duration.hpp>
#include <eventengine/detail/work.hpp>
#include <eventengine/thread_pool_types.hpp>
#include <functional>
#include <atomic>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>

namespace eventengine {

namespace timer { class Timer; }

namespace threadpool {

/// @brief Managed thread pool service with named tasks, cancellation,
///        timeout, and stats tracking.
///
/// Each task is identified by a unique name. Tasks can be cancelled
/// while queued (before a pool thread picks them up). An optional
/// timeout automatically cancels tasks that exceed the allowed duration.
class EVENTENGINE_API ThreadPoolService {
public:
    /// @brief Callback executed on a thread pool thread.
    using WorkCallback = std::function<void()>;
    /// @brief Callback executed on the loop thread after work completes.
    using AfterWorkCallback = std::function<void(const std::string& name, Error)>;

    /// @brief Constructs the service on the given loop.
    /// @param loop The event loop that receives after-work callbacks.
    explicit ThreadPoolService(Loop& loop);
    ~ThreadPoolService();

    ThreadPoolService(const ThreadPoolService&) = delete;
    ThreadPoolService& operator=(const ThreadPoolService&) = delete;

    /// @name Submit
    /// @{

    /// @brief Submits a named task to the thread pool.
    /// @param name     Unique task name.
    /// @param work_cb  Function executed on a thread pool thread.
    /// @param after_cb Function executed on the loop thread after completion.
    /// @param timeout  Maximum time before auto-cancel (zero = no timeout).
    /// @return Error if the name is duplicate or empty.
    Error submit(const std::string& name,
                 WorkCallback work_cb,
                 AfterWorkCallback after_cb,
                 timer::Duration timeout = timer::Duration());

    /// @brief Submits a named task, throwing on failure.
    /// @param name     Unique task name.
    /// @param work_cb  Function executed on a thread pool thread.
    /// @param after_cb Function executed on the loop thread after completion.
    /// @param timeout  Maximum time before auto-cancel (zero = no timeout).
    /// @throws Exception on failure.
    void submitOrThrow(const std::string& name,
                       WorkCallback work_cb,
                       AfterWorkCallback after_cb,
                       timer::Duration timeout = timer::Duration());

    /// @}

    /// @name Cancel
    /// @{

    /// @brief Cancels a queued task by name.
    /// @return Error if the task is not found or already running/completed.
    ///
    /// Only tasks that have not yet started executing can be cancelled.
    /// The after-work callback will receive EE_POOL_CANCELLED.
    Error cancel(const std::string& name);

    /// @brief Cancels a queued task by name, throwing on failure.
    /// @throws Exception on failure.
    void cancelOrThrow(const std::string& name);

    /// @}

    /// @name Query
    /// @{

    /// @brief Returns @c true if a task with the given name exists.
    bool hasTask(const std::string& name) const;

    /// @brief Returns the status of a task.
    TaskStatus status(const std::string& name) const;

    /// @brief Returns a snapshot of a task's information.
    TaskInfo info(const std::string& name) const;

    /// @brief Returns the names of all tracked tasks.
    std::vector<std::string> allTaskNames() const;

    /// @}

    /// @name Stats
    /// @{

    /// @brief Returns the number of currently in-flight tasks (thread-safe).
    size_t activeCount() const;

    /// @brief Returns the cumulative number of completed tasks (thread-safe).
    size_t completedCount() const;

    /// @}

private:
    struct TaskEntry {
        std::string name;
        AfterWorkCallback afterCb;
        timer::Duration timeout;
        TaskStatus status;
        Work::Handle workHandle = nullptr;
        std::unique_ptr<timer::Timer> timeoutTimer;
    };

    Loop& loop_;
    std::unordered_map<std::string, TaskEntry> tasks_;
    std::atomic<size_t> activeCount_{0};
    std::atomic<size_t> completedCount_{0};

    void onTaskComplete(const std::string& name, Error err);
    void startTimeoutTimer(TaskEntry& entry);
};

} // namespace threadpool
} // namespace eventengine

#endif // EVENTENGINE_THREAD_POOL_SERVICE_HPP
