#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Bootstrap script for GN build GitHub workflow.

# This is used to account bootstrap time in a dedicated workflow step; there's
# no need to use this script locally.

source "$(dirname "$0")/../../scripts/activate.sh"
