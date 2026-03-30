#include <eventengine/timer_service.hpp>
#include <eventengine/loop.hpp>
#include <algorithm>

namespace eventengine {
namespace timer {

TimerService::TimerService(Loop& loop, size_t maxTimers)
    : loop_(loop), maxTimers_(maxTimers) {}

TimerService::~TimerService() {
    cancelAll();
}

// --- Helpers ---

TimerInfo TimerService::buildInfo(const TimerEntry& entry) const {
    TimerInfo ti;
    ti.id = entry.id;
    ti.label = entry.label;
    ti.status = entry.status;
    ti.timeout = entry.timeout;
    ti.repeatPolicy = entry.repeatPolicy;
    ti.fireCount = entry.fireCount;
    if (entry.status == TimerStatus::Running && entry.timer) {
        ti.remainingTime = Duration::milliseconds(entry.timer->getDueIn());
    } else if (entry.status == TimerStatus::Paused) {
        ti.remainingTime = Duration::milliseconds(entry.remainingMs);
    } else {
        ti.remainingTime = Duration();
    }
    return ti;
}

void TimerService::emitEvent(TimerEvent event, const TimerEntry& entry) {
    if (eventListener_) {
        eventListener_(event, buildInfo(entry));
    }
}

void TimerService::startTimerEntry(TimerEntry& entry) {
    auto id = entry.id;
    uint64_t repeatMs = entry.repeatPolicy.isOneShot()
        ? 0 : entry.repeatPolicy.interval().toMilliseconds();

    auto wrappedCb = [this, id]() {
        auto it = timers_.find(id);
        if (it == timers_.end()) return;

        auto& e = it->second;
        e.fireCount++;

        bool shouldStop = false;
        if (e.repeatPolicy.isOneShot()) {
            shouldStop = true;
        } else if (!e.repeatPolicy.isForever() &&
                   e.fireCount >= e.repeatPolicy.maxCount()) {
            shouldStop = true;
        }

        if (shouldStop) {
            e.status = TimerStatus::Expired;
            e.timer->stop();
        }

        emitEvent(TimerEvent::Fired, e);
        if (shouldStop) {
            emitEvent(TimerEvent::Expired, e);
        }

        TimerInfo ti = buildInfo(e);
        e.callback(ti);

        if (shouldStop) {
            pendingRemoval_.push_back(id);
        }
    };

    entry.timer->startOrThrow(std::move(wrappedCb),
                               entry.timeout.toMilliseconds(),
                               repeatMs);
    entry.status = TimerStatus::Running;
}

void TimerService::flushPending() {
    graveyard_.clear();
    for (const auto& id : pendingRemoval_) {
        auto it = timers_.find(id);
        if (it != timers_.end()) {
            graveyard_.push_back(std::move(it->second.timer));
            timers_.erase(it);
        }
    }
    pendingRemoval_.clear();
}

// --- Create ---

Error TimerService::setTimer(const std::string& id,
                              Duration timeout,
                              Callback cb,
                              RepeatPolicy repeat,
                              const std::string& label) {
    if (id.empty()) return Error(EE_TIMER_INVALID_ID);
    if (timers_.count(id) > 0) return Error(EE_TIMER_ALREADY_EXISTS);
    if (timers_.size() >= maxTimers_) return Error(EE_TIMER_LIMIT_REACHED);

    flushPending();

    TimerEntry entry;
    entry.timer = std::make_unique<Timer>(loop_);
    entry.callback = std::move(cb);
    entry.id = id;
    entry.label = label;
    entry.timeout = timeout;
    entry.repeatPolicy = repeat;
    entry.status = TimerStatus::Running;
    entry.fireCount = 0;
    entry.remainingMs = 0;

    startTimerEntry(entry);
    auto [it, _] = timers_.emplace(id, std::move(entry));
    emitEvent(TimerEvent::Created, it->second);
    return Error();
}

void TimerService::setTimerOrThrow(const std::string& id,
                                    Duration timeout,
                                    Callback cb,
                                    RepeatPolicy repeat,
                                    const std::string& label) {
    setTimer(id, timeout, std::move(cb), repeat, label).throwIfError();
}

// --- Cancel ---

Error TimerService::cancelTimer(const std::string& id) {
    auto it = timers_.find(id);
    if (it == timers_.end()) return Error(EE_TIMER_NOT_FOUND);

    it->second.status = TimerStatus::Cancelled;
    if (it->second.timer) {
        it->second.timer->stop();
    }
    emitEvent(TimerEvent::Cancelled, it->second);
    pendingRemoval_.push_back(id);
    return Error();
}

void TimerService::cancelTimerOrThrow(const std::string& id) {
    cancelTimer(id).throwIfError();
}

void TimerService::cancelAll() {
    pendingRemoval_.clear();
    graveyard_.clear();
    for (auto& [id, entry] : timers_) {
        entry.status = TimerStatus::Cancelled;
        if (entry.timer) {
            entry.timer->stop();
        }
        emitEvent(TimerEvent::Cancelled, entry);
    }
    timers_.clear();
}

// --- Pause / Resume ---

Error TimerService::pauseTimer(const std::string& id) {
    auto it = timers_.find(id);
    if (it == timers_.end()) return Error(EE_TIMER_NOT_FOUND);
    if (it->second.status != TimerStatus::Running) return Error(EE_TIMER_ALREADY_PAUSED);

    auto& entry = it->second;
    entry.remainingMs = entry.timer->getDueIn();
    entry.timer->stop();
    entry.status = TimerStatus::Paused;
    emitEvent(TimerEvent::Paused, entry);
    return Error();
}

void TimerService::pauseTimerOrThrow(const std::string& id) {
    pauseTimer(id).throwIfError();
}

Error TimerService::resumeTimer(const std::string& id) {
    auto it = timers_.find(id);
    if (it == timers_.end()) return Error(EE_TIMER_NOT_FOUND);
    if (it->second.status != TimerStatus::Paused) return Error(EE_TIMER_NOT_PAUSED);

    auto& entry = it->second;
    entry.timeout = Duration::milliseconds(entry.remainingMs);
    entry.remainingMs = 0;
    startTimerEntry(entry);
    emitEvent(TimerEvent::Resumed, entry);
    return Error();
}

void TimerService::resumeTimerOrThrow(const std::string& id) {
    resumeTimer(id).throwIfError();
}

void TimerService::pauseAll() {
    for (auto& [id, entry] : timers_) {
        if (entry.status == TimerStatus::Running) {
            entry.remainingMs = entry.timer->getDueIn();
            entry.timer->stop();
            entry.status = TimerStatus::Paused;
            emitEvent(TimerEvent::Paused, entry);
        }
    }
}

void TimerService::resumeAll() {
    for (auto& [id, entry] : timers_) {
        if (entry.status == TimerStatus::Paused) {
            entry.timeout = Duration::milliseconds(entry.remainingMs);
            entry.remainingMs = 0;
            startTimerEntry(entry);
            emitEvent(TimerEvent::Resumed, entry);
        }
    }
}

// --- Reschedule ---

Error TimerService::rescheduleTimer(const std::string& id,
                                     Duration timeout,
                                     RepeatPolicy repeat) {
    auto it = timers_.find(id);
    if (it == timers_.end()) return Error(EE_TIMER_NOT_FOUND);

    auto& entry = it->second;
    entry.timer->stop();
    entry.timeout = timeout;
    entry.repeatPolicy = repeat;
    entry.fireCount = 0;
    entry.remainingMs = 0;
    startTimerEntry(entry);
    emitEvent(TimerEvent::Rescheduled, entry);
    return Error();
}

void TimerService::rescheduleTimerOrThrow(const std::string& id,
                                           Duration timeout,
                                           RepeatPolicy repeat) {
    rescheduleTimer(id, timeout, repeat).throwIfError();
}

// --- Query ---

bool TimerService::hasTimer(const std::string& id) const {
    auto it = timers_.find(id);
    if (it == timers_.end()) return false;
    return std::find(pendingRemoval_.begin(), pendingRemoval_.end(), id)
           == pendingRemoval_.end();
}

TimerStatus TimerService::status(const std::string& id) const {
    auto it = timers_.find(id);
    if (it == timers_.end()) return TimerStatus::Cancelled;
    return it->second.status;
}

TimerInfo TimerService::info(const std::string& id) const {
    auto it = timers_.find(id);
    if (it == timers_.end()) {
        TimerInfo ti;
        ti.id = id;
        ti.status = TimerStatus::Cancelled;
        ti.fireCount = 0;
        return ti;
    }
    return buildInfo(it->second);
}

Duration TimerService::remainingTime(const std::string& id) const {
    auto it = timers_.find(id);
    if (it == timers_.end()) return Duration();
    if (it->second.status == TimerStatus::Running && it->second.timer) {
        return Duration::milliseconds(it->second.timer->getDueIn());
    }
    if (it->second.status == TimerStatus::Paused) {
        return Duration::milliseconds(it->second.remainingMs);
    }
    return Duration();
}

std::vector<std::string> TimerService::allTimerIds() const {
    std::vector<std::string> ids;
    ids.reserve(timers_.size());
    for (const auto& [id, entry] : timers_) {
        if (std::find(pendingRemoval_.begin(), pendingRemoval_.end(), id)
            == pendingRemoval_.end()) {
            ids.push_back(id);
        }
    }
    return ids;
}

size_t TimerService::activeCount() const {
    size_t count = 0;
    for (const auto& [id, entry] : timers_) {
        if (entry.status == TimerStatus::Running ||
            entry.status == TimerStatus::Paused) {
            if (std::find(pendingRemoval_.begin(), pendingRemoval_.end(), id)
                == pendingRemoval_.end()) {
                count++;
            }
        }
    }
    return count;
}

size_t TimerService::maxTimers() const {
    return maxTimers_;
}

void TimerService::setEventListener(EventListener listener) {
    eventListener_ = std::move(listener);
}

void TimerService::clearEventListener() {
    eventListener_ = nullptr;
}

} // namespace timer
} // namespace eventengine
