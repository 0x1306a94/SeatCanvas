#!/usr/bin/env bash

# Update local baseline md5 cache when tests/baseline/version.json changes.
# Run this after pulling baseline changes from develop.
#
# Usage: ./update_baseline.sh [autotest args]

set -e

cd "$(dirname "$0")"

if [ "$(uname -s)" != "Darwin" ]; then
    echo "Error: baseline workflow is only supported on macOS."
    exit 1
fi

CACHE_DIR="tests/baseline/.cache/metal"
CACHE_VERSION_FILE="${CACHE_DIR}/version.json"
STABLE_BRANCH=develop
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

if [ "${CURRENT_BRANCH}" = "HEAD" ]; then
    echo "Error: detached HEAD. Switch to a branch first."
    exit 1
fi

sync_baseline_cache() {
    if [ -f "${CACHE_VERSION_FILE}" ]; then
        if cmp -s tests/baseline/version.json "${CACHE_VERSION_FILE}"; then
            return 0
        fi
    fi

    echo "Refreshing baseline md5 cache (${CACHE_DIR})..."
    BUILD_DIR="build_test"
    mkdir -p "${BUILD_DIR}"
    if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
        cmake -S . -B "${BUILD_DIR}" \
            -G Ninja \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
            -DPLATFORM=MAC_ARM64 \
            -DSEATCANVAS_BUILD_TESTS=ON \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    fi
    cmake --build "${BUILD_DIR}" --target SeatCanvasUpdateBaseline -j"$(sysctl -n hw.logicalcpu)"
    "${BUILD_DIR}/tests/SeatCanvasUpdateBaseline"
}

if [ "${CURRENT_BRANCH}" = "${STABLE_BRANCH}" ]; then
    sync_baseline_cache
    echo "On ${STABLE_BRANCH}, running autotest..."
    exec ./autotest.sh "$@"
fi

STABLE_VERSION_FILE=$(mktemp)
LOCAL_VERSION_FILE=$(mktemp)
trap 'rm -f "${STABLE_VERSION_FILE}" "${LOCAL_VERSION_FILE}"' EXIT

if ! git show "${STABLE_BRANCH}:tests/baseline/version.json" > "${STABLE_VERSION_FILE}" 2>/dev/null; then
    echo "Error: cannot read tests/baseline/version.json from ${STABLE_BRANCH}."
    exit 1
fi
cp tests/baseline/version.json "${LOCAL_VERSION_FILE}"

sync_baseline_cache

if cmp -s "${STABLE_VERSION_FILE}" "${LOCAL_VERSION_FILE}"; then
    echo "version.json matches ${STABLE_BRANCH}, running autotest on current branch..."
    exec ./autotest.sh "$@"
fi

echo "version.json differs from ${STABLE_BRANCH}."
echo "Step 1: Verify baseline on ${STABLE_BRANCH}..."
STASHED=false
if [ -n "$(git status --porcelain)" ]; then
    git stash push --include-untracked --quiet -m "update_baseline"
    STASHED=true
fi

git switch "${STABLE_BRANCH}" --quiet
./sync_deps.sh
set +e
./autotest.sh "$@"
STABLE_EXIT=$?
set -e

git switch "${CURRENT_BRANCH}" --quiet

if [ "${STASHED}" = true ]; then
    git stash pop --quiet
fi
./sync_deps.sh
sync_baseline_cache

if [ "${STABLE_EXIT}" -ne 0 ]; then
    echo "Error: autotest failed on ${STABLE_BRANCH}. Fix stable branch first."
    exit "${STABLE_EXIT}"
fi

echo ""
echo "Step 2: Compare on ${CURRENT_BRANCH}..."
./autotest.sh "$@"
