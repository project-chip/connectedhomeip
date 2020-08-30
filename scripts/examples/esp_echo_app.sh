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
env

root=examples/wifi-echo/server/esp32/

# shellcheck source=/dev/null
source "$root"/idf.sh

rm -f "$root"/sdkconfig
SDKCONFIG_DEFAULTS=sdkconfig_devkit.defaults idf make -j V=1 -C "$root" defconfig "$@"
idf make -j V=1 -C "$root" "$@" || {
    echo 'build DevKit-C failed'
    exit 1
}

rm -f "$root"/sdkconfig
SDKCONFIG_DEFAULTS=sdkconfig_m5stack.defaults idf make -j V=1 -C "$root" defconfig "$@"
idf make -j V=1 -C "$root" "$@" || {
    echo 'build M5Stack failed'
    exit 1
}
