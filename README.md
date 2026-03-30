# eventengine-cpp

A cross-platform C++17 event-driven library built on [libuv](https://libuv.org/), providing type-safe RAII wrappers for asynchronous I/O primitives.

## Features

- **Event Loop** — RAII-managed event loop with Default, Once, and NoWait run modes
- **Timers** — one-shot and repeating timers with pause/resume support
- **Timer Service** — managed timer creation, cancellation, rescheduling, and querying with configurable repeat policies (finite count or infinite) and event listeners
- **Thread Pool Service** — named task submission, cancellation, timeout, and status querying with stats tracking
- **Dual error model** — every fallible operation offers both an error-code variant and an exception-throwing variant
- **RAII handle lifecycle** — all handles are automatically cleaned up on destruction; move semantics are fully supported
- **Cross-platform** — builds and runs on macOS, Linux, and Windows
- **Signal handling** *(planned)*

## Namespace Structure

| Namespace | Contents |
|-----------|----------|
| `eventengine` | Core types — Error, Exception, Loop, Handle |
| `eventengine::timer` | Timer, TimerService, Duration, RepeatPolicy |
| `eventengine::threadpool` | Work, ThreadPoolService, TaskStatus, TaskInfo |

## Platform Dependencies

### All Platforms

- C++17 compiler
- CMake 3.16+
- Git (for the libuv submodule)

### macOS

- Xcode Command Line Tools or a standalone Clang 5+ / GCC 7+ installation

### Linux

- GCC 7+ or Clang 5+
- Standard build essentials (make or ninja)

### Windows

- MSVC 2017+ (Visual Studio or Build Tools)
- PowerShell 5.1+ (for the build script)

libuv v1.52.1 is bundled as a Git submodule under `third_party/libuv` and requires no separate installation.

## Building

### macOS / Linux

Run the provided build script from the project root:

    ./build_scripts/build.sh

### Windows

Run the PowerShell build script from the project root:

    .\build_scripts\build.ps1

### Manual CMake Build

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `EVENTENGINE_BUILD_SHARED` | `ON` | Build shared library |
| `EVENTENGINE_BUILD_STATIC` | `ON` | Build static library |
| `EVENTENGINE_BUILD_TESTS` | `ON` | Build unit tests |
| `EVENTENGINE_BUILD_EXAMPLES` | `ON` | Build examples |

Both build scripts support additional flags such as `--debug`, `--clean`, `--no-tests`, `--no-shared`, and `--tests-only`. Run the script with `--help` for the full list.

### Running Tests

    ./build_scripts/run_tests.sh              # macOS / Linux
    .\build_scripts\run_tests.ps1             # Windows

Use `--filter "TimerServiceTest.*"` to run a subset or `--list` to see available tests.

## API Documentation

API documentation is generated from source using [Doxygen](https://www.doxygen.nl/). To build it locally:

    ./build_scripts/build_docs.sh

Or via CMake (requires Doxygen to be installed):

    cmake --build build --target docs

The generated documentation will be available at `docs/api/html/index.html`.

Doxygen can be installed with:

- **macOS** — `brew install doxygen`
- **Linux** — `sudo apt-get install doxygen` (or equivalent)
- **Windows** — `choco install doxygen`

## Design Documentation

| Document | Description |
|----------|-------------|
| [docs/DESIGN.md](docs/DESIGN.md) | Library architecture, ownership model, error handling |
| [docs/IMPLEMENTATION.md](docs/IMPLEMENTATION.md) | Phase 1 implementation spec |
| [docs/services/TIMER_SERVICE.md](docs/services/TIMER_SERVICE.md) | TimerService feature spec |
| [docs/services/THREAD_POOL_SERVICE.md](docs/services/THREAD_POOL_SERVICE.md) | ThreadPoolService feature spec |
| [docs/FILE_MAP.md](docs/FILE_MAP.md) | Complete file and namespace mapping |
| [docs/TASKS.md](docs/TASKS.md) | Task tracker |

## License

See [LICENSE](LICENSE) for details.
