#ifndef EVENTENGINE_SERVICE_HPP
#define EVENTENGINE_SERVICE_HPP

/// @file service.hpp
/// @brief Enumeration of available engine services.

namespace eventengine {

/// @brief Identifies a service that can be enabled on the Engine.
enum class Service {
    Timer,      ///< Managed timer service.
    ThreadPool, ///< Thread pool work offloading.
    Signal,     ///< Signal handling (planned).
    Idle,       ///< Idle handle service (planned).
    Prepare,    ///< Prepare handle service (planned).
    Check       ///< Check handle service (planned).
};

} // namespace eventengine

#endif // EVENTENGINE_SERVICE_HPP
