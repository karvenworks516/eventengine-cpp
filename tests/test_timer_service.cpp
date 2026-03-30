#include <gtest/gtest.h>
#include <eventengine/loop.hpp>
#include <eventengine/timer_service.hpp>
#include <string>
#include <vector>
#include <algorithm>

class TimerServiceTest : public ::testing::Test {
protected:
    eventengine::Loop loop;
};

// --- Duration Tests ---

TEST(DurationTest, FactoryMethods) {
    EXPECT_EQ(eventengine::Duration::milliseconds(500).toMilliseconds(), 500u);
    EXPECT_EQ(eventengine::Duration::seconds(2).toMilliseconds(), 2000u);
    EXPECT_EQ(eventengine::Duration::minutes(1).toMilliseconds(), 60000u);
    EXPECT_EQ(eventengine::Duration::hours(1).toMilliseconds(), 3600000u);
    EXPECT_EQ(eventengine::Duration::days(1).toMilliseconds(), 86400000u);
}

TEST(DurationTest, Arithmetic) {
    auto a = eventengine::Duration::seconds(30);
    auto b = eventengine::Duration::minutes(1);
    EXPECT_EQ((a + b).toMilliseconds(), 90000u);
    EXPECT_EQ((b - a).toMilliseconds(), 30000u);
    EXPECT_EQ((a - b).toMilliseconds(), 0u);  // clamped to 0
}

TEST(DurationTest, Comparison) {
    auto a = eventengine::Duration::seconds(1);
    auto b = eventengine::Duration::seconds(2);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a == eventengine::Duration::milliseconds(1000));
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b >= a);
}

// --- RepeatPolicy Tests ---

TEST(RepeatPolicyTest, None) {
    auto p = eventengine::RepeatPolicy::none();
    EXPECT_TRUE(p.isOneShot());
    EXPECT_FALSE(p.isForever());
    EXPECT_EQ(p.maxCount(), 0u);
}

TEST(RepeatPolicyTest, Forever) {
    auto p = eventengine::RepeatPolicy::forever(eventengine::Duration::seconds(5));
    EXPECT_FALSE(p.isOneShot());
    EXPECT_TRUE(p.isForever());
    EXPECT_EQ(p.interval().toMilliseconds(), 5000u);
}

TEST(RepeatPolicyTest, Times) {
    auto p = eventengine::RepeatPolicy::times(eventengine::Duration::seconds(1), 10);
    EXPECT_FALSE(p.isOneShot());
    EXPECT_FALSE(p.isForever());
    EXPECT_EQ(p.maxCount(), 10u);
    EXPECT_EQ(p.interval().toMilliseconds(), 1000u);
}

// --- TimerService: Create ---

TEST_F(TimerServiceTest, OneShotFires) {
    eventengine::TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(10),
        [&](const eventengine::TimerInfo& info) {
            fired++;
            EXPECT_EQ(info.id, "t1");
            EXPECT_EQ(info.fireCount, 1u);
            EXPECT_EQ(info.status, eventengine::TimerStatus::Expired);
        });

    loop.run();
    EXPECT_EQ(fired, 1);
    EXPECT_EQ(svc.activeCount(), 0u);
}

TEST_F(TimerServiceTest, RepeatWithLimit) {
    eventengine::TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(10),
        [&](const eventengine::TimerInfo& info) { fired++; },
        eventengine::RepeatPolicy::times(eventengine::Duration::milliseconds(10), 5));

    loop.run();
    EXPECT_EQ(fired, 5);
    EXPECT_EQ(svc.activeCount(), 0u);
}

TEST_F(TimerServiceTest, InfiniteRepeatWithCancel) {
    eventengine::TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(10),
        [&](const eventengine::TimerInfo& info) {
            fired++;
            if (fired >= 3) svc.cancelTimer(info.id);
        },
        eventengine::RepeatPolicy::forever(eventengine::Duration::milliseconds(10)));

    loop.run();
    EXPECT_EQ(fired, 3);
}

TEST_F(TimerServiceTest, WithLabel) {
    eventengine::TimerService svc(loop);

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(10),
        [&](const eventengine::TimerInfo& info) {
            EXPECT_EQ(info.label, "My Timer");
        },
        eventengine::RepeatPolicy::none(), "My Timer");

    loop.run();
}

// --- TimerService: Error Cases ---

TEST_F(TimerServiceTest, DuplicateIdRejected) {
    eventengine::TimerService svc(loop);
    auto noop = [](const eventengine::TimerInfo&) {};

    svc.setTimerOrThrow("t1", eventengine::Duration::seconds(10), noop);
    auto err = svc.setTimer("t1", eventengine::Duration::seconds(5), noop);
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), UV_EEXIST);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, EmptyIdRejected) {
    eventengine::TimerService svc(loop);
    auto err = svc.setTimer("", eventengine::Duration::seconds(1),
        [](const eventengine::TimerInfo&) {});
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), UV_EINVAL);
}

TEST_F(TimerServiceTest, MaxTimerLimit) {
    eventengine::TimerService svc(loop, 2);
    auto noop = [](const eventengine::TimerInfo&) {};

    EXPECT_FALSE(static_cast<bool>(svc.setTimer("t1", eventengine::Duration::seconds(10), noop)));
    EXPECT_FALSE(static_cast<bool>(svc.setTimer("t2", eventengine::Duration::seconds(10), noop)));

    auto err = svc.setTimer("t3", eventengine::Duration::seconds(10), noop);
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), UV_ENOSPC);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, CancelNonexistent) {
    eventengine::TimerService svc(loop);
    auto err = svc.cancelTimer("nonexistent");
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), UV_ENOENT);
}

// --- TimerService: Pause/Resume ---

TEST_F(TimerServiceTest, PauseAndResume) {
    eventengine::TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(200),
        [&](const eventengine::TimerInfo&) { fired++; });

    // Pause after 50ms, resume after 100ms
    eventengine::Timer helper(loop);
    int step = 0;
    helper.start([&]() {
        step++;
        if (step == 1) {
            svc.pauseTimer("t1");
            EXPECT_EQ(svc.status("t1"), eventengine::TimerStatus::Paused);
        } else if (step == 2) {
            EXPECT_EQ(fired, 0);  // should not have fired while paused
            svc.resumeTimer("t1");
            EXPECT_EQ(svc.status("t1"), eventengine::TimerStatus::Running);
            helper.stop();
        }
    }, 50, 50);

    loop.run();
    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, PauseAlreadyPaused) {
    eventengine::TimerService svc(loop);
    svc.setTimerOrThrow("t1", eventengine::Duration::seconds(10),
        [](const eventengine::TimerInfo&) {});

    svc.pauseTimer("t1");
    auto err = svc.pauseTimer("t1");
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), UV_EALREADY);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, ResumeNotPaused) {
    eventengine::TimerService svc(loop);
    svc.setTimerOrThrow("t1", eventengine::Duration::seconds(10),
        [](const eventengine::TimerInfo&) {});

    auto err = svc.resumeTimer("t1");
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), UV_EALREADY);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, PauseAllResumeAll) {
    eventengine::TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("a", eventengine::Duration::milliseconds(200),
        [&](const eventengine::TimerInfo&) { fired++; });
    svc.setTimerOrThrow("b", eventengine::Duration::milliseconds(200),
        [&](const eventengine::TimerInfo&) { fired++; });

    eventengine::Timer helper(loop);
    int step = 0;
    helper.start([&]() {
        step++;
        if (step == 1) {
            svc.pauseAll();
            EXPECT_EQ(svc.status("a"), eventengine::TimerStatus::Paused);
            EXPECT_EQ(svc.status("b"), eventengine::TimerStatus::Paused);
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
    eventengine::TimerService svc(loop);
    svc.setTimerOrThrow("t1", eventengine::Duration::seconds(10),
        [](const eventengine::TimerInfo&) {});

    EXPECT_TRUE(svc.hasTimer("t1"));
    EXPECT_FALSE(svc.hasTimer("t2"));

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, AllTimerIds) {
    eventengine::TimerService svc(loop);
    auto noop = [](const eventengine::TimerInfo&) {};

    svc.setTimerOrThrow("alpha", eventengine::Duration::seconds(10), noop);
    svc.setTimerOrThrow("beta", eventengine::Duration::seconds(10), noop);
    svc.setTimerOrThrow("gamma", eventengine::Duration::seconds(10), noop);

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
    eventengine::TimerService svc(loop);
    svc.setTimerOrThrow("t1", eventengine::Duration::seconds(10),
        [](const eventengine::TimerInfo&) {},
        eventengine::RepeatPolicy::none(), "Test Label");

    auto ti = svc.info("t1");
    EXPECT_EQ(ti.id, "t1");
    EXPECT_EQ(ti.label, "Test Label");
    EXPECT_EQ(ti.status, eventengine::TimerStatus::Running);
    EXPECT_EQ(ti.fireCount, 0u);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, RemainingTime) {
    eventengine::TimerService svc(loop);
    svc.setTimerOrThrow("t1", eventengine::Duration::seconds(10),
        [](const eventengine::TimerInfo&) {});

    auto remaining = svc.remainingTime("t1");
    EXPECT_GT(remaining.toMilliseconds(), 0u);
    EXPECT_LE(remaining.toMilliseconds(), 10000u);

    svc.cancelAll();
    loop.run();
}

TEST_F(TimerServiceTest, MaxTimersAccessor) {
    eventengine::TimerService svc(loop, 512);
    EXPECT_EQ(svc.maxTimers(), 512u);
}

// --- TimerService: Reschedule ---

TEST_F(TimerServiceTest, Reschedule) {
    eventengine::TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(500),
        [&](const eventengine::TimerInfo& info) {
            fired++;
            EXPECT_EQ(info.fireCount, 1u);
        });

    // Reschedule to 50ms before it fires at 500ms
    eventengine::Timer helper(loop);
    helper.start([&]() {
        svc.rescheduleTimer("t1", eventengine::Duration::milliseconds(50));
        helper.stop();
    }, 20);

    loop.run();
    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, RescheduleNonexistent) {
    eventengine::TimerService svc(loop);
    auto err = svc.rescheduleTimer("nope", eventengine::Duration::seconds(1));
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), UV_ENOENT);
}

// --- TimerService: Event Listener ---

TEST_F(TimerServiceTest, EventListenerOneShotLifecycle) {
    std::vector<eventengine::TimerEvent> events;
    eventengine::TimerService svc(loop);

    svc.setEventListener([&](eventengine::TimerEvent event, const eventengine::TimerInfo& info) {
        events.push_back(event);
        EXPECT_EQ(info.id, "t1");
    });

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(10),
        [](const eventengine::TimerInfo&) {});

    loop.run();

    ASSERT_EQ(events.size(), 3u);
    EXPECT_EQ(events[0], eventengine::TimerEvent::Created);
    EXPECT_EQ(events[1], eventengine::TimerEvent::Fired);
    EXPECT_EQ(events[2], eventengine::TimerEvent::Expired);
}

TEST_F(TimerServiceTest, EventListenerCancel) {
    std::vector<eventengine::TimerEvent> events;
    eventengine::TimerService svc(loop);

    svc.setEventListener([&](eventengine::TimerEvent event, const eventengine::TimerInfo&) {
        events.push_back(event);
    });

    svc.setTimerOrThrow("t1", eventengine::Duration::seconds(10),
        [](const eventengine::TimerInfo&) {});
    svc.cancelTimer("t1");

    loop.run();

    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0], eventengine::TimerEvent::Created);
    EXPECT_EQ(events[1], eventengine::TimerEvent::Cancelled);
}

TEST_F(TimerServiceTest, EventListenerPauseResume) {
    std::vector<eventengine::TimerEvent> events;
    eventengine::TimerService svc(loop);

    svc.setEventListener([&](eventengine::TimerEvent event, const eventengine::TimerInfo&) {
        events.push_back(event);
    });

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(200),
        [](const eventengine::TimerInfo&) {});

    eventengine::Timer helper(loop);
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
    EXPECT_EQ(events[0], eventengine::TimerEvent::Created);
    EXPECT_EQ(events[1], eventengine::TimerEvent::Paused);
    EXPECT_EQ(events[2], eventengine::TimerEvent::Resumed);
    EXPECT_EQ(events[3], eventengine::TimerEvent::Fired);
    EXPECT_EQ(events[4], eventengine::TimerEvent::Expired);
}

TEST_F(TimerServiceTest, EventListenerReschedule) {
    std::vector<eventengine::TimerEvent> events;
    eventengine::TimerService svc(loop);

    svc.setEventListener([&](eventengine::TimerEvent event, const eventengine::TimerInfo&) {
        events.push_back(event);
    });

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(500),
        [](const eventengine::TimerInfo&) {});

    eventengine::Timer helper(loop);
    helper.start([&]() {
        svc.rescheduleTimer("t1", eventengine::Duration::milliseconds(50));
        helper.stop();
    }, 20);

    loop.run();

    ASSERT_EQ(events.size(), 4u);
    EXPECT_EQ(events[0], eventengine::TimerEvent::Created);
    EXPECT_EQ(events[1], eventengine::TimerEvent::Rescheduled);
    EXPECT_EQ(events[2], eventengine::TimerEvent::Fired);
    EXPECT_EQ(events[3], eventengine::TimerEvent::Expired);
}

TEST_F(TimerServiceTest, EventListenerRepeatFires) {
    std::vector<eventengine::TimerEvent> events;
    eventengine::TimerService svc(loop);

    svc.setEventListener([&](eventengine::TimerEvent event, const eventengine::TimerInfo&) {
        events.push_back(event);
    });

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(10),
        [](const eventengine::TimerInfo&) {},
        eventengine::RepeatPolicy::times(eventengine::Duration::milliseconds(10), 3));

    loop.run();

    // Created, Fired, Fired, Fired+Expired
    ASSERT_EQ(events.size(), 5u);
    EXPECT_EQ(events[0], eventengine::TimerEvent::Created);
    EXPECT_EQ(events[1], eventengine::TimerEvent::Fired);
    EXPECT_EQ(events[2], eventengine::TimerEvent::Fired);
    EXPECT_EQ(events[3], eventengine::TimerEvent::Fired);
    EXPECT_EQ(events[4], eventengine::TimerEvent::Expired);
}

TEST_F(TimerServiceTest, EventListenerCancelAll) {
    std::vector<std::pair<eventengine::TimerEvent, std::string>> events;
    eventengine::TimerService svc(loop);

    svc.setEventListener([&](eventengine::TimerEvent event, const eventengine::TimerInfo& info) {
        events.push_back({event, info.id});
    });

    auto noop = [](const eventengine::TimerInfo&) {};
    svc.setTimerOrThrow("a", eventengine::Duration::seconds(10), noop);
    svc.setTimerOrThrow("b", eventengine::Duration::seconds(10), noop);
    svc.cancelAll();

    loop.run();

    // 2 Created + 2 Cancelled
    EXPECT_EQ(events.size(), 4u);
    int created = 0, cancelled = 0;
    for (const auto& [ev, id] : events) {
        if (ev == eventengine::TimerEvent::Created) created++;
        if (ev == eventengine::TimerEvent::Cancelled) cancelled++;
    }
    EXPECT_EQ(created, 2);
    EXPECT_EQ(cancelled, 2);
}

TEST_F(TimerServiceTest, ClearEventListener) {
    int eventCount = 0;
    eventengine::TimerService svc(loop);

    svc.setEventListener([&](eventengine::TimerEvent, const eventengine::TimerInfo&) {
        eventCount++;
    });

    svc.setTimerOrThrow("t1", eventengine::Duration::seconds(10),
        [](const eventengine::TimerInfo&) {});
    EXPECT_EQ(eventCount, 1);  // Created

    svc.clearEventListener();
    svc.cancelAll();
    EXPECT_EQ(eventCount, 1);  // no Cancelled event

    loop.run();
}

TEST_F(TimerServiceTest, EventListenerBulkPauseResume) {
    std::vector<eventengine::TimerEvent> events;
    eventengine::TimerService svc(loop);

    svc.setEventListener([&](eventengine::TimerEvent event, const eventengine::TimerInfo&) {
        events.push_back(event);
    });

    auto noop = [](const eventengine::TimerInfo&) {};
    svc.setTimerOrThrow("a", eventengine::Duration::seconds(10), noop);
    svc.setTimerOrThrow("b", eventengine::Duration::seconds(10), noop);

    svc.pauseAll();
    svc.resumeAll();
    svc.cancelAll();

    loop.run();

    // 2 Created + 2 Paused + 2 Resumed + 2 Cancelled = 8
    EXPECT_EQ(events.size(), 8u);
}

// --- TimerService: Cancel ---

TEST_F(TimerServiceTest, CancelPreventsCallback) {
    eventengine::TimerService svc(loop);
    int fired = 0;

    svc.setTimerOrThrow("t1", eventengine::Duration::milliseconds(200),
        [&](const eventengine::TimerInfo&) { fired++; });

    // Cancel after 50ms
    eventengine::Timer helper(loop);
    helper.start([&]() {
        svc.cancelTimer("t1");
        helper.stop();
    }, 50);

    loop.run();
    EXPECT_EQ(fired, 0);
}

TEST_F(TimerServiceTest, CancelAll) {
    eventengine::TimerService svc(loop);
    int fired = 0;
    auto cb = [&](const eventengine::TimerInfo&) { fired++; };

    svc.setTimerOrThrow("a", eventengine::Duration::milliseconds(200), cb);
    svc.setTimerOrThrow("b", eventengine::Duration::milliseconds(200), cb);
    svc.setTimerOrThrow("c", eventengine::Duration::milliseconds(200), cb);

    eventengine::Timer helper(loop);
    helper.start([&]() {
        svc.cancelAll();
        helper.stop();
    }, 50);

    loop.run();
    EXPECT_EQ(fired, 0);
    EXPECT_EQ(svc.activeCount(), 0u);
}
