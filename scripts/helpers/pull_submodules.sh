#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# Helper script to update all initialized submodules to the current tip of the
# branch specified in .gitmodules.

set -e

CHIP_ROOT="$(dirname "$0")/../.."

if ! git -C "$CHIP_ROOT" diff-index --cached HEAD; then
    echo >&2 "Staged changes, please reset or commit first."
    exit 1
fi

UPDATES=$(git -C "$CHIP_ROOT" submodule --quiet foreach "bash \$toplevel/scripts/helpers/pull_submodule.sh")

git -C "$CHIP_ROOT" add .gitmodules

git -C "$CHIP_ROOT" commit -m "Pull submodules

Via scripts/helpers/pull_submodules.sh

Updates:
$UPDATES"
