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

if [ -z "$ANDROID_HOME" ]; then
    echo "ANDROID_HOME not set!"
    exit 1
fi

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ANDROID_NDK_HOME not set!"
    exit 1
fi

if [ -z "$TARGET_CPU" ]; then
    echo "TARGET_CPU not set! Candidates: arm, arm64, x86 and x64."
    exit 1
fi

source scripts/activate.sh
if [ -z "$USE_IDE" ] || [ "$USE_IDE" -eq '0' ]; then
    # Build shared CHIP libs
    echo "build scripts"
    gn gen --check --fail-on-unused-args out/"android_$TARGET_CPU" --args="target_os=\"android\" target_cpu=\"$TARGET_CPU\" android_ndk_root=\"$ANDROID_NDK_HOME\" android_sdk_root=\"$ANDROID_HOME\" chip_use_clusters_for_ip_commissioning=\"true\""
    ninja -C out/"android_$TARGET_CPU" src/setup_payload/java src/controller/java default

    rsync -a out/"android_$TARGET_CPU"/lib/*.jar src/android/CHIPTool/app/libs
    rsync -a out/"android_$TARGET_CPU"/lib/jni/* src/android/CHIPTool/app/libs/jniLibs
else
    # Build Cmake for Android Stduio
    echo "build ide"
    gn gen --check --fail-on-unused-args out/"android_$TARGET_CPU" --args="target_os=\"android\" target_cpu=\"$TARGET_CPU\" android_ndk_root=\"$ANDROID_NDK_HOME\" android_sdk_root=\"$ANDROID_HOME\" chip_use_clusters_for_ip_commissioning=\"true\"" --ide=json --json-ide-script=//scripts/examples/gn_to_cmakelists.py
fi
