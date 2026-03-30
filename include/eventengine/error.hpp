#ifndef EVENTENGINE_ERROR_HPP
#define EVENTENGINE_ERROR_HPP

/// @file error.hpp
/// @brief Error code wrapper and exception type for libuv error propagation.

#include <eventengine/export.hpp>
#include <stdexcept>
#include <string>

namespace eventengine {

/// @brief Lightweight wrapper around a libuv error code.
///
/// Converts to @c true when the code represents an error (negative value).
/// Used as the return type for the error-code variant of every fallible API.
class EVENTENGINE_API Error {
public:
    /// @brief Constructs a success (no-error) value.
    Error() noexcept : code_(0) {}

    /// @brief Constructs from a raw libuv errno (negative = error).
    explicit Error(int uv_errno) noexcept : code_(uv_errno) {}

    /// @brief Returns @c true if this represents an error.
    explicit operator bool() const noexcept { return code_ < 0; }

    /// @brief Returns the raw libuv error code.
    int code() const noexcept { return code_; }

    /// @brief Returns a human-readable error message.
    std::string message() const;

    /// @brief Returns the libuv error name (e.g. "EINVAL").
    std::string name() const;

    /// @brief Throws an Exception if this represents an error.
    void throwIfError() const;

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
