#ifndef EVENTENGINE_HANDLE_HPP
#define EVENTENGINE_HANDLE_HPP

/// @file handle.hpp
/// @brief CRTP base class for all libuv handle wrappers.

#include <eventengine/export.hpp>
#include <eventengine/loop.hpp>
#include <uv.h>
#include <utility>

namespace eventengine {

/// @brief CRTP base providing RAII lifecycle management for libuv handles.
/// @tparam Derived  The concrete handle type (e.g. Timer).
/// @tparam UvHandle The underlying libuv handle type (e.g. uv_timer_t).
///
/// Handles are non-copyable and movable. The destructor automatically
/// calls uv_close() if the handle is still open.
template <typename Derived, typename UvHandle>
class Handle {
public:
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    /// @brief Returns @c true if the handle is active.
    bool isActive() const {
        return handle_ && uv_is_active(reinterpret_cast<const uv_handle_t*>(handle_)) != 0;
    }

    /// @brief Returns @c true if the handle is closing or already closed.
    bool isClosing() const {
        return !handle_ || uv_is_closing(reinterpret_cast<const uv_handle_t*>(handle_)) != 0;
    }

    /// @brief Closes the handle. Idempotent — safe to call multiple times.
    void close() {
        if (handle_ && !uv_is_closing(reinterpret_cast<uv_handle_t*>(handle_))) {
            handle_->data = nullptr;
            uv_close(reinterpret_cast<uv_handle_t*>(handle_), onClose);
            handle_ = nullptr;
        }
    }

    /// @brief Returns the underlying libuv handle pointer.
    UvHandle* raw() noexcept { return handle_; }
    /// @copydoc raw()
    const UvHandle* raw() const noexcept { return handle_; }

protected:
    /// @brief Initialises the handle on the given loop.
    explicit Handle(Loop& loop) : loop_(loop), handle_(new UvHandle{}) {
        handle_->data = static_cast<Derived*>(this);
    }

    /// @brief Destructor. Calls uv_close() if the handle is still open.
    ~Handle() {
        if (handle_) {
            if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(handle_))) {
                handle_->data = nullptr;
                uv_close(reinterpret_cast<uv_handle_t*>(handle_), onClose);
            }
            handle_ = nullptr;
        }
    }

    /// @brief Move constructor. Transfers handle ownership.
    Handle(Handle&& other) noexcept
        : loop_(other.loop_), handle_(other.handle_) {
        if (handle_) {
            handle_->data = static_cast<Derived*>(this);
        }
        other.handle_ = nullptr;
    }

    /// @brief Move assignment. Closes current handle and transfers ownership.
    Handle& operator=(Handle&& other) noexcept {
        if (this != &other) {
            close();
            loop_ = other.loop_;
            handle_ = other.handle_;
            if (handle_) {
                handle_->data = static_cast<Derived*>(this);
            }
            other.handle_ = nullptr;
        }
        return *this;
    }

    /// @brief Recovers the Derived pointer from a raw libuv handle.
    static Derived* self(UvHandle* h) {
        return static_cast<Derived*>(h->data);
    }

    Loop& loop_;       ///< Non-owning reference to the parent loop.
    UvHandle* handle_; ///< Owned libuv handle (heap-allocated).

private:
    static void onClose(uv_handle_t* handle) {
        delete reinterpret_cast<UvHandle*>(handle);
    }
};

} // namespace eventengine

#endif // EVENTENGINE_HANDLE_HPP
