#!/usr/bin/env bash

# Baseline workflow: verify stable branch first, then compare on current branch.
# Run this after pulling version.json changes, or before accepting baseline on a feature branch.

set -e

cd "$(dirname "$0")"

if [ "$(uname -s)" != "Darwin" ]; then
    echo "Error: baseline workflow is only supported on macOS."
    exit 1
fi

STABLE_BRANCH=develop
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

if [ "${CURRENT_BRANCH}" = "HEAD" ]; then
    echo "Error: detached HEAD. Switch to a branch first."
    exit 1
fi

if [ "${CURRENT_BRANCH}" = "${STABLE_BRANCH}" ]; then
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

if cmp -s "${STABLE_VERSION_FILE}" "${LOCAL_VERSION_FILE}"; then
    echo "version.json matches ${STABLE_BRANCH}, running autotest on current branch..."
    exec ./autotest.sh "$@"
fi

echo "version.json differs from ${STABLE_BRANCH}."
echo "Step 1: Verify baseline on ${STABLE_BRANCH}..."
CURRENT_COMMIT=$(git rev-parse HEAD)

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

if [ "${STABLE_EXIT}" -ne 0 ]; then
    echo "Error: autotest failed on ${STABLE_BRANCH}. Fix stable branch first."
    exit "${STABLE_EXIT}"
fi

echo ""
echo "Step 2: Compare on ${CURRENT_BRANCH}..."
./autotest.sh "$@"
