#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -x

git clean -Xdf

git submodule foreach git clean -Xdf

# TODO - Remove after fixing
# https://bugs.chromium.org/p/pigweed/issues/detail?id=265
if test -n "$PW_ENVIRONMENT_ROOT" -a -w "$PW_ENVIRONMENT_ROOT"; then
    rm -rf "$PW_ENVIRONMENT_ROOT"
fi
