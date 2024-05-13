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

# Activating connectedhomeip build environment
source scripts/activate.sh

# Clone webos_sdk
echo "##### Cloning webOS OSE SDK #####"
git clone https://github.com/cabin15/webos-ose-ndk third_party/webos_sdk

# Extract webOS SDK
cat third_party/webos_sdk/v2.14.1/webos_sdk.tar* | (
    cd third_party/webos_sdk/v2.14.1/
    tar xvzf -
)

# Grant execute permission for NDK install script
chmod 555 third_party/webos_sdk/v2.14.1/webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.sh

# Install webOS OSE NDK
echo "##### Install webOS OSE NDK #####"
third_party/webos_sdk/v2.14.1/webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.sh -d third_party/webos_sdk/v2.14.1 -y

# Activating webOS NDK build environment
echo "##### Activating webOS NDK build environment #####"
source third_party/webos_sdk/v2.14.1/environment-setup-cortexa7t2hf-neon-vfpv4-webos-linux-gnueabi
echo ""

# Build webos example
echo "##### Build webos example #####"
echo "##### Performing gn gen #####"
gn gen out/host --args="is_debug=false target_os=\"webos\" target_cpu=\"arm\" chip_enable_python_modules=false ar_webos=\"$AR\" cc_webos=\"$CC -Wno-format-security\" cxx_webos=\"$CXX\" webos_sysroot=\"$PKG_CONFIG_SYSROOT_DIR\" chip_build_tests=false enable_syslog=true treat_warnings_as_errors=false"

echo "##### Building by ninja #####"
ninja -C out/host
