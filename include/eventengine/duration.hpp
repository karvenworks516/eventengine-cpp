#ifndef EVENTENGINE_DURATION_HPP
#define EVENTENGINE_DURATION_HPP

/// @file duration.hpp
/// @brief Time duration type with named constructors for common units.

#include <eventengine/export.hpp>
#include <cstdint>

namespace eventengine {

/// @brief Represents a time duration stored internally as milliseconds.
///
/// Construct via named static factories (milliseconds, seconds, etc.).
/// Supports arithmetic and comparison operators.
class EVENTENGINE_API Duration {
public:
    /// @brief Default-constructs a zero duration.
    Duration() noexcept : ms_(0) {}

    /// @name Named Constructors
    /// @{

    /// @brief Creates a Duration from milliseconds.
    static Duration milliseconds(uint64_t ms) { return Duration(ms); }
    /// @brief Creates a Duration from seconds.
    static Duration seconds(uint64_t s) { return Duration(s * 1000); }
    /// @brief Creates a Duration from minutes.
    static Duration minutes(uint64_t m) { return Duration(m * 60 * 1000); }
    /// @brief Creates a Duration from hours.
    static Duration hours(uint64_t h) { return Duration(h * 60 * 60 * 1000); }
    /// @brief Creates a Duration from days.
    static Duration days(uint64_t d) { return Duration(d * 24 * 60 * 60 * 1000); }
    /// @}

    /// @brief Returns the duration in milliseconds.
    uint64_t toMilliseconds() const noexcept { return ms_; }

    /// @name Arithmetic Operators
    /// @{

    /// @brief Returns the sum of two durations.
    Duration operator+(const Duration& other) const { return Duration(ms_ + other.ms_); }
    /// @brief Returns the difference (clamped to zero).
    Duration operator-(const Duration& other) const { return Duration(ms_ > other.ms_ ? ms_ - other.ms_ : 0); }
    /// @}

    /// @name Comparison Operators
    /// @{

    /// @brief Equality comparison.
    bool operator==(const Duration& other) const noexcept { return ms_ == other.ms_; }
    /// @brief Inequality comparison.
    bool operator!=(const Duration& other) const noexcept { return ms_ != other.ms_; }
    /// @brief Less-than comparison.
    bool operator<(const Duration& other) const noexcept { return ms_ < other.ms_; }
    /// @brief Greater-than comparison.
    bool operator>(const Duration& other) const noexcept { return ms_ > other.ms_; }
    /// @brief Less-than-or-equal comparison.
    bool operator<=(const Duration& other) const noexcept { return ms_ <= other.ms_; }
    /// @brief Greater-than-or-equal comparison.
    bool operator>=(const Duration& other) const noexcept { return ms_ >= other.ms_; }
    /// @}

private:
    explicit Duration(uint64_t ms) noexcept : ms_(ms) {}
    uint64_t ms_;
};

} // namespace eventengine

#endif // EVENTENGINE_DURATION_HPP
