#!/usr/bin/env bash
set -euo pipefail

# --- Configuration ---
LIBUV_VERSION="v1.52.1"

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
LIBUV_DIR="${PROJECT_DIR}/third_party/libuv"
BUILD_TYPE="Release"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"

# Defaults
STATIC=ON
SHARED=ON
TESTS=ON
EXAMPLES=ON
CLEAN=false
FORCE=false

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --clean              Remove build directory before building
  --force              Full clean rebuild (removes build dir + resets libuv submodule)
  --debug              Build in Debug mode (default: Release)
  --no-static          Skip static library
  --no-shared          Skip shared library
  --no-tests           Skip unit tests
  --no-examples        Skip example apps
  --tests-only         Build only static lib + tests (no shared, no examples)
  --jobs N             Parallel build jobs (default: ${JOBS})
  -h, --help           Show this help

libuv version: ${LIBUV_VERSION}
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)       CLEAN=true ;;
        --force)       FORCE=true; CLEAN=true ;;
        --debug)       BUILD_TYPE="Debug" ;;
        --no-static)   STATIC=OFF ;;
        --no-shared)   SHARED=OFF ;;
        --no-tests)    TESTS=OFF ;;
        --no-examples) EXAMPLES=OFF ;;
        --tests-only)  SHARED=OFF; EXAMPLES=OFF; TESTS=ON; STATIC=ON ;;
        --jobs)        JOBS="$2"; shift ;;
        -h|--help)     usage ;;
        *)             echo "Unknown option: $1"; usage ;;
    esac
    shift
done

# --- Submodule version check ---
sync_libuv() {
    if [[ ! -d "${LIBUV_DIR}/.git" ]]; then
        echo "==> libuv submodule not initialized, fetching..."
        git -C "${PROJECT_DIR}" submodule update --init --recursive
    fi

    CURRENT_TAG="$(git -C "${LIBUV_DIR}" describe --tags --exact-match 2>/dev/null || echo "")"

    if [[ "${CURRENT_TAG}" == "${LIBUV_VERSION}" ]]; then
        echo "==> libuv already at ${LIBUV_VERSION}, skipping"
    else
        echo "==> libuv version mismatch (current: ${CURRENT_TAG:-unknown}, want: ${LIBUV_VERSION})"
        echo "    Cleaning and re-syncing..."
        git -C "${LIBUV_DIR}" clean -fdx --quiet
        git -C "${LIBUV_DIR}" checkout . --quiet
        git -C "${LIBUV_DIR}" fetch --tags --quiet
        git -C "${LIBUV_DIR}" checkout "${LIBUV_VERSION}" --quiet
        echo "    libuv pinned to ${LIBUV_VERSION}"
        # Force cmake reconfigure since libuv source changed
        CLEAN=true
    fi
}

# --- Force reset ---
if [[ "$FORCE" == true ]]; then
    echo "==> Force rebuild: resetting libuv submodule"
    rm -rf "${LIBUV_DIR}"
    git -C "${PROJECT_DIR}" submodule update --init --recursive
fi

sync_libuv

# --- Build ---
if [[ "$CLEAN" == true ]]; then
    echo "==> Cleaning build directory"
    rm -rf "${BUILD_DIR}"
fi

echo "==> Configuring (${BUILD_TYPE})"
echo "    Static: ${STATIC}  Shared: ${SHARED}  Tests: ${TESTS}  Examples: ${EXAMPLES}"

cmake -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DEVENTENGINE_BUILD_STATIC="${STATIC}" \
    -DEVENTENGINE_BUILD_SHARED="${SHARED}" \
    -DEVENTENGINE_BUILD_TESTS="${TESTS}" \
    -DEVENTENGINE_BUILD_EXAMPLES="${EXAMPLES}" \
    "${PROJECT_DIR}"

echo "==> Building (jobs: ${JOBS})"
cmake --build "${BUILD_DIR}" -j "${JOBS}"

echo "==> Build complete"
echo "    Artifacts in: ${BUILD_DIR}"
