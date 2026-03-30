#include <eventengine/error.hpp>
#include <uv.h>

namespace eventengine {

// --- Core error strings ---

const char* errorCodeName(CoreError code) {
    switch (code) {
        case EE_SUCCESS:     return "EE_SUCCESS";
        case EE_UNKNOWN:     return "EE_UNKNOWN";
        case EE_INIT_FAILED: return "EE_INIT_FAILED";
    }
    return "EE_UNKNOWN";
}

const char* errorCodeMessage(CoreError code) {
    switch (code) {
        case EE_SUCCESS:     return "success";
        case EE_UNKNOWN:     return "unknown error";
        case EE_INIT_FAILED: return "initialization failed";
    }
    return "unknown error";
}

namespace timer {

const char* errorCodeName(TimerError code) {
    switch (code) {
        case EE_TIMER_SUCCESS:        return "EE_TIMER_SUCCESS";
        case EE_TIMER_INVALID_ID:     return "EE_TIMER_INVALID_ID";
        case EE_TIMER_ALREADY_EXISTS: return "EE_TIMER_ALREADY_EXISTS";
        case EE_TIMER_NOT_FOUND:      return "EE_TIMER_NOT_FOUND";
        case EE_TIMER_ALREADY_PAUSED: return "EE_TIMER_ALREADY_PAUSED";
        case EE_TIMER_NOT_PAUSED:     return "EE_TIMER_NOT_PAUSED";
        case EE_TIMER_LIMIT_REACHED:  return "EE_TIMER_LIMIT_REACHED";
    }
    return "EE_UNKNOWN";
}

const char* errorCodeMessage(TimerError code) {
    switch (code) {
        case EE_TIMER_SUCCESS:        return "success";
        case EE_TIMER_INVALID_ID:     return "empty or invalid timer ID";
        case EE_TIMER_ALREADY_EXISTS: return "timer with this ID already exists";
        case EE_TIMER_NOT_FOUND:      return "timer ID not found";
        case EE_TIMER_ALREADY_PAUSED: return "timer is already paused";
        case EE_TIMER_NOT_PAUSED:     return "timer is not paused";
        case EE_TIMER_LIMIT_REACHED:  return "maximum timer count exceeded";
    }
    return "unknown error";
}

} // namespace timer

namespace threadpool {

const char* errorCodeName(ThreadPoolError code) {
    switch (code) {
        case EE_POOL_SUCCESS:        return "EE_POOL_SUCCESS";
        case EE_POOL_INVALID_NAME:   return "EE_POOL_INVALID_NAME";
        case EE_POOL_ALREADY_EXISTS: return "EE_POOL_ALREADY_EXISTS";
        case EE_POOL_NOT_FOUND:      return "EE_POOL_NOT_FOUND";
        case EE_POOL_NOT_CANCELLABLE: return "EE_POOL_NOT_CANCELLABLE";
        case EE_POOL_CANCELLED:      return "EE_POOL_CANCELLED";
        case EE_POOL_QUEUE_FAILED:   return "EE_POOL_QUEUE_FAILED";
    }
    return "EE_UNKNOWN";
}

const char* errorCodeMessage(ThreadPoolError code) {
    switch (code) {
        case EE_POOL_SUCCESS:        return "success";
        case EE_POOL_INVALID_NAME:   return "empty or invalid task name";
        case EE_POOL_ALREADY_EXISTS: return "task with this name already exists";
        case EE_POOL_NOT_FOUND:      return "task name not found";
        case EE_POOL_NOT_CANCELLABLE: return "task already running or completed";
        case EE_POOL_CANCELLED:      return "task was cancelled";
        case EE_POOL_QUEUE_FAILED:   return "failed to queue work";
    }
    return "unknown error";
}

} // namespace threadpool

// --- Error methods ---

static const char* lookupName(int code) {
    if (code >= -99 && code <= 0)
        return errorCodeName(static_cast<CoreError>(code));
    if (code >= -199 && code <= -100)
        return timer::errorCodeName(static_cast<timer::TimerError>(code));
    if (code >= -299 && code <= -200)
        return threadpool::errorCodeName(static_cast<threadpool::ThreadPoolError>(code));
    return "EE_UNKNOWN";
}

static const char* lookupMessage(int code) {
    if (code >= -99 && code <= 0)
        return errorCodeMessage(static_cast<CoreError>(code));
    if (code >= -199 && code <= -100)
        return timer::errorCodeMessage(static_cast<timer::TimerError>(code));
    if (code >= -299 && code <= -200)
        return threadpool::errorCodeMessage(static_cast<threadpool::ThreadPoolError>(code));
    return "unknown error";
}

std::string Error::message() const {
    return lookupMessage(code_);
}

std::string Error::name() const {
    return lookupName(code_);
}

void Error::throwIfError() const {
    if (code_ < 0) {
        throw Exception(*this);
    }
}

Error Error::fromUV(int uv_errno) {
    if (uv_errno >= 0) return Error(EE_SUCCESS);
    return Error(EE_UNKNOWN);
}

// --- Exception ---

Exception::Exception(Error err)
    : std::runtime_error(err.message()), error_(err) {}

} // namespace eventengine
