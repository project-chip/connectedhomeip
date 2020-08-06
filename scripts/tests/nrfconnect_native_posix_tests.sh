#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

set -x
[[ -n $ZEPHYR_BASE ]] && source "$ZEPHYR_BASE/zephyr-env.sh"
env

CHIP_ROOT="$(dirname "$0")/../.."

cd "$CHIP_ROOT" &&
    ./bootstrap &&
    cd src/test_driver/nrfconnect &&
    west build -b native_posix &&
    cd build &&
    timeout 5m ctest -V
