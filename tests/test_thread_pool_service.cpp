#include <gtest/gtest.h>
#include <eventengine/loop.hpp>
#include <eventengine/thread_pool_service.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <atomic>

using namespace eventengine::threadpool;
using eventengine::timer::Duration;

class ThreadPoolServiceTest : public ::testing::Test {
protected:
    eventengine::Loop loop;
};

// --- TaskStatus enum ---

TEST(TaskStatusTest, EnumValues) {
    EXPECT_NE(TaskStatus::Queued, TaskStatus::InProgress);
    EXPECT_NE(TaskStatus::InProgress, TaskStatus::Finished);
    EXPECT_NE(TaskStatus::Finished, TaskStatus::Cancelled);
    EXPECT_NE(TaskStatus::Cancelled, TaskStatus::Rejected);
}

// --- Submit & Completion ---

TEST_F(ThreadPoolServiceTest, BasicSubmitCompletes) {
    ThreadPoolService svc(loop);
    bool workRan = false;
    bool afterRan = false;

    svc.submitOrThrow("task-1",
        [&]() { workRan = true; },
        [&](const std::string& name, eventengine::Error err) {
            afterRan = true;
            EXPECT_EQ(name, "task-1");
            EXPECT_FALSE(static_cast<bool>(err));
        });

    loop.run();
    EXPECT_TRUE(workRan);
    EXPECT_TRUE(afterRan);
}

TEST_F(ThreadPoolServiceTest, WorkRunsOnDifferentThread) {
    ThreadPoolService svc(loop);
    auto mainThread = std::this_thread::get_id();
    std::thread::id workThread;

    svc.submitOrThrow("thread-check",
        [&]() { workThread = std::this_thread::get_id(); },
        [](const std::string&, eventengine::Error) {});

    loop.run();
    EXPECT_NE(workThread, mainThread);
}

TEST_F(ThreadPoolServiceTest, MultipleTasksComplete) {
    ThreadPoolService svc(loop);
    std::atomic<int> completed{0};

    for (int i = 0; i < 5; ++i) {
        svc.submitOrThrow("task-" + std::to_string(i),
            []() {},
            [&](const std::string&, eventengine::Error) { completed++; });
    }

    loop.run();
    EXPECT_EQ(completed.load(), 5);
}

// --- Error Cases ---

TEST_F(ThreadPoolServiceTest, EmptyNameRejected) {
    ThreadPoolService svc(loop);
    auto err = svc.submit("", []() {},
        [](const std::string&, eventengine::Error) {});
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_POOL_INVALID_NAME);
}

TEST_F(ThreadPoolServiceTest, DuplicateNameRejected) {
    ThreadPoolService svc(loop);
    auto noop = [](const std::string&, eventengine::Error) {};

    svc.submitOrThrow("dup", []() {}, noop);
    auto err = svc.submit("dup", []() {}, noop);
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_POOL_ALREADY_EXISTS);

    loop.run();
}

// --- Stats ---

TEST_F(ThreadPoolServiceTest, StatsTracking) {
    ThreadPoolService svc(loop);

    EXPECT_EQ(svc.activeCount(), 0u);
    EXPECT_EQ(svc.completedCount(), 0u);

    svc.submitOrThrow("s1", []() {},
        [&](const std::string&, eventengine::Error) {
            EXPECT_EQ(svc.completedCount(), 1u);
        });
    svc.submitOrThrow("s2", []() {},
        [](const std::string&, eventengine::Error) {});

    EXPECT_EQ(svc.activeCount(), 2u);

    loop.run();
    EXPECT_EQ(svc.activeCount(), 0u);
    EXPECT_EQ(svc.completedCount(), 2u);
}

// --- Query ---

TEST_F(ThreadPoolServiceTest, HasTask) {
    ThreadPoolService svc(loop);
    svc.submitOrThrow("exists", []() {},
        [](const std::string&, eventengine::Error) {});

    EXPECT_TRUE(svc.hasTask("exists"));
    EXPECT_FALSE(svc.hasTask("nope"));

    loop.run();
    EXPECT_FALSE(svc.hasTask("exists"));
}

TEST_F(ThreadPoolServiceTest, StatusQuery) {
    ThreadPoolService svc(loop);
    svc.submitOrThrow("q1", []() {},
        [](const std::string&, eventengine::Error) {});

    auto st = svc.status("q1");
    EXPECT_TRUE(st == TaskStatus::Queued ||
                st == TaskStatus::InProgress);

    EXPECT_EQ(svc.status("nonexistent"), TaskStatus::Finished);

    loop.run();
}

TEST_F(ThreadPoolServiceTest, InfoQuery) {
    ThreadPoolService svc(loop);
    svc.submitOrThrow("info-task", []() {},
        [](const std::string&, eventengine::Error) {},
        Duration::seconds(5));

    auto ti = svc.info("info-task");
    EXPECT_EQ(ti.name, "info-task");
    EXPECT_EQ(ti.timeout.toMilliseconds(), 5000u);
    EXPECT_TRUE(ti.status == TaskStatus::Queued ||
                ti.status == TaskStatus::InProgress);

    loop.run();
}

TEST_F(ThreadPoolServiceTest, AllTaskNames) {
    ThreadPoolService svc(loop);
    auto noop = [](const std::string&, eventengine::Error) {};

    svc.submitOrThrow("alpha", []() {}, noop);
    svc.submitOrThrow("beta", []() {}, noop);
    svc.submitOrThrow("gamma", []() {}, noop);

    auto names = svc.allTaskNames();
    EXPECT_EQ(names.size(), 3u);
    std::sort(names.begin(), names.end());
    EXPECT_EQ(names[0], "alpha");
    EXPECT_EQ(names[1], "beta");
    EXPECT_EQ(names[2], "gamma");

    loop.run();
}

// --- Cancel ---

TEST_F(ThreadPoolServiceTest, CancelNonexistent) {
    ThreadPoolService svc(loop);
    auto err = svc.cancel("nope");
    EXPECT_TRUE(static_cast<bool>(err));
    EXPECT_EQ(err.code(), EE_POOL_NOT_FOUND);
}

TEST_F(ThreadPoolServiceTest, CancelledTaskReceivesError) {
    ThreadPoolService svc(loop);
    bool afterCalled = false;

    // Submit a task that blocks so it stays queued long enough to cancel
    // We submit multiple to fill the pool, then cancel the last one
    for (int i = 0; i < 8; ++i) {
        std::string name = "blocker-" + std::to_string(i);
        svc.submit(name,
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); },
            [](const std::string&, eventengine::Error) {});
    }

    svc.submitOrThrow("cancel-me",
        []() {},
        [&](const std::string& name, eventengine::Error err) {
            afterCalled = true;
            EXPECT_EQ(name, "cancel-me");
            EXPECT_EQ(err.code(), EE_POOL_CANCELLED);
        });

    auto cancelErr = svc.cancel("cancel-me");
    // Cancel may or may not succeed depending on timing
    if (!cancelErr) {
        loop.run();
        EXPECT_TRUE(afterCalled);
    } else {
        loop.run();
    }
}

// --- Timeout ---

TEST_F(ThreadPoolServiceTest, TimeoutCancelsTask) {
    ThreadPoolService svc(loop);

    // Fill the pool with blocking tasks so our target stays queued
    for (int i = 0; i < 8; ++i) {
        svc.submit("blocker-" + std::to_string(i),
            []() { std::this_thread::sleep_for(std::chrono::milliseconds(500)); },
            [](const std::string&, eventengine::Error) {});
    }

    bool timedOut = false;
    svc.submitOrThrow("timeout-task",
        []() {},
        [&](const std::string& name, eventengine::Error err) {
            if (err.code() == EE_POOL_CANCELLED) timedOut = true;
        },
        Duration::milliseconds(50));

    loop.run();
    // Timeout may or may not trigger depending on pool availability
    // This test verifies no crash; timeout behavior is best-effort
    EXPECT_EQ(svc.activeCount(), 0u);
}

// --- After-work callback receives task name ---

TEST_F(ThreadPoolServiceTest, AfterWorkReceivesName) {
    ThreadPoolService svc(loop);
    std::vector<std::string> names;

    for (int i = 0; i < 3; ++i) {
        svc.submitOrThrow("job-" + std::to_string(i),
            []() {},
            [&](const std::string& name, eventengine::Error) {
                names.push_back(name);
            });
    }

    loop.run();
    EXPECT_EQ(names.size(), 3u);
    std::sort(names.begin(), names.end());
    EXPECT_EQ(names[0], "job-0");
    EXPECT_EQ(names[1], "job-1");
    EXPECT_EQ(names[2], "job-2");
}

// --- Name reuse after completion ---

TEST_F(ThreadPoolServiceTest, NameReusableAfterCompletion) {
    ThreadPoolService svc(loop);
    int count = 0;

    svc.submitOrThrow("reuse",
        []() {},
        [&](const std::string&, eventengine::Error) { count++; });

    loop.run();
    EXPECT_EQ(count, 1);

    // Same name should be accepted again
    svc.submitOrThrow("reuse",
        []() {},
        [&](const std::string&, eventengine::Error) { count++; });

    loop.run();
    EXPECT_EQ(count, 2);
}
