#ifndef EVENTENGINE_ERROR_HPP
#define EVENTENGINE_ERROR_HPP

/// @file error.hpp
/// @brief Error wrapper and exception type for eventengine operations.

#include <eventengine/export.hpp>
#include <eventengine/error_code.hpp>
#include <stdexcept>
#include <string>

namespace eventengine {

/// @brief Lightweight error wrapper for all eventengine operations.
///
/// Stores an integer error code. Zero means success, negative means error.
/// Error codes are defined per service in error_code.hpp.
class EVENTENGINE_API Error {
public:
    /// @brief Constructs a success (no-error) value.
    Error() noexcept : code_(0) {}

    /// @brief Constructs from an integer error code.
    explicit Error(int code) noexcept : code_(code) {}

    /// @brief Returns @c true if this represents an error.
    explicit operator bool() const noexcept { return code_ < 0; }

    /// @brief Returns the integer error code.
    int code() const noexcept { return code_; }

    /// @brief Returns a human-readable error message.
    std::string message() const;

    /// @brief Returns the error code name (e.g. "EE_TIMER_NOT_FOUND").
    std::string name() const;

    /// @brief Throws an Exception if this represents an error.
    void throwIfError() const;

    /// @brief Creates an Error from a raw libuv errno, mapping to the appropriate core ErrorCode.
    static Error fromUV(int uv_errno);

private:
    int code_;
};

/// @brief Exception type thrown by the @c OrThrow API variants.
///
/// Inherits from @c std::runtime_error and carries the originating Error.
class EVENTENGINE_API Exception : public std::runtime_error {
public:
    /// @brief Constructs from an Error.
    explicit Exception(Error err);

    /// @brief Returns the underlying Error.
    const Error& error() const noexcept { return error_; }

private:
    Error error_;
};

} // namespace eventengine

#endif // EVENTENGINE_ERROR_HPP
