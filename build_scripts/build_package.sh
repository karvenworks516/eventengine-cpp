#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
SCRIPTS_DIR="${PROJECT_DIR}/build_scripts"
VERSION="$(grep 'project(' "${PROJECT_DIR}/CMakeLists.txt" | sed 's/.*VERSION \([^ ]*\).*/\1/')"
PLATFORM="$(uname -s | tr '[:upper:]' '[:lower:]')-$(uname -m)"
PACKAGE_NAME="eventengine-cpp-${VERSION}-${PLATFORM}"
PACKAGE_DIR="${BUILD_DIR}/package/${PACKAGE_NAME}"

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Creates a distributable package containing:
  - Static and shared libraries
  - Public header files
  - API documentation (Doxygen)
  - Example source files
  - Example binaries

Output: build/package/${PACKAGE_NAME}.tar.gz

Options:
  --no-docs            Skip Doxygen documentation
  --no-examples        Skip example binaries
  -h, --help           Show this help
EOF
    exit 0
}

BUILD_DOCS=true
BUILD_EXAMPLES=true

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-docs)     BUILD_DOCS=false ;;
        --no-examples) BUILD_EXAMPLES=false ;;
        -h|--help)     usage ;;
        *)             echo "Unknown option: $1"; usage ;;
    esac
    shift
done

echo "==> Building eventengine-cpp ${VERSION} package for ${PLATFORM}"

# --- Step 1: Build libraries and examples ---
echo ""
echo "==> Step 1: Building libraries"
EXAMPLES_FLAG="ON"
if [[ "$BUILD_EXAMPLES" == false ]]; then
    EXAMPLES_FLAG="OFF"
fi

bash "${SCRIPTS_DIR}/build.sh" \
    --clean \
    --no-tests \
    $(if [[ "$BUILD_EXAMPLES" == false ]]; then echo "--no-examples"; fi)

# --- Step 2: Generate API docs ---
if [[ "$BUILD_DOCS" == true ]]; then
    echo ""
    echo "==> Step 2: Generating API documentation"
    if command -v doxygen &>/dev/null; then
        bash "${SCRIPTS_DIR}/build_docs.sh"
    else
        echo "    WARNING: doxygen not found, skipping API docs"
        BUILD_DOCS=false
    fi
else
    echo ""
    echo "==> Step 2: Skipping API documentation"
fi

# --- Step 3: Assemble package ---
echo ""
echo "==> Step 3: Assembling package"
rm -rf "${PACKAGE_DIR}"
mkdir -p "${PACKAGE_DIR}"

# Headers
echo "    Copying headers..."
mkdir -p "${PACKAGE_DIR}/include/eventengine"
cp "${PROJECT_DIR}"/include/eventengine/*.hpp "${PACKAGE_DIR}/include/eventengine/"
cp "${BUILD_DIR}/include/eventengine/version.hpp" "${PACKAGE_DIR}/include/eventengine/"

# Libraries
echo "    Copying libraries..."
mkdir -p "${PACKAGE_DIR}/lib"
for f in "${BUILD_DIR}"/src/libeventengine-cpp.*; do
    [[ -f "$f" ]] && cp "$f" "${PACKAGE_DIR}/lib/"
done

# API docs
if [[ "$BUILD_DOCS" == true && -d "${PROJECT_DIR}/docs/api/html" ]]; then
    echo "    Copying API documentation..."
    mkdir -p "${PACKAGE_DIR}/docs"
    cp -r "${PROJECT_DIR}/docs/api/html" "${PACKAGE_DIR}/docs/api"
fi

# Examples (source + binaries)
echo "    Copying examples..."
mkdir -p "${PACKAGE_DIR}/examples/src"
cp "${PROJECT_DIR}"/examples/*.cpp "${PACKAGE_DIR}/examples/src/"
if [[ "$BUILD_EXAMPLES" == true ]]; then
    mkdir -p "${PACKAGE_DIR}/examples/bin"
    for f in "${BUILD_DIR}"/examples/test_*; do
        [[ -x "$f" && ! -d "$f" ]] && cp "$f" "${PACKAGE_DIR}/examples/bin/"
    done
fi

# LICENSE and README
[[ -f "${PROJECT_DIR}/LICENSE" ]] && cp "${PROJECT_DIR}/LICENSE" "${PACKAGE_DIR}/"
cp "${PROJECT_DIR}/README.md" "${PACKAGE_DIR}/"

# --- Step 4: Create archive ---
echo ""
echo "==> Step 4: Creating archive"
ARCHIVE="${BUILD_DIR}/package/${PACKAGE_NAME}.tar.gz"
tar -czf "${ARCHIVE}" -C "${BUILD_DIR}/package" "${PACKAGE_NAME}"

echo ""
echo "==> Package complete"
echo "    ${ARCHIVE}"
echo ""
echo "    Contents:"
echo "    ├── include/eventengine/   ($(ls "${PACKAGE_DIR}"/include/eventengine/ | wc -l | tr -d ' ') headers)"
echo "    ├── lib/                   ($(ls "${PACKAGE_DIR}"/lib/ | wc -l | tr -d ' ') libraries)"
if [[ "$BUILD_DOCS" == true && -d "${PACKAGE_DIR}/docs" ]]; then
echo "    ├── docs/api/              (Doxygen HTML)"
fi
if [[ "$BUILD_EXAMPLES" == true && -d "${PACKAGE_DIR}/examples/bin" ]]; then
echo "    ├── examples/src/          ($(ls "${PACKAGE_DIR}"/examples/src/ | wc -l | tr -d ' ') source files)"
echo "    ├── examples/bin/          ($(ls "${PACKAGE_DIR}"/examples/bin/ | wc -l | tr -d ' ') binaries)"
else
echo "    ├── examples/src/          ($(ls "${PACKAGE_DIR}"/examples/src/ | wc -l | tr -d ' ') source files)"
fi
echo "    └── README.md"
