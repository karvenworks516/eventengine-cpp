# EventEngine — File Map

## Directory Structure Convention

Every service follows this layout:

```
src/<service>/
├── core/           ← low-level libuv wrapper (.cpp)
└── service/        ← high-level service layer (.cpp)

include/eventengine/      ← all public headers (flat, as distributed)
```

Headers stay flat in `include/eventengine/` for simple distribution (`include/eventengine/` + `lib/`).

---

## Namespace Mapping

| Namespace | Types |
|-----------|-------|
| `eventengine` | Error, Exception, Loop, Handle, Service, ServiceConfig |
| `eventengine::timer` | Timer, TimerService, Duration, RepeatPolicy, TimerStatus, TimerEvent, TimerInfo |
| `eventengine::threadpool` | Work, ThreadPoolService, TaskStatus, TaskInfo |

---

## Current Files

### Public Headers — `include/eventengine/`

| File | Namespace | Description |
|------|-----------|-------------|
| `export.hpp` | — | EVENTENGINE_API symbol visibility macros |
| `version.hpp.in` | — | Version macros (CMake configured) |
| `fwd.hpp` | — | Forward declarations for all namespaces |
| `error.hpp` | `eventengine` | Error & Exception classes |
| `loop.hpp` | `eventengine` | Event loop wrapper |
| `handle.hpp` | `eventengine` | CRTP Handle base template |
| `duration.hpp` | `eventengine::timer` | Duration value type (ms/s/m/h/d) |
| `repeat_policy.hpp` | `eventengine::timer` | RepeatPolicy (none/forever/times) |
| `timer_types.hpp` | `eventengine::timer` | TimerStatus, TimerEvent, TimerInfo |
| `timer_service.hpp` | `eventengine::timer` | High-level TimerService API |
| `thread_pool_types.hpp` | `eventengine::threadpool` | TaskStatus, TaskInfo |
| `thread_pool_service.hpp` | `eventengine::threadpool` | High-level ThreadPoolService API |
| `service.hpp` | `eventengine` | Service enum |
| `service_config.hpp` | `eventengine` | ServiceConfig builder |
| `eventengine.hpp` | — | Convenience include-all header |
| `detail/handle.hpp` | `eventengine` | CRTP Handle base (internal) |
| `detail/timer.hpp` | `eventengine::timer` | Low-level Timer handle (internal) |
| `detail/work.hpp` | `eventengine::threadpool` | Low-level Work request (internal) |

### Source Files — `src/`

| File | Category |
|------|----------|
| `src/core/error.cpp` | Core |
| `src/core/loop.cpp` | Core |
| `src/timer/core/timer.cpp` | Timer — low-level wrapper |
| `src/timer/service/timer_service.cpp` | Timer — TimerService |
| `src/threadpool/core/thread_pool_work.cpp` | ThreadPool — low-level Work request |
| `src/threadpool/service/thread_pool_service.cpp` | ThreadPool — ThreadPoolService |

### Tests — `tests/`

| File | Covers |
|------|--------|
| `tests/test_timer_service.cpp` | Duration, RepeatPolicy, TimerService (35 tests) |
| `tests/test_thread_pool_service.cpp` | TaskStatus, ThreadPoolService (16 tests) |

### Examples — `examples/`

| File | Description |
|------|-------------|
| `examples/test_version.cpp` | Prints EventEngine and libuv versions |
| `examples/test_timer.cpp` | Exercises all TimerService features (9 scenarios) |
| `examples/test_thread_pool.cpp` | Exercises all ThreadPoolService features (8 scenarios) |

### Build

| File | Description |
|------|-------------|
| `CMakeLists.txt` | Root project config |
| `src/CMakeLists.txt` | Library targets (static + shared) |
| `tests/CMakeLists.txt` | Test executables |
| `examples/CMakeLists.txt` | Example executables |
| `cmake/eventengine-cppConfig.cmake.in` | Package config for consumers |
| `Doxyfile` | Doxygen configuration |

### Build Scripts — `build_scripts/`

| File | Description |
|------|-------------|
| `build.sh` | macOS/Linux build script |
| `build.ps1` | Windows PowerShell build script |
| `run_tests.sh` | macOS/Linux test runner |
| `run_tests.ps1` | Windows test runner |
| `build_docs.sh` | Doxygen documentation generator |

### Docs — `docs/`

| File | Description |
|------|-------------|
| `DESIGN.md` | Library architecture & design decisions |
| `IMPLEMENTATION.md` | Phase 1 implementation spec |
| `FILE_MAP.md` | This file |
| `TASKS.md` | Task tracker |
| `services/TIMER_SERVICE.md` | TimerService feature spec |
| `services/THREAD_POOL_SERVICE.md` | ThreadPoolService feature spec |

---

## Adding a New Service

Follow this pattern:

```
include/eventengine/
    <service>.hpp               ← low-level wrapper header
    <service>_types.hpp         ← types/enums if needed
    <service>_service.hpp       ← service header

src/<service>/
    core/<service>.cpp          ← low-level wrapper
    service/<service>_service.cpp  ← service implementation

tests/
    test_<service>_service.cpp

examples/
    test_<service>.cpp

docs/services/
    <SERVICE>_SERVICE.md
```
