#!/bin/bash

set -e

cd $(dirname $0)

PROJECT_DIR=$(pwd)
BUILD_DIR="build_test"

if [ "${1}" = "clean" ]; then
    rm -rf "${BUILD_DIR}"
fi

rm -rf ${BUILD_DIR}/CMakeCache.txt
mkdir -p "${BUILD_DIR}"

if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    cmake -S ${PROJECT_DIR} -B ${BUILD_DIR} \
        -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
        -DPLATFORM=MAC_ARM64 \
        -DSEATCANVAS_BUILD_TESTS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
fi

cmake --build ${BUILD_DIR} --target SeatCanvasFullTests -j$(sysctl -n hw.logicalcpu)

${BUILD_DIR}/tests/SeatCanvasFullTests
