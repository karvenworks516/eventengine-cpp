#ifndef EVENTENGINE_THREAD_POOL_TYPES_HPP
#define EVENTENGINE_THREAD_POOL_TYPES_HPP

/// @file thread_pool_types.hpp
/// @brief Enumerations and data structures used by ThreadPoolService.

#include <eventengine/duration.hpp>
#include <string>
#include <cstdint>

namespace eventengine {
namespace threadpool {

/// @brief Lifecycle status of a managed thread pool task.
///
/// State transitions:
/// @code
///   submit() ──▶ Queued ──▶ InProgress ──▶ Finished
///                  │            │
///                  ▼            ▼
///              Cancelled    Cancelled (timeout)
///
///   submit() ──▶ Rejected  (duplicate name, empty name, queue failure)
/// @endcode
enum class TaskStatus {
    Queued,     ///< Task is waiting in the queue, not yet picked up by a pool thread.
    InProgress, ///< Task is executing on a pool thread.
    Finished,   ///< Task completed successfully.
    Cancelled,  ///< Task was cancelled (by user or timeout) before or during execution.
    Rejected    ///< Task was never queued (duplicate name, empty name, or queue failure).
};

/// @brief Read-only snapshot of a managed task's state.
struct TaskInfo {
    std::string name;       ///< User-provided task name.
    TaskStatus status;      ///< Current lifecycle status.
    timer::Duration timeout;       ///< Configured timeout (zero = no timeout).
};

} // namespace threadpool
} // namespace eventengine

#endif // EVENTENGINE_THREAD_POOL_TYPES_HPP
