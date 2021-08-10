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

set -e
set -x
env

# todo: we do not really need SDK and NDK here, remove it later if posiable
if [ -z "$ANDROID_HOME" ]; then
    echo "ANDROID_HOME not set!"
    exit 1
fi

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ANDROID_NDK_HOME not set!"
    exit 1
fi

# Build shared CHIP libs
source scripts/activate.sh
gn gen --check --fail-on-unused-args out/"android_as" --args="target_os=\"android\" target_cpu=\"arm64\" android_ndk_root=\"$ANDROID_NDK_HOME\" android_sdk_root=\"$ANDROID_HOME\""  --ide=json --json-ide-script=//scripts/examples/gn_to_cmake.py
