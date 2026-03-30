#include <eventengine/detail/work.hpp>

namespace eventengine {
namespace threadpool {

Error Work::queue(Loop& loop, WorkCb work_cb, AfterWorkCb after_cb,
                  Handle* handle) {
    auto* ctx = new Context{};
    ctx->workCb = std::move(work_cb);
    ctx->afterCb = std::move(after_cb);
    ctx->req.data = ctx;

    int r = uv_queue_work(loop.raw(), &ctx->req, onWork, onAfterWork);
    if (r < 0) {
        delete ctx;
        return Error(EE_POOL_QUEUE_FAILED);
    }
    if (handle) *handle = &ctx->req;
    return Error();
}

void Work::queueOrThrow(Loop& loop, WorkCb work_cb, AfterWorkCb after_cb,
                         Handle* handle) {
    queue(loop, std::move(work_cb), std::move(after_cb), handle).throwIfError();
}

Error Work::cancel(Handle handle) {
    int r = uv_cancel(reinterpret_cast<uv_req_t*>(handle));
    if (r < 0) return Error(EE_POOL_NOT_CANCELLABLE);
    return Error();
}

void Work::cancelOrThrow(Handle handle) {
    cancel(handle).throwIfError();
}

void Work::onWork(uv_work_t* req) {
    auto* ctx = static_cast<Context*>(req->data);
    if (ctx->workCb) ctx->workCb();
}

void Work::onAfterWork(uv_work_t* req, int status) {
    auto* ctx = static_cast<Context*>(req->data);
    if (ctx->afterCb) {
        Error err = (status == UV_ECANCELED) ? Error(EE_POOL_CANCELLED) : Error();
        ctx->afterCb(err);
    }
    delete ctx;
}

} // namespace threadpool
} // namespace eventengine
