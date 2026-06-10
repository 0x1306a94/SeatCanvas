#!/bin/bash

# Accept baseline changes.
# Runs compare tests to generate tests/out/, then accepts version.json and refreshes cache.

set -e

cd "$(dirname "$0")"

PROJECT_DIR=$(pwd)
BUILD_DIR="build_test"
TEST_BIN="${BUILD_DIR}/tests/SeatCanvasFullTests"

echo "Step 1: Building SeatCanvasFullTests..."
cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
    -DPLATFORM=MAC_ARM64 \
    -DSEATCANVAS_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
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

echo "Step 4: Running SeatCanvasFullTests in update baseline mode..."
SEATCANVAS_UPDATE_BASELINE=1 "${TEST_BIN}"

echo ""
echo "Baseline accepted. Commit:"
echo "  git add tests/baseline/version.json && git commit -m 'Update baseline.'"
