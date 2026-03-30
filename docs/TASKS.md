# EventEngine — Task Tracker

---

## Epic: Core Library

### Requirements
- RAII wrappers for libuv primitives (loop, handles, requests)
- Dual error model (error codes + exceptions) on every fallible operation
- Non-copyable, movable handles with automatic cleanup on destruction
- Cross-platform: macOS, Linux, Windows
- C++17, no external dependencies beyond bundled libuv

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | Error/Exception classes wrapping libuv error codes | ✅ Done |
| F02 | Event loop with Default/Once/NoWait run modes | ✅ Done |
| F03 | CRTP Handle base with RAII close-on-destruct | ✅ Done |
| F04 | Timer handle (one-shot + repeating) | ✅ Done |
| F05 | Thread pool Work request (fire-and-forget with cancel) | ✅ Done |
| F06 | Async handle (cross-thread signaling) | 🔲 Planned |
| F07 | Signal handle | 🔲 Planned |
| F08 | Idle handle | 🔲 Planned |
| F09 | Prepare handle | 🔲 Planned |
| F10 | Check handle | 🔲 Planned |
| F11 | Convenience include-all header | 🔲 Planned |

### Tasks
- [x] T01: `include/eventengine/error.hpp` + `src/core/error.cpp`
- [x] T02: `include/eventengine/loop.hpp` + `src/core/loop.cpp`
- [x] T03: `include/eventengine/handle.hpp` (header-only)
- [x] T04: `include/eventengine/timer.hpp` + `src/timer/core/timer.cpp`
- [x] T05: `include/eventengine/work.hpp` + `src/threadpool/core/thread_pool_work.cpp`
- [ ] T06: `include/eventengine/async.hpp` + `src/core/async.cpp`
- [ ] T07: `include/eventengine/signal.hpp` + `src/core/signal.cpp`
- [ ] T08: `include/eventengine/idle.hpp` + `src/core/idle.cpp`
- [ ] T09: `include/eventengine/prepare.hpp` + `src/core/prepare.cpp`
- [ ] T10: `include/eventengine/check.hpp` + `src/core/check.cpp`
- [ ] T11: `include/eventengine/eventengine.hpp` — convenience include-all
- [ ] T12: `tests/test_error.cpp`
- [ ] T13: `tests/test_loop.cpp`
- [ ] T14: `tests/test_timer.cpp`
- [ ] T15: `tests/test_work.cpp`
- [ ] T16: `tests/test_async.cpp`
- [ ] T17: `tests/test_signal.cpp`
- [ ] T18: `tests/test_idle.cpp`
- [ ] T19: `tests/test_prepare.cpp`
- [ ] T20: `tests/test_check.cpp`

---

## Epic: Build System & Configuration

### Requirements
- CMake 3.16+ with static and shared library targets
- Bundled libuv as git submodule (no external install)
- GoogleTest fetched automatically for tests
- Cross-platform build scripts (bash + PowerShell)
- Symbol export macros for shared library builds
- Version info generated from CMake

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | Root CMakeLists with build options | ✅ Done |
| F02 | Static + shared library targets | ✅ Done |
| F03 | Test infrastructure with GoogleTest | ✅ Done |
| F04 | Package config for consumers | ✅ Done |
| F05 | Export macros (dllexport/visibility) | ✅ Done |
| F06 | Version header from CMake | ✅ Done |
| F07 | Forward declarations header | ✅ Done |
| F08 | Doxygen API doc generation | ✅ Done |

### Tasks
- [x] T21: Root `CMakeLists.txt`
- [x] T22: `src/CMakeLists.txt`
- [x] T23: `tests/CMakeLists.txt`
- [x] T24: `cmake/eventengine-cppConfig.cmake.in`
- [x] T25: `include/eventengine/export.hpp`
- [x] T26: `include/eventengine/version.hpp.in`
- [x] T27: `include/eventengine/fwd.hpp`
- [x] T28: `Doxyfile` + `build_scripts/build_docs.sh`

---

## Epic: Design Documents

### Requirements
- Document architecture decisions before implementation
- Keep docs in sync with code changes

### Tasks
- [x] D01: Library design — class hierarchy, ownership model, lifetime management
- [x] D02: Error handling — Error/Exception contract, error propagation
- [x] D03: Handle lifecycle — state machine, RAII guarantees
- [x] D04: Thread safety — per-class guarantees, callback thread rules
- [x] D05: API conventions — naming, callbacks, OrThrow pattern
- [x] D06: Service layer design — Engine, ServiceConfig, services

---

## Epic: Timer Service

### Requirements
- High-level managed timer API on top of low-level Timer handle
- Named timers with unique string IDs
- One-shot, finite repeat, and infinite repeat policies
- Pause/resume preserving remaining time
- Configurable max timer limit
- Event listener for lifecycle notifications
- Namespace: `eventengine::timer`

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | Named timer creation with string IDs | ✅ Done |
| F02 | Duration value type (ms/s/m/h/d) | ✅ Done |
| F03 | RepeatPolicy (none/forever/times) | ✅ Done |
| F04 | One-shot timers with auto-cleanup | ✅ Done |
| F05 | Repeating timers with count limit | ✅ Done |
| F06 | Cancel timer / cancel all | ✅ Done |
| F07 | Pause / resume (preserves remaining time) | ✅ Done |
| F08 | Bulk pause / resume all | ✅ Done |
| F09 | Reschedule with new timeout/policy | ✅ Done |
| F10 | Status / info / remaining time queries | ✅ Done |
| F11 | List all timer IDs | ✅ Done |
| F12 | Max timer limit (configurable) | ✅ Done |
| F13 | Timer label for debugging | ✅ Done |
| F14 | TimerInfo passed to callbacks | ✅ Done |
| F15 | Event listener (created/fired/cancelled/etc.) | ✅ Done |
| F16 | Dual error model (Error + OrThrow) | ✅ Done |
| F17 | Thread-safe operations via Async | 🔲 Future |

### Tasks
- [x] S01: Duration class
- [x] S02: RepeatPolicy class
- [x] S03: TimerStatus enum, TimerEvent enum, TimerInfo struct
- [x] S04: TimerService — create, cancel, cancelAll
- [x] S05: TimerService — pause/resume, pauseAll/resumeAll
- [x] S06: TimerService — repeat count limit
- [x] S07: TimerService — status/info/remainingTime queries
- [x] S08: TimerService — allTimerIds, activeCount, maxTimers
- [x] S09: TimerService — event listener
- [x] S10: TimerService — reschedule
- [x] S11: Unit tests (`tests/test_timer_service.cpp` — 35 tests)
- [x] S12: Example app (`examples/test_timer.cpp`)
- [x] S13: Design doc (`docs/services/TIMER_SERVICE.md`)

---

## Epic: Thread Pool Service

### Requirements
- High-level managed thread pool API on top of low-level Work request
- Named tasks with unique string names
- Task cancellation for queued (not yet started) tasks via `uv_cancel`
- Task timeout via internal Timer that auto-cancels
- 5 lifecycle states: Queued, InProgress, Finished, Cancelled, Rejected
- Atomic stats counters safe to read from any thread
- Namespace: `eventengine::threadpool`

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | Named task submission | ✅ Done |
| F02 | After-work callback with task name + Error | ✅ Done |
| F03 | Task cancellation by name | ✅ Done |
| F04 | Task timeout via Timer | ✅ Done |
| F05 | Task status query by name | ✅ Done |
| F06 | Task info query (name, status, timeout) | ✅ Done |
| F07 | List all task names | ✅ Done |
| F08 | Active count (atomic) | ✅ Done |
| F09 | Completed count (atomic) | ✅ Done |
| F10 | Duplicate/empty name rejection | ✅ Done |
| F11 | Failure rollback on queue error | ✅ Done |
| F12 | Dual error model (Error + OrThrow) | ✅ Done |
| F13 | Configurable pool size via Engine | 🔲 Future |
| F14 | Priority queues | 🔲 Future |
| F15 | Dynamic pool resize | 🔲 Future |

### Tasks
- [x] TP01: Work class with queue + cancel
- [x] TP02: TaskStatus enum, TaskInfo struct
- [x] TP03: ThreadPoolService — submit, submitOrThrow
- [x] TP04: ThreadPoolService — cancel, cancelOrThrow
- [x] TP05: ThreadPoolService — timeout via Timer
- [x] TP06: ThreadPoolService — status/info/allTaskNames queries
- [x] TP07: Unit tests (`tests/test_thread_pool_service.cpp` — 16 tests)
- [x] TP08: Example app (`examples/test_thread_pool.cpp`)
- [x] TP09: Design doc (`docs/services/THREAD_POOL_SERVICE.md`)

---

## Epic: Engine & Service Configuration

### Requirements
- Central entry point that owns the Loop and all enabled services
- Declarative configuration: app builds a config, passes it to Engine::init()
- Per-service configuration parameters (e.g. max timers, thread pool size)
- Engine lifecycle: init(config) → start() → stop() → destroy
- Service accessors that throw if service not enabled
- Clean shutdown: stop loop, destroy services in reverse order
- App should never interact with Loop directly — Engine owns it

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | EngineConfig — master config holding per-service configs | 🔲 Planned |
| F02 | TimerServiceConfig — maxTimers | 🔲 Planned |
| F03 | ThreadPoolServiceConfig — poolSize (UV_THREADPOOL_SIZE) | 🔲 Planned |
| F04 | Engine::init(config) — create Loop, instantiate enabled services | 🔲 Planned |
| F05 | Engine::start() — run the loop (blocking) | 🔲 Planned |
| F06 | Engine::stop() — stop the loop (thread-safe) | 🔲 Planned |
| F07 | Engine::isRunning() | 🔲 Planned |
| F08 | Service accessors — timerService(), threadPoolService() | 🔲 Planned |
| F09 | Accessor throws if service not enabled in config | 🔲 Planned |
| F10 | Graceful shutdown — services destroyed before Loop | 🔲 Planned |

### Design Notes

**App lifecycle:**
```
1. App creates EngineConfig
2. App enables services and sets per-service parameters
3. App calls Engine::init(config)  → Loop created, services instantiated
4. App uses service accessors to interact with services
5. App calls Engine::start()       → loop.run() blocks
6. App calls Engine::stop()        → loop.stop() from any thread
7. Engine destructor               → services destroyed, loop closed
```

**Proposed config API:**
```
EngineConfig config;
config.enableTimer(TimerServiceConfig{}.setMaxTimers(512));
config.enableThreadPool(ThreadPoolServiceConfig{}.setPoolSize(8));

Engine engine;
engine.initOrThrow(config);

auto& timers = engine.timerService();     // OK
auto& pool   = engine.threadPoolService(); // OK
// auto& fs  = engine.fsService();         // throws — not enabled

engine.start(); // blocks
```

**Per-service config structs:**

| Config struct | Parameters | Defaults |
|--------------|------------|----------|
| `TimerServiceConfig` | maxTimers | 1024 |
| `ThreadPoolServiceConfig` | poolSize | 4 (libuv default) |
| `FsServiceConfig` | *(future)* | — |
| `FsWatchServiceConfig` | *(future)* | — |

**Error codes (range -300 to -399):**

| Code | Meaning |
|------|---------|
| `EE_ENGINE_NOT_INITIALIZED` | Engine::start() called before init() |
| `EE_ENGINE_ALREADY_RUNNING` | Engine::start() called while already running |
| `EE_ENGINE_SERVICE_DISABLED` | Accessor called for a service not enabled in config |

### Tasks
- [ ] E01: Design doc (`docs/services/ENGINE.md`)
- [ ] E02: `include/eventengine/engine_config.hpp` — EngineConfig, TimerServiceConfig, ThreadPoolServiceConfig
- [ ] E03: `include/eventengine/engine.hpp` + `src/core/engine.cpp` — Engine class
- [ ] E04: Engine::init(config) — create Loop, instantiate services per config
- [ ] E05: Engine::start() / stop() / isRunning()
- [ ] E06: Service accessors with disabled-service error
- [ ] E07: Engine error codes in `error_code.hpp`
- [ ] E08: Update existing ServiceConfig/Service to align or deprecate
- [ ] E09: Unit tests (`tests/test_engine.cpp`)
- [ ] E10: Example app (`examples/test_engine.cpp`)
- [ ] E11: Update README with engine usage

---

## Epic: Cron Support

### Requirements
- Parse standard 5-field cron expressions (minute, hour, day-of-month, month, day-of-week)
- Compute next fire time from current time
- Integrate with TimerService — auto-rescheduling cron-based timers
- No external dependency — self-contained parser
- Namespace: `eventengine::timer`

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | Cron expression parser (*, N, N-M, N/S, N,M,O) | 🔲 Planned |
| F02 | Validate expressions, report parse errors | 🔲 Planned |
| F03 | Compute next occurrence from a time point | 🔲 Planned |
| F04 | setCronTimer on TimerService — auto-reschedule after each fire | 🔲 Planned |
| F05 | Cron timers support pause/resume/cancel via existing APIs | 🔲 Planned |

### Tasks
- [ ] CR01: Design doc
- [ ] CR02: `include/eventengine/cron.hpp` — CronExpression class
- [ ] CR03: `include/eventengine/cron_types.hpp` — CronField enum, parse result types
- [ ] CR04: CronExpression::next() — compute next fire time
- [ ] CR05: Integrate with TimerService — `setCronTimer()`
- [ ] CR06: Cron timer pause/resume/cancel
- [ ] CR07: Unit tests for cron parser
- [ ] CR08: Unit tests for cron-based timers
- [ ] CR09: Example app

---

## Epic: File System

### Requirements
- Async file system operations wrapping `uv_fs_*` request types
- Fire-and-forget requests, self-cleaning via `uv_fs_req_cleanup`
- Cover core operations: open, read, write, close, stat, unlink, rename, mkdir, rmdir, scandir, copy, chmod
- High-level FsService with named operations and stats
- Dual error model
- Namespace: `eventengine::fs`

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | Async file open/close | 🔲 Planned |
| F02 | Async read/write with offset | 🔲 Planned |
| F03 | Async stat/fstat | 🔲 Planned |
| F04 | Async directory ops (mkdir, rmdir, scandir) | 🔲 Planned |
| F05 | Async file ops (unlink, rename, copyfile, chmod) | 🔲 Planned |
| F06 | Async temp file/dir creation (mkdtemp, mkstemp) | 🔲 Planned |
| F07 | FileInfo struct, OpenFlags enum | 🔲 Planned |
| F08 | FsService — high-level named operations with stats | 🔲 Planned |

### Tasks
- [ ] FS01: Design doc (`docs/services/FS_SERVICE.md`)
- [ ] FS02: `include/eventengine/fs_types.hpp` — FileInfo, OpenFlags
- [ ] FS03: `include/eventengine/fs.hpp` — low-level Fs class
- [ ] FS04: Implement open/close
- [ ] FS05: Implement read/write
- [ ] FS06: Implement stat/fstat
- [ ] FS07: Implement directory ops (mkdir, rmdir, scandir)
- [ ] FS08: Implement file ops (unlink, rename, copyfile, chmod)
- [ ] FS09: Implement mkdtemp/mkstemp
- [ ] FS10: `include/eventengine/fs_service.hpp` — FsService
- [ ] FS11: Unit tests for low-level Fs
- [ ] FS12: Unit tests for FsService
- [ ] FS13: Example app

---

## Epic: File System Events

### Requirements
- Watch files/directories for changes using OS-native mechanisms
- Provide stat-based polling fallback for portability
- High-level FsWatchService managing multiple named watches
- Namespace: `eventengine::fs`

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | FsEvent — OS-native file watching (inotify/kqueue/ReadDirectoryChanges) | 🔲 Planned |
| F02 | FsPoll — stat-based polling fallback | 🔲 Planned |
| F03 | FsEventType enum (Renamed, Changed) | 🔲 Planned |
| F04 | FsWatchService — named watches with add/remove/list | 🔲 Planned |

### Tasks
- [ ] FE01: Design doc (`docs/services/FS_WATCH_SERVICE.md`)
- [ ] FE02: `include/eventengine/fs_event_types.hpp` — FsEventType, FsEventInfo
- [ ] FE03: `include/eventengine/fs_event.hpp` — FsEvent handle
- [ ] FE04: `include/eventengine/fs_poll.hpp` — FsPoll handle
- [ ] FE05: FsEvent start/stop
- [ ] FE06: FsPoll start/stop
- [ ] FE07: `include/eventengine/fs_watch_service.hpp` — FsWatchService
- [ ] FE08: Unit tests for FsEvent and FsPoll
- [ ] FE09: Unit tests for FsWatchService
- [ ] FE10: Example app

---

## Epic: Logging

### Requirements
- Callback-based logging — no external framework dependency
- Consumer provides a LogCallback; library calls it with level + message
- Zero cost when no logger is set (no formatting, no allocations)
- Thread-safe: callback may be invoked from pool threads and the loop thread

### Features
| # | Feature | Status |
|---|---------|--------|
| F01 | LogLevel enum (Trace, Debug, Info, Warn, Error) | 🔲 Planned |
| F02 | Global setLogger / setLogLevel | 🔲 Planned |
| F03 | No-op default (zero overhead) | 🔲 Planned |
| F04 | Log points in core (Loop, Handle, Error) | 🔲 Planned |
| F05 | Log points in TimerService | 🔲 Planned |
| F06 | Log points in ThreadPoolService | 🔲 Planned |

### Tasks
- [ ] L01: Design doc
- [ ] L02: `include/eventengine/log.hpp` — LogLevel, LogCallback, setLogger, setLogLevel
- [ ] L03: Integrate into core (Loop, Handle, Error)
- [ ] L04: Integrate into TimerService
- [ ] L05: Integrate into ThreadPoolService
- [ ] L06: Default no-op logger
- [ ] L07: Unit tests

---

## Epic: Validation

### Tasks
- [ ] V01: Build static library (clang)
- [ ] V02: Build shared library (clang)
- [ ] V03: Run all unit tests — all pass
- [ ] V04: Update docs with any design changes during coding

---

## Summary

| Epic | Total | Done | Remaining |
|------|-------|------|-----------|
| Core Library | 20 | 5 | 15 |
| Build System & Configuration | 8 | 8 | 0 |
| Design Documents | 6 | 6 | 0 |
| Timer Service | 13 | 13 | 0 |
| Thread Pool Service | 9 | 9 | 0 |
| Engine & Service Configuration | 11 | 0 | 11 |
| Cron Support | 9 | 0 | 9 |
| File System | 13 | 0 | 13 |
| File System Events | 10 | 0 | 10 |
| Logging | 7 | 0 | 7 |
| Validation | 4 | 0 | 4 |
| **Total** | **110** | **41** | **69** |
