#!/usr/bin/env bash

# Update local baseline cache from remote changes.
# Run this after pulling main branch that contains baseline changes from others.
# Without updating the cache, affected tests may report mismatches incorrectly.

set -e

cd "$(dirname "$0")"

if [ "$(uname -s)" != "Darwin" ]; then
    echo "Error: baseline update is only supported on macOS."
    exit 1
fi

CACHE_VERSION_FILE="./tests/baseline/.cache/version.json"

if [ -f "${CACHE_VERSION_FILE}" ]; then
    MAIN_VERSION=$(git show origin/main:tests/baseline/version.json 2>/dev/null || true)
    if [ -n "${MAIN_VERSION}" ]; then
        CACHE_CONTENT=$(cat "${CACHE_VERSION_FILE}")
        if [ "${MAIN_VERSION}" = "${CACHE_CONTENT}" ]; then
            exit 0
        fi
    fi
fi

echo "~~~~~~~~~~~~~~~~~~~Update Baseline Start~~~~~~~~~~~~~~~~~~~~~"

CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
CURRENT_COMMIT=$(git rev-parse HEAD)
BUILD_DIR="build-update-baseline"
COMPILE_RESULT=true

rm -rf "${BUILD_DIR}"
STASH_LIST_BEFORE=$(git stash list)
git stash push --include-untracked --quiet
STASH_LIST_AFTER=$(git stash list)
git switch develop --quiet

./sync_deps.sh

cmake -S "$(pwd)" -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
    -DPLATFORM=MAC_ARM64 \
    -DSEATCANVAS_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build "${BUILD_DIR}" --target SeatCanvasFullTests -j"$(sysctl -n hw.logicalcpu)"

if SEATCANVAS_UPDATE_BASELINE=1 "${BUILD_DIR}/tests/SeatCanvasFullTests"; then
    echo "~~~~~~~~~~~~~~~~~~~Update Baseline Success~~~~~~~~~~~~~~~~~~~~~"
else
    echo "~~~~~~~~~~~~~~~~~~~Update Baseline Failed~~~~~~~~~~~~~~~~~~~~~"
    COMPILE_RESULT=false
fi

if [ "${CURRENT_BRANCH}" = "HEAD" ]; then
    git checkout "${CURRENT_COMMIT}" --quiet
else
    git switch "${CURRENT_BRANCH}" --quiet
fi

if [ "${STASH_LIST_BEFORE}" != "${STASH_LIST_AFTER}" ]; then
    git stash pop --index --quiet
fi

./sync_deps.sh

if [ "${COMPILE_RESULT}" = false ]; then
    if [ -d tests/out ]; then
        mkdir -p result
        cp -r tests/out result
    fi
    exit 1
fi

rm -rf "${BUILD_DIR}"
