#!/bin/bash

set -e

cd $(dirname $0)

PROJECT_DIR=$(pwd)
BUILD_DIR=${PROJECT_DIR}/build_test

ENABLE_COVERAGE=OFF
if [ "${1}" = "coverage" ]; then
    ENABLE_COVERAGE=ON
elif [ "${1}" = "clean" ]; then
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"

if [ "${ENABLE_COVERAGE}" = "ON" ]; then
    # Always reconfigure for coverage to ensure flags are applied
    rm -rf "${BUILD_DIR}/CMakeCache.txt"
fi

if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    cmake -S ${PROJECT_DIR} -B ${BUILD_DIR} \
        -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
        -DPLATFORM=MAC_ARM64 \
        -DSEATCANVAS_BUILD_TESTS=ON \
        -DSEATCANVAS_ENABLE_COVERAGE=${ENABLE_COVERAGE} \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
fi

cmake --build ${BUILD_DIR} --target SeatCanvasFullTests -j$(sysctl -n hw.logicalcpu)

if [ "${ENABLE_COVERAGE}" = "ON" ]; then
    PROFRAW_DIR=${BUILD_DIR}/coverage
    rm -rf ${PROFRAW_DIR}
    mkdir -p ${PROFRAW_DIR}
    FRAMEWORK_DYLIB=${BUILD_DIR}/src/SeatCanvas/SeatCanvas.framework/Versions/A/SeatCanvas
    LLVM_PROFILE_FILE="${PROFRAW_DIR}/seatcanvas_%p.profraw" ${BUILD_DIR}/tests/SeatCanvasFullTests || true
    xcrun llvm-profdata merge -sparse ${PROFRAW_DIR}/*.profraw -o ${PROFRAW_DIR}/coverage.profdata

    echo ""
    echo "=== Coverage Summary ==="
    xcrun llvm-cov report ${BUILD_DIR}/tests/SeatCanvasFullTests \
        -object ${FRAMEWORK_DYLIB} \
        -instr-profile=${PROFRAW_DIR}/coverage.profdata \
        -ignore-filename-regex=".*/third_party/.*|.*/tests/.*"

    xcrun llvm-cov show ${BUILD_DIR}/tests/SeatCanvasFullTests \
        -object ${FRAMEWORK_DYLIB} \
        -instr-profile=${PROFRAW_DIR}/coverage.profdata \
        -ignore-filename-regex=".*/third_party/.*|.*/tests/.*" \
        -format=html \
        --output-dir=${PROFRAW_DIR}/html

    echo ""
    echo "Coverage report: ${PROFRAW_DIR}/html/index.html"
else
    ${BUILD_DIR}/tests/SeatCanvasFullTests
fi
