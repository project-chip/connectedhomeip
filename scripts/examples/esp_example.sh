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

app="$1"
root=examples/$app/esp32/

shift 1

if [ -z "$app" ]; then
    echo "No mandatory app argument supplied!"
    exit 1
fi

source "scripts/activate.sh"
# shellcheck source=/dev/null
source "$IDF_PATH/export.sh"

for sdkconfig in "$root"/sdkconfig*.defaults; do
    # remove root path to get sdkconfig*.defaults name
    sdkconfig_name=${sdkconfig#"$root"/}
    rm -f "$root"/sdkconfig
    (
        cd "$root"
        idf.py -D SDKCONFIG_DEFAULTS="$sdkconfig_name" build
    ) || {
        echo "build $sdkconfig_name failed"
        exit 1
    }
    cp "$root"/build/chip-"$app".elf "$root"/build/"${sdkconfig_name%".defaults"}"-chip-"$app".elf
done
