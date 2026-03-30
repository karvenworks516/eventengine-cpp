#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
TEST_DIR="${BUILD_DIR}/tests"
FILTER=""
VERBOSE=false

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --filter PATTERN     GoogleTest filter (e.g. "TimerServiceTest.*")
  --verbose            Show individual test output
  --list               List available tests without running
  -h, --help           Show this help
EOF
    exit 0
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --filter)  FILTER="$2"; shift ;;
        --verbose) VERBOSE=true ;;
        --list)    LIST=true ;;
        -h|--help) usage ;;
        *)         echo "Unknown option: $1"; usage ;;
    esac
    shift
done

if [[ ! -d "$TEST_DIR" ]]; then
    echo "Error: Build directory not found. Run ./build.sh first."
    exit 1
fi

# Find test executables
TEST_BINS=()
for f in "${TEST_DIR}"/test_*; do
    [[ -x "$f" && ! "$f" =~ \. ]] && TEST_BINS+=("$f")
done

if [[ ${#TEST_BINS[@]} -eq 0 ]]; then
    echo "Error: No test executables found in ${TEST_DIR}"
    echo "Run ./build.sh with tests enabled first."
    exit 1
fi

# List mode
if [[ "${LIST:-false}" == true ]]; then
    for bin in "${TEST_BINS[@]}"; do
        echo "--- $(basename "$bin") ---"
        "$bin" --gtest_list_tests
    done
    exit 0
fi

# Run tests
FAILED=0
for bin in "${TEST_BINS[@]}"; do
    echo "==> Running $(basename "$bin")"

    ARGS=""
    [[ -n "$FILTER" ]] && ARGS+="--gtest_filter=${FILTER} "
    [[ "$VERBOSE" == true ]] && ARGS+="--gtest_print_time=1 "

    if "$bin" ${ARGS}; then
        echo "    PASSED"
    else
        echo "    FAILED"
        FAILED=$((FAILED + 1))
    fi
    echo
done

if [[ $FAILED -gt 0 ]]; then
    echo "==> ${FAILED} test suite(s) FAILED"
    exit 1
else
    echo "==> All test suites PASSED"
fi
