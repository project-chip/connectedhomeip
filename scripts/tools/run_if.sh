#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -x

if [[ $1 == *"$2"* ]]; then
    shift
    shift
    "$@"
fi
