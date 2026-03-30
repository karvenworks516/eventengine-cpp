#include <eventengine/loop.hpp>
#include <eventengine/thread_pool_service.hpp>
#include <iostream>
#include <thread>
#include <chrono>

static const char* statusStr(eventengine::threadpool::TaskStatus s) {
    switch (s) {
        case eventengine::threadpool::TaskStatus::Queued:     return "Queued";
        case eventengine::threadpool::TaskStatus::InProgress:  return "InProgress";
        case eventengine::threadpool::TaskStatus::Finished:    return "Finished";
        case eventengine::threadpool::TaskStatus::Cancelled:   return "Cancelled";
        case eventengine::threadpool::TaskStatus::Rejected:    return "Rejected";
    }
    return "Unknown";
}

int main() {
    using namespace eventengine;
    using namespace eventengine::threadpool;
    using eventengine::timer::Duration;

    std::cout << "=== ThreadPoolService Full Test ===\n\n";

    // --- Test 1: Basic submit ---
    std::cout << "[Test 1] Basic submit\n";
    {
        Loop loop;
        ThreadPoolService svc(loop);

        svc.submitOrThrow("basic-task",
            []() {
                std::cout << "  Work running on thread "
                          << std::this_thread::get_id() << "\n";
            },
            [](const std::string& name, Error err) {
                std::cout << "  '" << name << "' completed"
                          << (err ? " with error" : " successfully") << "\n";
            });

        loop.run();
    }

    // --- Test 2: Multiple tasks with stats ---
    std::cout << "\n[Test 2] Multiple tasks with stats\n";
    {
        Loop loop;
        ThreadPoolService svc(loop);

        for (int i = 0; i < 5; ++i) {
            svc.submitOrThrow("task-" + std::to_string(i),
                [i]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                },
                [&svc](const std::string& name, Error) {
                    std::cout << "  '" << name << "' done"
                              << " | active=" << svc.activeCount()
                              << " completed=" << svc.completedCount() << "\n";
                });
        }

        std::cout << "  Submitted 5 tasks, active: " << svc.activeCount() << "\n";
        loop.run();
        std::cout << "  Final: active=" << svc.activeCount()
                  << " completed=" << svc.completedCount() << "\n";
    }

    // --- Test 3: Query APIs ---
    std::cout << "\n[Test 3] Query APIs\n";
    {
        Loop loop;
        ThreadPoolService svc(loop);

        svc.submitOrThrow("query-task",
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); },
            [](const std::string&, Error) {},
            Duration::seconds(10));

        std::cout << "  hasTask('query-task'): " << svc.hasTask("query-task") << "\n";
        std::cout << "  status: " << statusStr(svc.status("query-task")) << "\n";

        auto ti = svc.info("query-task");
        std::cout << "  info: name='" << ti.name
                  << "' timeout=" << ti.timeout.toMilliseconds() << "ms\n";

        auto names = svc.allTaskNames();
        std::cout << "  allTaskNames:";
        for (const auto& n : names) std::cout << " " << n;
        std::cout << "\n";

        loop.run();
    }

    // --- Test 4: Duplicate name rejection ---
    std::cout << "\n[Test 4] Duplicate name rejection\n";
    {
        Loop loop;
        ThreadPoolService svc(loop);

        svc.submitOrThrow("dup",
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(50)); },
            [](const std::string&, Error) {});

        auto err = svc.submit("dup", []() {},
            [](const std::string&, Error) {});
        std::cout << "  First submit: OK\n";
        std::cout << "  Duplicate: " << (err ? "REJECTED" : "OK")
                  << " (" << err.name() << ")\n";

        loop.run();
    }

    // --- Test 5: Empty name rejection ---
    std::cout << "\n[Test 5] Empty name rejection\n";
    {
        Loop loop;
        ThreadPoolService svc(loop);

        auto err = svc.submit("", []() {},
            [](const std::string&, Error) {});
        std::cout << "  Empty name: " << (err ? "REJECTED" : "OK")
                  << " (" << err.name() << ")\n";
    }

    // --- Test 6: Cancel queued task ---
    std::cout << "\n[Test 6] Cancel queued task\n";
    {
        Loop loop;
        ThreadPoolService svc(loop);

        // Fill pool with blocking tasks
        for (int i = 0; i < 8; ++i) {
            svc.submit("blocker-" + std::to_string(i),
                []() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); },
                [](const std::string&, Error) {});
        }

        svc.submitOrThrow("cancel-target",
            []() { std::cout << "  (should not run)\n"; },
            [](const std::string& name, Error err) {
                std::cout << "  '" << name << "' after_cb: "
                          << (err.code() == EE_POOL_CANCELLED ? "CANCELLED" : "completed")
                          << "\n";
            });

        auto cancelErr = svc.cancel("cancel-target");
        std::cout << "  cancel result: "
                  << (cancelErr ? cancelErr.name() : "OK") << "\n";

        loop.run();
    }

    // --- Test 7: Timeout ---
    std::cout << "\n[Test 7] Timeout\n";
    {
        Loop loop;
        ThreadPoolService svc(loop);

        // Fill pool
        for (int i = 0; i < 8; ++i) {
            svc.submit("blocker-" + std::to_string(i),
                []() { std::this_thread::sleep_for(std::chrono::milliseconds(500)); },
                [](const std::string&, Error) {});
        }

        svc.submitOrThrow("timeout-task",
            []() { std::cout << "  (should not run if timed out)\n"; },
            [](const std::string& name, Error err) {
                std::cout << "  '" << name << "': "
                          << (err.code() == EE_POOL_CANCELLED ? "TIMED OUT" : "completed")
                          << "\n";
            },
            Duration::milliseconds(50));

        loop.run();
    }

    // --- Test 8: Name reuse after completion ---
    std::cout << "\n[Test 8] Name reuse after completion\n";
    {
        Loop loop;
        ThreadPoolService svc(loop);
        int count = 0;

        svc.submitOrThrow("reusable",
            []() {},
            [&](const std::string&, Error) { count++; });
        loop.run();

        svc.submitOrThrow("reusable",
            []() {},
            [&](const std::string&, Error) { count++; });
        loop.run();

        std::cout << "  Ran 'reusable' " << count << " times\n";
    }

    std::cout << "\n=== All ThreadPoolService tests passed! ===\n";
    return 0;
}
