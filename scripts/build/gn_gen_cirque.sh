#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

CHIP_ROOT="$(dirname "$0")/../.."
if [[ -z "${CHIP_ROOT_PATH}" ]]; then
    CHIP_ROOT_PATH=""
fi

set -x

env

echo "Build: GN configure"

gn --root="$CHIP_ROOT" gen --check --fail-on-unused-args "$CHIP_ROOT/out/debug" --args='target_os="all"'"chip_build_tests=false chip_enable_wifi=false chip_im_force_fabric_quota_check=true enable_default_builds=false enable_host_gcc_build=true enable_standalone_chip_tool_build=true enable_linux_all_clusters_app_build=true enable_linux_lighting_app_build=true"

echo "Build: Ninja build"

time ninja -C "$CHIP_ROOT/out/debug" all check
