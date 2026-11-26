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

cd "$(dirname "$0")/../.."

if [ "$ANDROID_HOME" = "" ]; then
    echo "ANDROID_HOME not set!"
    exit 1
fi

if [ "$ANDROID_NDK_HOME" = "" ]; then
    echo "ANDROID_NDK_HOME not set!"
    exit 1
fi

if [ "$TARGET_CPU" = "" ]; then
    echo "TARGET_CPU not set! Candidates: arm, arm64, x86 and x64."
    exit 1
fi

if [ "$PW_PROJECT_ROOT" = "" ]; then
    source scripts/activate.sh
fi

# Set up JARs
python3 third_party/android_deps/set_up_android_deps.py
third_party/java_deps/set_up_java_deps.sh

# Build CMake for Android Studio
echo "build ide"
gn gen --check --fail-on-unused-args out/"android_$TARGET_CPU" --args="target_os=\"android\" target_cpu=\"$TARGET_CPU\" android_ndk_root=\"$ANDROID_NDK_HOME\" android_sdk_root=\"$ANDROID_HOME\"" --ide=json --json-ide-script=//scripts/examples/gn_to_cmakelists.py

echo "=== Android App IDE Setup Completed Successfully ==="
echo "Output build directory: out/android_$TARGET_CPU"
echo "To build from source in Android Studio or Gradle:"
echo "1. Set 'matterSdkSourceBuild=true' in examples/android/CHIPTool/gradle.properties"
echo "2. Set 'matterBuildSrcDir=../../../../out/android_$TARGET_CPU' in gradle.properties"
echo "3. Open examples/android/CHIPTool in Android Studio or run cd examples/android/CHIPTool && ./gradlew assembleDebug"
