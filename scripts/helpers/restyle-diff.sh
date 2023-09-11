#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# Use git and restyle-path to help reformat anything that differs from
#  some base edit point.  Use before pushing a PR to make sure everything
#  you've written is kosher to CI
#
# Usage:
#  restyle-diff.sh [ref]
#
# if unspecified, ref defaults to upstream/master (or master)
#

here=${0%/*}

set -e

CHIP_ROOT=$(cd "$here/../.." && pwd)
cd "$CHIP_ROOT"

restyle-paths() {
    if hash restyle-path 2>/dev/null; then
        echo "$@" | xargs restyle-path
    else
        url=https://github.com/restyled-io/restyler/raw/main/bin/restyle-path
        echo "$@" | xargs sh <(curl --location --proto "=https" --tlsv1.2 "$url" -sSf)
    fi
}

ref="$1"
if [[ -z "$ref" ]]; then
    ref="master"
    git remote | grep -qxF upstream && ref="upstream/master"
fi

declare -a paths=("$(git diff --ignore-submodules --name-only --merge-base "$ref")")
restyle-paths "${paths[@]}"
