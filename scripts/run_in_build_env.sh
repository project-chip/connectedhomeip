#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# This script executes the command given as an argument within
# CHIP_ROOT with a build environment activated.

if [[ -n ${BASH_SOURCE[0]} ]]; then
    source "${BASH_SOURCE[0]%/*}/activate.sh"
else
    source "${0%/*}/activate.sh"
fi

set -e

echo "Executing in build environment: $*"
bash -c -- "$@"
