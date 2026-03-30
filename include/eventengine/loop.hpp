#ifndef EVENTENGINE_LOOP_HPP
#define EVENTENGINE_LOOP_HPP

/// @file loop.hpp
/// @brief RAII wrapper for the libuv event loop.

#include <eventengine/export.hpp>
#include <eventengine/error.hpp>
#include <uv.h>

namespace eventengine {

/// @brief RAII wrapper around @c uv_loop_t.
///
/// Owns the underlying libuv loop and ensures proper cleanup on destruction.
/// Not thread-safe — all calls must be made from the loop thread.
class EVENTENGINE_API Loop {
public:
    /// @brief Event loop execution mode.
    enum class RunMode {
        Default = UV_RUN_DEFAULT, ///< Run until no active handles/requests remain.
        Once    = UV_RUN_ONCE,    ///< Process a single event, blocking if none pending.
        NoWait  = UV_RUN_NOWAIT   ///< Process pending events without blocking.
    };

    /// @brief Creates and initialises a new event loop.
    /// @throws Exception if uv_loop_init fails.
    Loop();
    ~Loop();

    Loop(const Loop&) = delete;
    Loop& operator=(const Loop&) = delete;
    /// @brief Move constructor.
    Loop(Loop&& other) noexcept;
    /// @brief Move assignment operator.
    Loop& operator=(Loop&& other) noexcept;

    /// @brief Runs the event loop.
    /// @param mode Execution mode (Default, Once, or NoWait).
    /// @return Error on failure, success otherwise.
    Error run(RunMode mode = RunMode::Default);

    /// @brief Stops the event loop.
    void stop();

    /// @brief Returns @c true if the loop has active handles or requests.
    bool alive() const;

    /// @brief Returns the underlying libuv loop pointer.
    uv_loop_t* raw() noexcept { return loop_; }
    /// @copydoc raw()
    const uv_loop_t* raw() const noexcept { return loop_; }

private:
    Loop(uv_loop_t* loop, bool owns);
    void closeAllHandles();

    uv_loop_t* loop_;
    bool owns_;
};

} // namespace eventengine

#endif // EVENTENGINE_LOOP_HPP
