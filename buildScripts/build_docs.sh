#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

if ! command -v doxygen &>/dev/null; then
    echo "Error: doxygen is not installed."
    echo "  macOS:   brew install doxygen"
    echo "  Linux:   sudo apt-get install doxygen  (or equivalent)"
    echo "  Windows: choco install doxygen"
    exit 1
fi

mkdir -p "${PROJECT_DIR}/docs/api"

echo "==> Generating API documentation"
cd "${PROJECT_DIR}"
doxygen Doxyfile

echo "==> Done. Open docs/api/html/index.html in your browser."
