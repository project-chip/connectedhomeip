#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

CHIP_ROOT="$(dirname "$0")/../.."

PW_ENVSETUP_QUIET=1 source "$CHIP_ROOT/scripts/activate.sh"

set -x

env

gn gen out/default

ninja -C out/default
