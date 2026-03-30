#include <gtest/gtest.h>
#include <eventengine/loop.hpp>
#include <eventengine/timer_service.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace eventengine::timer;

class TimerServiceTest : public ::testing::Test {
protected:
    eventengine::Loop loop;
};

// --- Duration Tests ---

TEST(DurationTest, FactoryMethods) {
    EXPECT_EQ(Duration::milliseconds(500).toMilliseconds(), 500u);
    EXPECT_EQ(Duration::seconds(2).toMilliseconds(), 2000u);
    EXPECT_EQ(Duration::minutes(1).toMilliseconds(), 60000u);
    EXPECT_EQ(Duration::hours(1).toMilliseconds(), 3600000u);
    EXPECT_EQ(Duration::days(1).toMilliseconds(), 86400000u);
}

TEST(DurationTest, Arithmetic) {
    auto a = Duration::seconds(30);
    auto b = Duration::minutes(1);
    EXPECT_EQ((a + b).toMilliseconds(), 90000u);
    EXPECT_EQ((b - a).toMilliseconds(), 30000u);
    EXPECT_EQ((a - b).toMilliseconds(), 0u);  // clamped to 0
}

TEST(DurationTest, Comparison) {
    auto a = Duration::seconds(1);
    auto b = Duration::seconds(2);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a == Duration::milliseconds(1000));
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b >= a);
}

// --- RepeatPolicy Tests ---

TEST(RepeatPolicyTest, None) {
    auto p = RepeatPolicy::none();
    EXPECT_TRUE(p.isOneShot());
    EXPECT_FALSE(p.isForever());
    EXPECT_EQ(p.maxCount(), 0u);
}

TEST(RepeatPolicyTest, Forever) {
    auto p = RepeatPolicy::forever(Duration::seconds(5));
    EXPECT_FALSE(p.isOneShot());
    EXPECT_TRUE(p.isForever());
    EXPECT_EQ(p.interval().toMilliseconds(), 5000u);
}

TEST(RepeatPolicyTest, Times) {
    auto p = RepeatPolicy::times(Duration::seconds(1), 10);
    EXPECT_FALSE(p.isOneShot());
    EXPECT_FALSE(p.isForever());
    EXPECT_EQ(p.maxCount(), 10u);
    EXPECT_EQ(p.interval().toMilliseconds(), 1000u);
}

// --- TimerService: Create ---

TEST_F(TimerServiceTest, OneShotFires) {
    TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", Duration::milliseconds(10),
        [&](const TimerInfo& info) {
            fired++;
            EXPECT_EQ(info.id, "t1");
            EXPECT_EQ(info.fireCount, 1u);
            EXPECT_EQ(info.status, TimerStatus::Expired);
        });

    loop.run();
    EXPECT_EQ(fired, 1);
    EXPECT_EQ(svc.activeCount(), 0u);
}

TEST_F(TimerServiceTest, RepeatWithLimit) {
    TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", Duration::milliseconds(10),
        [&](const TimerInfo& info) { fired++; },
        RepeatPolicy::times(Duration::milliseconds(10), 5));

    loop.run();
    EXPECT_EQ(fired, 5);
    EXPECT_EQ(svc.activeCount(), 0u);
}

TEST_F(TimerServiceTest, InfiniteRepeatWithCancel) {
    TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", Duration::milliseconds(10),
        [&](const TimerInfo& info) {
            fired++;
            if (fired >= 3) svc.cancelTimer(info.id);
        },
        RepeatPolicy::forever(Duration::milliseconds(10)));

    loop.run();
    EXPECT_EQ(fired, 3);
}

TEST_F(TimerServiceTest, WithLabel) {
    TimerService svc(loop);

    svc.setTimerOrThrow("t1", Duration::milliseconds(10),
        [&](const TimerInfo& info) {
            EXPECT_EQ(info.label, "My Timer");
        },
        RepeatPolicy::none(), "My Timer");

    loop.run();
}

// --- TimerService: Error Cases ---

TEST_F(TimerServiceTest, DuplicateIdRejected) {
    TimerService svc(loop);
    auto noop = [](const TimerInfo&) {};

    svc.setTimerOrThrow("t1", Duration::seconds(10), noop);
    auto err = svc.setTimer("t1", Duration::seconds(5), noop);
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_TIMER_ALREADY_EXISTS);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, EmptyIdRejected) {
    TimerService svc(loop);
    auto err = svc.setTimer("", Duration::seconds(1),
        [](const TimerInfo&) {});
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_TIMER_INVALID_ID);
}

TEST_F(TimerServiceTest, MaxTimerLimit) {
    TimerService svc(loop, 2);
    auto noop = [](const TimerInfo&) {};

    EXPECT_FALSE(static_cast<bool>(svc.setTimer("t1", Duration::seconds(10), noop)));
    EXPECT_FALSE(static_cast<bool>(svc.setTimer("t2", Duration::seconds(10), noop)));

    auto err = svc.setTimer("t3", Duration::seconds(10), noop);
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_TIMER_LIMIT_REACHED);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, CancelNonexistent) {
    TimerService svc(loop);
    auto err = svc.cancelTimer("nonexistent");
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_TIMER_NOT_FOUND);
}

// --- TimerService: Pause/Resume ---

TEST_F(TimerServiceTest, PauseAndResume) {
    TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", Duration::milliseconds(200),
        [&](const TimerInfo&) { fired++; });

    // Pause after 50ms, resume after 100ms
    Timer helper(loop);
    int step = 0;
    helper.start([&]() {
        step++;
        if (step == 1) {
            svc.pauseTimer("t1");
            EXPECT_EQ(svc.status("t1"), TimerStatus::Paused);
        } else if (step == 2) {
            EXPECT_EQ(fired, 0);  // should not have fired while paused
            svc.resumeTimer("t1");
            EXPECT_EQ(svc.status("t1"), TimerStatus::Running);
            helper.stop();
        }
    }, 50, 50);

    loop.run();
    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, PauseAlreadyPaused) {
    TimerService svc(loop);
    svc.setTimerOrThrow("t1", Duration::seconds(10),
        [](const TimerInfo&) {});

    svc.pauseTimer("t1");
    auto err = svc.pauseTimer("t1");
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_TIMER_ALREADY_PAUSED);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, ResumeNotPaused) {
    TimerService svc(loop);
    svc.setTimerOrThrow("t1", Duration::seconds(10),
        [](const TimerInfo&) {});

    auto err = svc.resumeTimer("t1");
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_TIMER_NOT_PAUSED);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, PauseAllResumeAll) {
    TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("a", Duration::milliseconds(200),
        [&](const TimerInfo&) { fired++; });
    svc.setTimerOrThrow("b", Duration::milliseconds(200),
        [&](const TimerInfo&) { fired++; });

    Timer helper(loop);
    int step = 0;
    helper.start([&]() {
        step++;
        if (step == 1) {
            svc.pauseAll();
            EXPECT_EQ(svc.status("a"), TimerStatus::Paused);
            EXPECT_EQ(svc.status("b"), TimerStatus::Paused);
        } else if (step == 2) {
            EXPECT_EQ(fired, 0);
            svc.resumeAll();
            helper.stop();
        }
    }, 50, 50);

    loop.run();
    EXPECT_EQ(fired, 2);
}

// --- TimerService: Query ---

TEST_F(TimerServiceTest, HasTimer) {
    TimerService svc(loop);
    svc.setTimerOrThrow("t1", Duration::seconds(10),
        [](const TimerInfo&) {});

    EXPECT_TRUE(svc.hasTimer("t1"));
    EXPECT_FALSE(svc.hasTimer("t2"));

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, AllTimerIds) {
    TimerService svc(loop);
    auto noop = [](const TimerInfo&) {};

    svc.setTimerOrThrow("alpha", Duration::seconds(10), noop);
    svc.setTimerOrThrow("beta", Duration::seconds(10), noop);
    svc.setTimerOrThrow("gamma", Duration::seconds(10), noop);

    auto ids = svc.allTimerIds();
    EXPECT_EQ(ids.size(), 3u);
    std::sort(ids.begin(), ids.end());
    EXPECT_EQ(ids[0], "alpha");
    EXPECT_EQ(ids[1], "beta");
    EXPECT_EQ(ids[2], "gamma");

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, InfoQuery) {
    TimerService svc(loop);
    svc.setTimerOrThrow("t1", Duration::seconds(10),
        [](const TimerInfo&) {},
        RepeatPolicy::none(), "Test Label");

    auto ti = svc.info("t1");
    EXPECT_EQ(ti.id, "t1");
    EXPECT_EQ(ti.label, "Test Label");
    EXPECT_EQ(ti.status, TimerStatus::Running);
    EXPECT_EQ(ti.fireCount, 0u);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, RemainingTime) {
    TimerService svc(loop);
    svc.setTimerOrThrow("t1", Duration::seconds(10),
        [](const TimerInfo&) {});

    auto remaining = svc.remainingTime("t1");
    EXPECT_GT(remaining.toMilliseconds(), 0u);
    EXPECT_LE(remaining.toMilliseconds(), 10000u);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, MaxTimersAccessor) {
    TimerService svc(loop, 512);
    EXPECT_EQ(svc.maxTimers(), 512u);
}

// --- TimerService: Reschedule ---

TEST_F(TimerServiceTest, Reschedule) {
    TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", Duration::milliseconds(500),
        [&](const TimerInfo& info) {
            fired++;
            EXPECT_EQ(info.fireCount, 1u);
        });

    // Reschedule to 50ms before it fires at 500ms
    Timer helper(loop);
    helper.start([&]() {
        svc.rescheduleTimer("t1", Duration::milliseconds(50));
        helper.stop();
    }, 20);

    loop.run();
    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, RescheduleNonexistent) {
    TimerService svc(loop);
    auto err = svc.rescheduleTimer("nope", Duration::seconds(1));
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_TIMER_NOT_FOUND);
}

// --- TimerService: Event Listener ---

TEST_F(TimerServiceTest, EventListenerOneShotLifecycle) {
    std::vector<TimerEvent> events;
    TimerService svc(loop);

    svc.setEventListener([&](TimerEvent event, const TimerInfo& info) {
        events.push_back(event);
        EXPECT_EQ(info.id, "t1");
    });

    svc.setTimerOrThrow("t1", Duration::milliseconds(10),
        [](const TimerInfo&) {});

    loop.run();

    ASSERT_EQ(events.size(), 3u);
    EXPECT_EQ(events[0], TimerEvent::Created);
    EXPECT_EQ(events[1], TimerEvent::Fired);
    EXPECT_EQ(events[2], TimerEvent::Expired);
}

TEST_F(TimerServiceTest, EventListenerCancel) {
    std::vector<TimerEvent> events;
    TimerService svc(loop);

    svc.setEventListener([&](TimerEvent event, const TimerInfo&) {
        events.push_back(event);
    });

    svc.setTimerOrThrow("t1", Duration::seconds(10),
        [](const TimerInfo&) {});
    svc.cancelTimer("t1");

    loop.run();

    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0], TimerEvent::Created);
    EXPECT_EQ(events[1], TimerEvent::Cancelled);
}

TEST_F(TimerServiceTest, EventListenerPauseResume) {
    std::vector<TimerEvent> events;
    TimerService svc(loop);

    svc.setEventListener([&](TimerEvent event, const TimerInfo&) {
        events.push_back(event);
    });

    svc.setTimerOrThrow("t1", Duration::milliseconds(200),
        [](const TimerInfo&) {});

    Timer helper(loop);
    int step = 0;
    helper.start([&]() {
        step++;
        if (step == 1) {
            svc.pauseTimer("t1");
        } else if (step == 2) {
            svc.resumeTimer("t1");
            helper.stop();
        }
    }, 50, 50);

    loop.run();

    ASSERT_EQ(events.size(), 5u);
    EXPECT_EQ(events[0], TimerEvent::Created);
    EXPECT_EQ(events[1], TimerEvent::Paused);
    EXPECT_EQ(events[2], TimerEvent::Resumed);
    EXPECT_EQ(events[3], TimerEvent::Fired);
    EXPECT_EQ(events[4], TimerEvent::Expired);
}

TEST_F(TimerServiceTest, EventListenerReschedule) {
    std::vector<TimerEvent> events;
    TimerService svc(loop);

    svc.setEventListener([&](TimerEvent event, const TimerInfo&) {
        events.push_back(event);
    });

    svc.setTimerOrThrow("t1", Duration::milliseconds(500),
        [](const TimerInfo&) {});

    Timer helper(loop);
    helper.start([&]() {
        svc.rescheduleTimer("t1", Duration::milliseconds(50));
        helper.stop();
    }, 20);

    loop.run();

    ASSERT_EQ(events.size(), 4u);
    EXPECT_EQ(events[0], TimerEvent::Created);
    EXPECT_EQ(events[1], TimerEvent::Rescheduled);
    EXPECT_EQ(events[2], TimerEvent::Fired);
    EXPECT_EQ(events[3], TimerEvent::Expired);
}

TEST_F(TimerServiceTest, EventListenerRepeatFires) {
    std::vector<TimerEvent> events;
    TimerService svc(loop);

    svc.setEventListener([&](TimerEvent event, const TimerInfo&) {
        events.push_back(event);
    });

    svc.setTimerOrThrow("t1", Duration::milliseconds(10),
        [](const TimerInfo&) {},
        RepeatPolicy::times(Duration::milliseconds(10), 3));

    loop.run();

    // Created, Fired, Fired, Fired+Expired
    ASSERT_EQ(events.size(), 5u);
    EXPECT_EQ(events[0], TimerEvent::Created);
    EXPECT_EQ(events[1], TimerEvent::Fired);
    EXPECT_EQ(events[2], TimerEvent::Fired);
    EXPECT_EQ(events[3], TimerEvent::Fired);
    EXPECT_EQ(events[4], TimerEvent::Expired);
}

TEST_F(TimerServiceTest, EventListenerCancelAll) {
    std::vector<std::pair<TimerEvent, std::string>> events;
    TimerService svc(loop);

    svc.setEventListener([&](TimerEvent event, const TimerInfo& info) {
        events.push_back({event, info.id});
    });

    auto noop = [](const TimerInfo&) {};
    svc.setTimerOrThrow("a", Duration::seconds(10), noop);
    svc.setTimerOrThrow("b", Duration::seconds(10), noop);
    svc.cancelAll();

    loop.run();

    // 2 Created + 2 Cancelled
    EXPECT_EQ(events.size(), 4u);
    int created = 0, cancelled = 0;
    for (const auto& [ev, id] : events) {
        if (ev == TimerEvent::Created) created++;
        if (ev == TimerEvent::Cancelled) cancelled++;
    }
    EXPECT_EQ(created, 2);
    EXPECT_EQ(cancelled, 2);
}

TEST_F(TimerServiceTest, ClearEventListener) {
    int eventCount = 0;
    TimerService svc(loop);

    svc.setEventListener([&](TimerEvent, const TimerInfo&) {
        eventCount++;
    });

    svc.setTimerOrThrow("t1", Duration::seconds(10),
        [](const TimerInfo&) {});
    EXPECT_EQ(eventCount, 1);  // Created

    svc.clearEventListener();
    svc.cancelAll();
    EXPECT_EQ(eventCount, 1);  // no Cancelled event

    loop.run();
}

TEST_F(TimerServiceTest, EventListenerBulkPauseResume) {
    std::vector<TimerEvent> events;
    TimerService svc(loop);

    svc.setEventListener([&](TimerEvent event, const TimerInfo&) {
        events.push_back(event);
    });

    auto noop = [](const TimerInfo&) {};
    svc.setTimerOrThrow("a", Duration::seconds(10), noop);
    svc.setTimerOrThrow("b", Duration::seconds(10), noop);

    svc.pauseAll();
    svc.resumeAll();
    svc.cancelAll();

    loop.run();

    // 2 Created + 2 Paused + 2 Resumed + 2 Cancelled = 8
    EXPECT_EQ(events.size(), 8u);
}

// --- TimerService: Cancel ---

TEST_F(TimerServiceTest, CancelPreventsCallback) {
    TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", Duration::milliseconds(200),
        [&](const TimerInfo&) { fired++; });

    // Cancel after 50ms
    Timer helper(loop);
    helper.start([&]() {
        svc.cancelTimer("t1");
        helper.stop();
    }, 50);

    loop.run();
    EXPECT_EQ(fired, 0);
}

TEST_F(TimerServiceTest, CancelAll) {
    TimerService svc(loop);
    int fired = 0;
    auto cb = [&](const TimerInfo&) { fired++; };

    svc.setTimerOrThrow("a", Duration::milliseconds(200), cb);
    svc.setTimerOrThrow("b", Duration::milliseconds(200), cb);
    svc.setTimerOrThrow("c", Duration::milliseconds(200), cb);

    Timer helper(loop);
    helper.start([&]() {
        svc.cancelAll();
        helper.stop();
    }, 50);

    loop.run();
    EXPECT_EQ(fired, 0);
    EXPECT_EQ(svc.activeCount(), 0u);
}
