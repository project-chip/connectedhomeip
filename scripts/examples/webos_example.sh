#!/usr/bin/env bash

#
#    Copyright (c) 2021 Project CHIP Authors
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

# Download webOS OSE NDK
echo "##### Download webOS OSE NDK #####"
wget http://10.178.94.187/ndk/opt/webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.host.manifest
wget http://10.178.94.187/ndk/opt/webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.sh
wget http://10.178.94.187/ndk/opt/webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.target.manifest
wget http://10.178.94.187/ndk/opt/webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.testdata.json

# Grant execute permission for NDK install script
chmod 555 webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.sh

# Install webOS OSE NDK
echo "##### Install webOS OSE NDK #####"
#sudo ./webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.sh

# Activating connectedhomeip build environment
source scripts/activate.sh

# Activating webOS NDK build environment
echo "##### Activating webOS NDK build environment #####"
source /opt/webos-sdk-x86_64/environment-setup-cortexa7t2hf-neon-vfpv4-webos-linux-gnueabi
echo ""

# Build webos example
echo "##### Build webos example #####"
echo "##### Performing gn gen #####"
gn gen out/host --args="is_debug=false target_os=\"webos\" target_cpu=\"arm\" chip_enable_python_modules=false ar_webos=\"${AR}\" cc_webos=\"${CC}\" cxx_webos=\"${CXX}\" sysroot_webos=\"${PKG_CONFIG_SYSROOT_DIR}\" chip_build_tests=false enable_syslog=true chip_config_network_layer_ble=false"

echo "##### Building by ninja #####"
ninja -C out/host

# Remove webOS OSE NDK archive
rm -rf webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.host.manifest
rm -rf webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.sh
rm -rf webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.target.manifest
rm -rf webos-sdk-x86_64-cortexa7t2hf-neon-vfpv4-toolchain-2.14.1.g.testdata.json
