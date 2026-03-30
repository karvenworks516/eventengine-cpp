#ifndef EVENTENGINE_WORK_HPP
#define EVENTENGINE_WORK_HPP

/// @file work.hpp
/// @brief Thread pool work request — offload blocking tasks to libuv's thread pool.

#include <eventengine/export.hpp>
#include <eventengine/error.hpp>
#include <eventengine/loop.hpp>
#include <functional>

namespace eventengine {
namespace threadpool {

/// @brief Fire-and-forget thread pool work request.
///
/// Wraps @c uv_queue_work. The work callback runs on a thread pool thread;
/// the after-work callback runs on the loop thread. The request is
/// heap-allocated internally and self-destructs after completion.
///
/// This is a request type, not a handle — there is no persistent object
/// to manage. Use the static queue() / queueOrThrow() methods.
class EVENTENGINE_API Work {
public:
    /// @brief Callback executed on a thread pool thread.
    using WorkCb = std::function<void()>;
    /// @brief Callback executed on the loop thread after work completes.
    using AfterWorkCb = std::function<void(Error)>;

    /// @brief Opaque handle returned by queue(), used to cancel pending work.
    using Handle = uv_work_t*;

    Work() = delete;
    Work(const Work&) = delete;
    Work& operator=(const Work&) = delete;

    /// @brief Queues work to run on the thread pool.
    /// @param loop     The event loop that receives the after-work callback.
    /// @param work_cb  Function executed on a thread pool thread.
    /// @param after_cb Function executed on the loop thread after work_cb finishes.
    /// @param[out] handle Optional pointer to receive the work handle for cancellation.
    /// @return Error on failure.
    static Error queue(Loop& loop, WorkCb work_cb, AfterWorkCb after_cb,
                       Handle* handle = nullptr);

    /// @brief Queues work to run on the thread pool, throwing on failure.
    /// @param loop     The event loop that receives the after-work callback.
    /// @param work_cb  Function executed on a thread pool thread.
    /// @param after_cb Function executed on the loop thread after work_cb finishes.
    /// @param[out] handle Optional pointer to receive the work handle for cancellation.
    /// @throws Exception on failure.
    static void queueOrThrow(Loop& loop, WorkCb work_cb, AfterWorkCb after_cb,
                              Handle* handle = nullptr);

    /// @brief Cancels a pending work request that has not yet started executing.
    /// @param handle The work handle returned by queue().
    /// @return Error on failure (e.g. work already started or completed).
    ///
    /// If successful, the after-work callback will be invoked with EE_POOL_CANCELLED.
    /// Has no effect on work that is already running on a pool thread.
    static Error cancel(Handle handle);

    /// @brief Cancels a pending work request, throwing on failure.
    /// @param handle The work handle returned by queue().
    /// @throws Exception on failure.
    static void cancelOrThrow(Handle handle);

private:
    struct Context {
        uv_work_t req{};
        WorkCb workCb;
        AfterWorkCb afterCb;
    };

    static void onWork(uv_work_t* req);
    static void onAfterWork(uv_work_t* req, int status);
};

} // namespace threadpool
} // namespace eventengine

#endif // EVENTENGINE_WORK_HPP
