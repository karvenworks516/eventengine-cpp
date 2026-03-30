#include <eventengine/loop.hpp>
#include <utility>

namespace eventengine {

Loop::Loop() : loop_(new uv_loop_t), owns_(true) {
    int r = uv_loop_init(loop_);
    if (r < 0) {
        delete loop_;
        loop_ = nullptr;
        Error(r).throwIfError();
    }
}

Loop::Loop(uv_loop_t* loop, bool owns) : loop_(loop), owns_(owns) {}

Loop::~Loop() {
    if (loop_ && owns_) {
        closeAllHandles();
        uv_run(loop_, UV_RUN_DEFAULT);
        uv_loop_close(loop_);
        delete loop_;
    }
}

Loop::Loop(Loop&& other) noexcept
    : loop_(other.loop_), owns_(other.owns_) {
    other.loop_ = nullptr;
    other.owns_ = false;
}

Loop& Loop::operator=(Loop&& other) noexcept {
    if (this != &other) {
        if (loop_ && owns_) {
            closeAllHandles();
            uv_run(loop_, UV_RUN_DEFAULT);
            uv_loop_close(loop_);
            delete loop_;
        }
        loop_ = other.loop_;
        owns_ = other.owns_;
        other.loop_ = nullptr;
        other.owns_ = false;
    }
    return *this;
}

Error Loop::run(RunMode mode) {
    int r = uv_run(loop_, static_cast<uv_run_mode>(mode));
    return Error(r < 0 ? r : 0);
}

void Loop::stop() {
    uv_stop(loop_);
}

bool Loop::alive() const {
    return uv_loop_alive(loop_) != 0;
}

void Loop::closeAllHandles() {
    uv_walk(loop_, [](uv_handle_t* handle, void*) {
        if (!uv_is_closing(handle)) {
            uv_close(handle, nullptr);
        }
    }, nullptr);
}

} // namespace eventengine
