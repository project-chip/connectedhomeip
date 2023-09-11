#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# Internal script for pull_submodules.sh. Not intended to be used directly.

set -e

CHIP_ROOT="$(dirname "$0")/../.."

if [[ -z "$name" || -z "$toplevel" ]]; then
    echo >&2 "This script should be run via update_submodules.sh"
    exit 1
fi

get_submodule_config() {
    git config --file "$toplevel/.gitmodules" "submodule.$name.$1"
}

SUBMODULE_URL=$(get_submodule_config url)
SUBMODULE_BRANCH=$(get_submodule_config branch)
SUBMODULE_PATH=$(get_submodule_config path)

git fetch --quiet "$SUBMODULE_URL" "$SUBMODULE_BRANCH"

HEAD_COMMIT=$(git -C "$toplevel" rev-parse HEAD:"$SUBMODULE_PATH")
FETCH_COMMIT="$(git rev-parse FETCH_HEAD)"

if [[ "$HEAD_COMMIT" == "$FETCH_COMMIT" ]]; then
    exit 0
fi

git checkout --quiet "$FETCH_COMMIT"
git -C "$toplevel" add "$SUBMODULE_PATH"

echo "    git -C $SUBMODULE_PATH log $(git rev-parse --short "$HEAD_COMMIT")..$(git rev-parse --short "$FETCH_COMMIT")"
