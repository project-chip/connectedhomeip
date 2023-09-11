#!/bin/sh
#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

git grep -n -E '^[[:space:]]*#[[:space:]]*include[[:space:]]*[<"]' src |
    python3 "${0%.*}.py"
