#include <eventengine/thread_pool_service.hpp>
#include <eventengine/detail/timer.hpp>

namespace eventengine {
namespace threadpool {

ThreadPoolService::ThreadPoolService(Loop& loop) : loop_(loop) {}

ThreadPoolService::~ThreadPoolService() {
    for (auto& [name, entry] : tasks_) {
        if (entry.timeoutTimer) entry.timeoutTimer->stop();
    }
}

Error ThreadPoolService::submit(const std::string& name,
                                 WorkCallback work_cb,
                                 AfterWorkCallback after_cb,
                                 timer::Duration timeout) {
    if (name.empty()) return Error(EE_POOL_INVALID_NAME);
    if (tasks_.count(name)) return Error(EE_POOL_ALREADY_EXISTS);

    auto& entry = tasks_[name];
    entry.name = name;
    entry.afterCb = std::move(after_cb);
    entry.timeout = timeout;
    entry.status = TaskStatus::Queued;

    activeCount_++;

    auto wrappedWork = [this, taskName = name, userWork = std::move(work_cb)]() {
        auto it = tasks_.find(taskName);
        if (it != tasks_.end()) it->second.status = TaskStatus::InProgress;
        if (userWork) userWork();
    };

    auto wrappedAfter = [this, taskName = name](Error err) {
        onTaskComplete(taskName, err);
    };

    Work::Handle handle = nullptr;
    auto err = Work::queue(loop_, std::move(wrappedWork), std::move(wrappedAfter), &handle);
    if (err) {
        activeCount_--;
        entry.status = TaskStatus::Rejected;
        tasks_.erase(name);
        return err;
    }

    entry.workHandle = handle;
    if (timeout.toMilliseconds() > 0) startTimeoutTimer(entry);

    return Error();
}

void ThreadPoolService::submitOrThrow(const std::string& name,
                                       WorkCallback work_cb,
                                       AfterWorkCallback after_cb,
                                       timer::Duration timeout) {
    submit(name, std::move(work_cb), std::move(after_cb), timeout).throwIfError();
}

Error ThreadPoolService::cancel(const std::string& name) {
    auto it = tasks_.find(name);
    if (it == tasks_.end()) return Error(EE_POOL_NOT_FOUND);

    auto& entry = it->second;
    if (entry.status != TaskStatus::Queued) return Error(EE_POOL_NOT_CANCELLABLE);

    return Work::cancel(entry.workHandle);
}

void ThreadPoolService::cancelOrThrow(const std::string& name) {
    cancel(name).throwIfError();
}

bool ThreadPoolService::hasTask(const std::string& name) const {
    return tasks_.count(name) > 0;
}

TaskStatus ThreadPoolService::status(const std::string& name) const {
    auto it = tasks_.find(name);
    if (it == tasks_.end()) return TaskStatus::Finished;
    return it->second.status;
}

TaskInfo ThreadPoolService::info(const std::string& name) const {
    auto it = tasks_.find(name);
    if (it == tasks_.end()) return {name, TaskStatus::Finished, timer::Duration()};
    return {it->second.name, it->second.status, it->second.timeout};
}

std::vector<std::string> ThreadPoolService::allTaskNames() const {
    std::vector<std::string> names;
    names.reserve(tasks_.size());
    for (const auto& [name, entry] : tasks_) names.push_back(name);
    return names;
}

size_t ThreadPoolService::activeCount() const {
    return activeCount_.load();
}

size_t ThreadPoolService::completedCount() const {
    return completedCount_.load();
}

void ThreadPoolService::onTaskComplete(const std::string& name, Error err) {
    activeCount_--;
    completedCount_++;

    auto it = tasks_.find(name);
    if (it == tasks_.end()) return;

    auto& entry = it->second;
    if (entry.timeoutTimer) {
        entry.timeoutTimer->stop();
        entry.timeoutTimer.reset();
    }

    if (err.code() == EE_POOL_CANCELLED)
        entry.status = TaskStatus::Cancelled;
    else
        entry.status = TaskStatus::Finished;

    AfterWorkCallback cb = std::move(entry.afterCb);
    tasks_.erase(it);

    if (cb) cb(name, err);
}

void ThreadPoolService::startTimeoutTimer(TaskEntry& entry) {
    entry.timeoutTimer = std::make_unique<timer::Timer>(loop_);
    std::string taskName = entry.name;
    entry.timeoutTimer->start([this, taskName]() {
        auto it = tasks_.find(taskName);
        if (it == tasks_.end()) return;
        Work::cancel(it->second.workHandle);
    }, entry.timeout.toMilliseconds());
}

} // namespace threadpool
} // namespace eventengine
