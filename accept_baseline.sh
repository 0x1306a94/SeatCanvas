#!/bin/bash

# Accept baseline changes.
# Recommended: run ./update_baseline.sh first (verify develop, then compare current branch).
# This script runs compare tests to generate tests/out/, then accepts version.json.

set -e

cd "$(dirname "$0")"

PROJECT_DIR=$(pwd)
BUILD_DIR=${PROJECT_DIR}/build_test
TEST_BIN="${BUILD_DIR}/tests/SeatCanvasFullTests"

if [ "${1}" = "clean" ]; then
    rm -rf "${BUILD_DIR}"
fi

echo "Step 1: Building SeatCanvasFullTests..."
mkdir -p "${BUILD_DIR}"
if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" \
        -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
        -DPLATFORM=MAC_ARM64 \
        -DSEATCANVAS_BUILD_TESTS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
fi
cmake --build "${BUILD_DIR}" --target SeatCanvasFullTests -j"$(sysctl -n hw.logicalcpu)"

echo "Step 2: Running SeatCanvasFullTests..."
set +e
"${TEST_BIN}"
COMPARE_EXIT=$?
set -e

if [ ! -f "tests/out/version.json" ]; then
    echo "Error: tests/out/version.json not found after running SeatCanvasFullTests."
    exit 1
fi

if [ "${COMPARE_EXIT}" -ne 0 ]; then
    echo "Note: SeatCanvasFullTests reported failures (exit ${COMPARE_EXIT})."
    echo "Review tests/out/ before accepting."
fi

echo "Step 3: Accepting version.json..."
cp tests/out/version.json tests/baseline/version.json

echo ""
echo "Baseline accepted. Commit:"
echo "  git add tests/baseline/version.json && git commit -m 'Update baseline.'"
