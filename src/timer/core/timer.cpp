#include <eventengine/detail/timer.hpp>

namespace eventengine {
namespace timer {

Timer::Timer(Loop& loop) : Handle(loop) {
    int r = uv_timer_init(loop.raw(), handle_);
    if (r < 0) {
        delete handle_;
        handle_ = nullptr;
        Error::fromUV(r).throwIfError();
    }
}

Error Timer::start(Callback cb, uint64_t timeout_ms, uint64_t repeat_ms) {
    callback_ = std::move(cb);
    int r = uv_timer_start(handle_, onTimer, timeout_ms, repeat_ms);
    return Error::fromUV(r);
}

void Timer::startOrThrow(Callback cb, uint64_t timeout_ms, uint64_t repeat_ms) {
    start(std::move(cb), timeout_ms, repeat_ms).throwIfError();
}

Error Timer::stop() {
    int r = uv_timer_stop(handle_);
    return Error::fromUV(r);
}

void Timer::stopOrThrow() {
    stop().throwIfError();
}

Error Timer::again() {
    int r = uv_timer_again(handle_);
    return Error::fromUV(r);
}

void Timer::setRepeat(uint64_t repeat_ms) {
    uv_timer_set_repeat(handle_, repeat_ms);
}

uint64_t Timer::getRepeat() const {
    return uv_timer_get_repeat(handle_);
}

uint64_t Timer::getDueIn() const {
    return uv_timer_get_due_in(handle_);
}

void Timer::onTimer(uv_timer_t* handle) {
    auto* t = self(handle);
    if (t && t->callback_) t->callback_();
}

} // namespace timer
} // namespace eventengine
