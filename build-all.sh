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

echo 'Building ot-br-posix for Thread BR simulation..'
(cd third_party/ot-br-posix/repo &&
    ./build.sh) |
    while IFS= read -r line; do
        printf "\r%s%s" "$(tput el)" "$line"
    done

echo 'Done'

echo 'Building openthread for Thread RCP simulation..'
(cd third_party/openthread/repo &&
    ./build.sh) |
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
