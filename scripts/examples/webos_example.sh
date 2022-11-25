#!/usr/bin/env bash
#
#    Copyright (c) 2022 Project CHIP Authors
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

SCRIPT_DIR=$(dirname "$0")
REPO_DIR=$(realpath "$SCRIPT_DIR/../..")
WEBOS_REPO_DIR="$REPO_DIR/third_party/webos_sdk"
WEBOS_SDK_VER="v2.14.1"

WEBOS_SDK_PATH="$WEBOS_REPO_DIR/$WEBOS_SDK_VER"
WEBOS_SYSROOT="$WEBOS_SDK_PATH/sysroots"
WEBOS_SDK_EXTRACT_SCRIPT_PATH="$WEBOS_SDK_PATH/webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.sh"
ENVIRONMENT_SETUP_SCRIPT="$WEBOS_SDK_PATH/environment-setup-cortexa7t2hf-neon-vfpv4-webos-linux-gnueabi"

# Activating connectedhomeip build environment
source "$REPO_DIR/scripts/activate.sh"

echo "Clone webos_sdk"
echo "##### Cloning webOS OSE SDK #####"

if [ ! -d "$WEBOS_REPO_DIR" ]; then
    git clone https://github.com/cabin15/webos-ose-ndk "$WEBOS_REPO_DIR"
else
    echo "webOS-ose-ndk already exists in $WEBOS_REPO_DIR"
fi

echo "Extract webOS SDK"
if [ ! -e "$WEBOS_SDK_EXTRACT_SCRIPT_PATH" ]; then
    cat "$WEBOS_SDK_PATH/webos_sdk.tar"* | (
        cd "$WEBOS_SDK_PATH" || (echo "Failed to cd to $WEBOS_SDK_PATH" && exit 1)
        tar xvzf -
    )
else
    echo "webOS SDK already extracted"
fi

if [ ! -d "$WEBOS_SYSROOT" ]; then
    echo "webOS SDK not extracted"
    chmod 555 "$WEBOS_SDK_EXTRACT_SCRIPT_PATH"

    echo "##### Install webOS OSE NDK #####"
    "$WEBOS_SDK_EXTRACT_SCRIPT_PATH" -d "$WEBOS_SDK_PATH" -y
fi

# Activating webOS NDK build environment
echo "##### Activating webOS NDK build environment #####"
source "$ENVIRONMENT_SETUP_SCRIPT"
echo ""

# Build webos example
echo "##### Build webos example #####"
echo "##### Performing gn gen #####"
gn gen "$REPO_DIR/out/host" --args="is_debug=false target_os=\"webos\" target_cpu=\"arm\" chip_enable_python_modules=false ar_webos=\"$AR\" cc_webos=\"$CC -Wno-format-security\" cxx_webos=\"$CXX\" webos_sysroot=\"$PKG_CONFIG_SYSROOT_DIR\" chip_build_tests=false enable_syslog=true treat_warnings_as_errors=false"

echo "##### Building by ninja #####"
ninja -C "$REPO_DIR/out/host"
