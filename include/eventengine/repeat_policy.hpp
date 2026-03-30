#ifndef EVENTENGINE_REPEAT_POLICY_HPP
#define EVENTENGINE_REPEAT_POLICY_HPP

/// @file repeat_policy.hpp
/// @brief Repeat policy for managed timers (one-shot, finite, or infinite).

#include <eventengine/export.hpp>
#include <eventengine/duration.hpp>
#include <cstdint>
#include <limits>

namespace eventengine {
namespace timer {

/// @brief Describes how a managed timer repeats after its initial fire.
///
/// Construct via the named static factories: none(), forever(), or times().
class EVENTENGINE_API RepeatPolicy {
public:
    /// @brief Default-constructs a one-shot (no repeat) policy.
    RepeatPolicy() noexcept : interval_(), maxCount_(0) {}

    /// @brief One-shot — the timer fires once and expires.
    static RepeatPolicy none() {
        return RepeatPolicy(Duration(), 0);
    }

    /// @brief Infinite repeat at the given interval.
    static RepeatPolicy forever(Duration interval) {
        return RepeatPolicy(interval, std::numeric_limits<uint32_t>::max());
    }

    /// @brief Repeat a fixed number of times at the given interval.
    static RepeatPolicy times(Duration interval, uint32_t count) {
        return RepeatPolicy(interval, count);
    }

    /// @brief Returns @c true if this is a one-shot policy.
    bool isOneShot() const noexcept { return maxCount_ == 0; }
    /// @brief Returns @c true if this repeats indefinitely.
    bool isForever() const noexcept { return maxCount_ == std::numeric_limits<uint32_t>::max(); }
    /// @brief Returns the maximum fire count (0 = one-shot, UINT32_MAX = infinite).
    uint32_t maxCount() const noexcept { return maxCount_; }
    /// @brief Returns the repeat interval.
    Duration interval() const noexcept { return interval_; }

private:
    RepeatPolicy(Duration interval, uint32_t maxCount) noexcept
        : interval_(interval), maxCount_(maxCount) {}

    Duration interval_;
    uint32_t maxCount_;
};

} // namespace timer
} // namespace eventengine

#endif // EVENTENGINE_REPEAT_POLICY_HPP
