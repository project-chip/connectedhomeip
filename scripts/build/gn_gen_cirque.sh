#!/usr/bin/env bash

#
# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

CHIP_ROOT="$(dirname "$0")/../.."
if [[ -z "${CHIP_ROOT_PATH}" ]]; then
    CHIP_ROOT_PATH=""
fi

set -x

env

cd "$ROOT_PATH"

echo "Ensure submodules for Linux builds are checked out"
./scripts/checkout_submodules.py --allow-changing-global-git-config --shallow --platform linux

echo "Setup build environment"
source "./scripts/activate.sh"

echo "Build Samples needed by the tests"
./scripts/examples/gn_build_example.sh examples/all-clusters-app/linux/ out/debug/standalone chip_inet_config_enable_ipv4=false 'chip_logging_backend="stdio"' 'chip_enable_wifi=false' 'chip_build_tests=false' 'chip_im_force_fabric_quota_check=true'
./scripts/examples/gn_build_example.sh examples/lighting-app/linux/ out/debug/standalone chip_inet_config_enable_ipv4=false 'chip_logging_backend="stdio"' 'chip_enable_wifi=false' 'chip_build_tests=false' 'chip_im_force_fabric_quota_check=true'
./scripts/examples/gn_build_example.sh examples/lit-icd-app/linux/ out/debug/standalone chip_inet_config_enable_ipv4=false 'chip_logging_backend="stdio"' 'chip_enable_wifi=false' 'chip_build_tests=false' 'chip_im_force_fabric_quota_check=true'
