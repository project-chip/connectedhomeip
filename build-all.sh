#!/bin/bash
#
# Copyright (c) 2025 Project CHIP Authors
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

set -euo pipefail

build-otbr()
{
  ./script/cmake-build \
  -DOTBR_MDNS=openthread \
  -DOT_LOG_LEVEL=INFO \
  -DOT_TREL=OFF \
  -DOTBR_BORDER_ROUTING=ON \
  -DOT_FIREWALL=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=OFF \
  "$@"
}

build-ot-rcp()
{
  ./script/cmake-build simulation \
    -DBUILD_TESTING=OFF \
    -DOT_BUILD_GTEST=OFF \
    -DOT_CHANNEL_MANAGER=OFF \
    -DOT_CHANNEL_MONITOR=OFF \
    -DOT_LOG_MAX_SIZE=1024 \
    -DOT_LOG_OUTPUT=PLATFORM_DEFINED \
    "$@"
}

echo 'Building ot-br-posix for Thread BR simulation..'
export -f build-otbr
(cd third_party/ot-br-posix/repo &&
    build-otbr) |
    while IFS= read -r line; do
        printf "\r%s%s" "$(tput el)" "$line"
    done

echo 'Done'

echo 'Building openthread for Thread RCP simulation..'
export -f build-ot-rcp
(cd third_party/openthread/repo &&
    build-ot-rcp) |
    while IFS= read -r line; do
        printf "\r%s%s" "$(tput el)" "$line"
    done
echo 'Done'

set +u
source scripts/activate.sh
set -u

echo 'Building chip tool as the Matter Commissioner..'
(cd examples/chip-tool &&
    gn gen out/debug --add-export-compile-commands='*' &&
    ninja -C out/debug)
echo 'Done'

echo 'Building chip-lighting-app as the Matter Commissionee..'
(cd examples/lighting-app/linux &&
    gn gen out/debug --add-export-compile-commands='*' &&
    ninja -C out/debug)
echo 'Done'

echo 'You may now run ./env.sh ./test-commission-over-thread.exp'
