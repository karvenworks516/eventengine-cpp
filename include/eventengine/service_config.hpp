#ifndef EVENTENGINE_SERVICE_CONFIG_HPP
#define EVENTENGINE_SERVICE_CONFIG_HPP

/// @file service_config.hpp
/// @brief Declarative configuration for enabling engine services.

#include <eventengine/export.hpp>
#include <eventengine/service.hpp>
#include <unordered_set>
#include <cstddef>

namespace eventengine {

/// @brief Declarative configuration specifying which services to enable
///        and their parameters.
class EVENTENGINE_API ServiceConfig {
public:
    /// @brief Enables a service.
    ServiceConfig& enable(Service svc);
    /// @brief Disables a service.
    ServiceConfig& disable(Service svc);
    /// @brief Returns @c true if the service is enabled.
    bool isEnabled(Service svc) const;

    /// @brief Sets the thread pool size (default: 4).
    ServiceConfig& setThreadPoolSize(size_t size);
    /// @brief Returns the configured thread pool size.
    size_t getThreadPoolSize() const;

    /// @brief Sets the maximum number of managed timers (default: 1024).
    ServiceConfig& setMaxTimers(size_t max);
    /// @brief Returns the configured maximum timer count.
    size_t getMaxTimers() const;

private:
    struct ServiceHash {
        size_t operator()(Service s) const noexcept {
            return static_cast<size_t>(s);
        }
    };
    std::unordered_set<Service, ServiceHash> enabled_;
    size_t threadPoolSize_ = 4;
    size_t maxTimers_ = 1024;
};

} // namespace eventengine

#endif // EVENTENGINE_SERVICE_CONFIG_HPP
