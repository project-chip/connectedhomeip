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

# Build script for GN examples GitHub workflow.

source "$(dirname "$0")/../../scripts/activate.sh"

GN_ARGS=()

EXAMPLE_DIR=$1
shift
OUTPUT_DIR=$1
shift

NINJA_ARGS=()
for arg; do
    case $arg in
        -v)
            NINJA_ARGS+=(-v)
            ;;
        *=*)
            GN_ARGS+=("$arg")
            ;;
        *import*)
            GN_ARGS+=("$arg")
            ;;
        *)
            echo >&2 "invalid argument: $arg"
            exit 2
            ;;
    esac
done

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

python3 "$(dirname "$0")"/../../third_party/android_deps/set_up_android_deps.py

gn gen --check --fail-on-unused-args --root="$EXAMPLE_DIR" "$OUTPUT_DIR" --args="target_os=\"android\" target_cpu=\"$TARGET_CPU\" android_ndk_root=\"$ANDROID_NDK_HOME\" android_sdk_root=\"$ANDROID_HOME\" ${GN_ARGS[*]}"

ninja -C "$OUTPUT_DIR" "${NINJA_ARGS[@]}"

cp -rf "$OUTPUT_DIR/lib/jni/." "$EXAMPLE_DIR/App/app/libs/jniLibs/"
cp -f "$OUTPUT_DIR/lib/third_party/connectedhomeip/src/platform/android/AndroidPlatform.jar" "$EXAMPLE_DIR/App/app/libs/"
cp -f "$OUTPUT_DIR/lib/third_party/connectedhomeip/src/app/server/java/CHIPAppServer.jar" "$EXAMPLE_DIR/App/app/libs/"
cp -f "$OUTPUT_DIR/lib/third_party/connectedhomeip/src/setup_payload/java/SetupPayloadParser.jar" "$EXAMPLE_DIR/App/app/libs/"
cp -f "$OUTPUT_DIR/lib/TvApp.jar" "$EXAMPLE_DIR/App/app/libs/"
