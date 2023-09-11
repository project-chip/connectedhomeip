#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Build script for K32W examples GitHub workflow.

source "$(dirname "$0")/../../scripts/activate.sh"

set -x
env

"$(dirname "$0")"/../../third_party/nxp/k32w0_sdk/sdk_fixes/patch_k32w_sdk.sh

gn gen --check --fail-on-unused-args --root="$1" "$2" --args="k32w0_sdk_root=\"$NXP_K32W0_SDK_ROOT\" is_debug=false chip_crypto=\"mbedtls\" chip_with_se05x=1"
ninja -C "$2"
