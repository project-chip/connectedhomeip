#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# GN gen script for GN build GitHub workflow.

CHIP_ROOT="$(dirname "$0")/../.."
if [[ -z "${CHIP_ROOT_PATH}" ]]; then
    CHIP_ROOT_PATH=""
fi

source "$CHIP_ROOT/scripts/activate.sh"

set -x

env

gn --root="$CHIP_ROOT/$CHIP_ROOT_PATH" gen --check --fail-on-unused-args "$CHIP_ROOT/out/$BUILD_TYPE" "$@"
