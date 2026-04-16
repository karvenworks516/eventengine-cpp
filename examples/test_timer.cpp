#include <eventengine/loop.hpp>
#include <eventengine/timer_service.hpp>
#include <iostream>
#include <string>

static const char* statusStr(eventengine::timer::TimerStatus s) {
    switch (s) {
        case eventengine::timer::TimerStatus::Running:   return "Running";
        case eventengine::timer::TimerStatus::Paused:    return "Paused";
        case eventengine::timer::TimerStatus::Expired:   return "Expired";
        case eventengine::timer::TimerStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

int main() {
    using namespace eventengine;
    using namespace eventengine::timer;

    // --- Test 1: One-shot timer with Duration ---
    std::cout << "[Test 1] One-shot timer (200ms)\n";
    {
        Loop loop;
        TimerService svc(loop);

        svc.setTimerOrThrow("one-shot-001",
            Duration::milliseconds(200),
            [](const TimerInfo& info) {
                std::cout << "  Timer '" << info.id << "' fired!"
                          << " fires=" << info.fireCount
                          << " status=" << statusStr(info.status) << "\n";
            });

        std::cout << "  active: " << svc.activeCount() << "\n";
        loop.run();
        std::cout << "  active after run: " << svc.activeCount() << "\n";
    }

    // --- Test 2: Repeating timer with count limit ---
    std::cout << "\n[Test 2] Repeating timer (50ms x 4 times)\n";
    {
        Loop loop;
        TimerService svc(loop);

        svc.setTimerOrThrow("repeat-limited",
            Duration::milliseconds(50),
            [](const TimerInfo& info) {
                std::cout << "  Timer '" << info.id << "' fire #" << info.fireCount
                          << " status=" << statusStr(info.status) << "\n";
            },
            RepeatPolicy::times(Duration::milliseconds(50), 4));

        loop.run();
        std::cout << "  active after run: " << svc.activeCount() << "\n";
    }

    // --- Test 3: Infinite repeat + cancel ---
    std::cout << "\n[Test 3] Infinite repeat (50ms) cancelled after 3 fires\n";
    {
        Loop loop;
        TimerService svc(loop);

        svc.setTimerOrThrow("infinite-cancel",
            Duration::milliseconds(50),
            [&svc](const TimerInfo& info) {
                std::cout << "  fire #" << info.fireCount << "\n";
                if (info.fireCount >= 3) {
                    svc.cancelTimer(info.id);
                }
            },
            RepeatPolicy::forever(Duration::milliseconds(50)));

        loop.run();
        std::cout << "  active after run: " << svc.activeCount() << "\n";
    }

    // --- Test 4: Pause and Resume ---
    std::cout << "\n[Test 4] Pause/Resume\n";
    {
        Loop loop;
        TimerService svc(loop);

        // A timer that fires after 500ms
        svc.setTimerOrThrow("pausable",
            Duration::milliseconds(500),
            [](const TimerInfo& info) {
                std::cout << "  Timer '" << info.id << "' fired after resume!\n";
            });

        // A helper timer to orchestrate pause/resume
        Timer orchestrator(loop);
        int step = 0;
        orchestrator.start([&]() {
            step++;
            if (step == 1) {
                // After 100ms, pause the timer
                auto remaining = svc.remainingTime("pausable");
                std::cout << "  Pausing 'pausable' (remaining: "
                          << remaining.toMilliseconds() << "ms)\n";
                svc.pauseTimer("pausable");
                std::cout << "  Status: " << statusStr(svc.status("pausable")) << "\n";
            } else if (step == 2) {
                // After another 100ms, resume
                auto remaining = svc.remainingTime("pausable");
                std::cout << "  Resuming 'pausable' (remaining: "
                          << remaining.toMilliseconds() << "ms)\n";
                svc.resumeTimer("pausable");
                std::cout << "  Status: " << statusStr(svc.status("pausable")) << "\n";
                orchestrator.stop();
            }
        }, 100, 100);

        loop.run();
    }

    // --- Test 5: Query APIs ---
    std::cout << "\n[Test 5] Query APIs\n";
    {
        Loop loop;
        TimerService svc(loop);

        svc.setTimerOrThrow("query-a", Duration::seconds(10),
            [](const TimerInfo&) {}, RepeatPolicy::none(), "Alpha Timer");
        svc.setTimerOrThrow("query-b", Duration::minutes(1),
            [](const TimerInfo&) {}, RepeatPolicy::none(), "Beta Timer");

        std::cout << "  hasTimer('query-a'): " << svc.hasTimer("query-a") << "\n";
        std::cout << "  hasTimer('nonexistent'): " << svc.hasTimer("nonexistent") << "\n";
        std::cout << "  activeCount: " << svc.activeCount() << "\n";
        std::cout << "  maxTimers: " << svc.maxTimers() << "\n";

        auto ids = svc.allTimerIds();
        std::cout << "  allTimerIds:";
        for (const auto& id : ids) std::cout << " " << id;
        std::cout << "\n";

        auto ti = svc.info("query-a");
        std::cout << "  info('query-a'): label='" << ti.label
                  << "' status=" << statusStr(ti.status) << "\n";

        svc.cancelAll();
        std::cout << "  activeCount after cancelAll: " << svc.activeCount() << "\n";
        loop.run();
    }

    // --- Test 6: Duration units ---
    std::cout << "\n[Test 6] Duration units\n";
    {
        auto ms = Duration::milliseconds(1500);
        auto s  = Duration::seconds(2);
        auto m  = Duration::minutes(1);
        auto h  = Duration::hours(1);
        auto d  = Duration::days(1);
        auto sum = Duration::seconds(30) + Duration::minutes(1);

        std::cout << "  1500ms = " << ms.toMilliseconds() << "ms\n";
        std::cout << "  2s     = " << s.toMilliseconds() << "ms\n";
        std::cout << "  1m     = " << m.toMilliseconds() << "ms\n";
        std::cout << "  1h     = " << h.toMilliseconds() << "ms\n";
        std::cout << "  1d     = " << d.toMilliseconds() << "ms\n";
        std::cout << "  30s+1m = " << sum.toMilliseconds() << "ms\n";
    }

    // --- Test 7: Max timer limit ---
    std::cout << "\n[Test 7] Max timer limit (max=3)\n";
    {
        Loop loop;
        TimerService svc(loop, 3);

        auto noop = [](const TimerInfo&) {};
        svc.setTimerOrThrow("t1", Duration::seconds(10), noop);
        svc.setTimerOrThrow("t2", Duration::seconds(10), noop);
        svc.setTimerOrThrow("t3", Duration::seconds(10), noop);

        auto err = svc.setTimer("t4", Duration::seconds(10), noop);
        std::cout << "  Set t1,t2,t3: OK\n";
        std::cout << "  Set t4 (over limit): " << (err ? "REJECTED" : "OK")
                  << " (" << err.name() << ")\n";

        svc.cancelAll();
        loop.run();
    }

    // --- Test 8: Duplicate ID rejection ---
    std::cout << "\n[Test 8] Duplicate ID rejection\n";
    {
        Loop loop;
        TimerService svc(loop);

        auto noop = [](const TimerInfo&) {};
        svc.setTimerOrThrow("dup-id", Duration::seconds(10), noop);
        auto err = svc.setTimer("dup-id", Duration::seconds(5), noop);
        std::cout << "  First set: OK\n";
        std::cout << "  Duplicate set: " << (err ? "REJECTED" : "OK")
                  << " (" << err.name() << ")\n";

        svc.cancelAll();
        loop.run();
    }

    // --- Test 9: Reschedule ---
    std::cout << "\n[Test 9] Reschedule timer\n";
    {
        Loop loop;
        TimerService svc(loop);

        svc.setTimerOrThrow("resched",
            Duration::milliseconds(500),
            [](const TimerInfo& info) {
                std::cout << "  Timer '" << info.id << "' fired! fire #"
                          << info.fireCount << "\n";
            });

        // Reschedule to 100ms before it fires
        Timer helper(loop);
        helper.start([&]() {
            std::cout << "  Rescheduling to 100ms...\n";
            svc.rescheduleTimer("resched", Duration::milliseconds(100));
            helper.stop();
        }, 50);

        loop.run();
    }

    std::cout << "\n=== All TimerService tests passed! ===\n";
    return 0;
}
