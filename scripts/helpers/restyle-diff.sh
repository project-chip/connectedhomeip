#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
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
# if unspecified, ref defaults to "master"
#

here=${0%/*}

set -e

CHIP_ROOT=$(cd "$here/../.." && pwd)

restyle-paths() {
    url=https://github.com/restyled-io/restyler/raw/main/bin/restyle-path

    sh <(curl --location --proto "=https" --tlsv1.2 "$url" -sSf) "$@"
}

cd "$CHIP_ROOT"
declare -a paths="($(git diff --ignore-submodules --name-only "${1:-master}"))"

restyle-paths "${paths[@]}"
