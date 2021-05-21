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
root=examples/$app/esp32

IDF_REPO_URL="https://github.com/espressif/esp-idf.git"
IDF_BRANCH="release/v4.2"

#
# CLONE ESP-IDF
#

if [ -z "$IDF_PATH" ]; then
    echo "ESP-IDF is not installed! Installing local copy"
    idf_was_installed="1"
    if ! [ -d esp-idf ]; then
        git clone "$IDF_REPO_URL" -b "$IDF_BRANCH"
    fi
    export IDF_PATH="$PWD/esp-idf"
fi

if [ -x "$idf_was_installed" ]; then
    git -C "$IDF_PATH" submodule update --init --recursive
    cd "$IDF_PATH"
    ./install.sh
    . ./export.sh
fi

shift 1

if [ -z "$app" ]; then
    echo "No mandatory app argument supplied!"
    exit 1
fi

source "../activate.sh"
# shellcheck source=/dev/null

for sdkconfig in "$root"/sdkconfig*.defaults; do
    # remove root path to get sdkconfig*.defaults name
    sdkconfig_name=${sdkconfig#"$root"/}
    rm -f "$root"/sdkconfig
    SDKCONFIG_DEFAULTS=$sdkconfig_name idf.py -C "$root" defconfig "$@"
    cd "$root"
    idf.py build "$@" || {
        cd -
        echo "build $sdkconfig_name failed"
        exit 1
    }
    cd -
    cp "$root"/build/chip-"$app".elf "$root"/build/"${sdkconfig_name%".defaults"}"-chip-"$app".elf
done
