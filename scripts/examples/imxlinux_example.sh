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

set -e
set -x
if [ "$#" != 2 ]; then
    exit -1
fi

source "$(dirname "$0")/../../scripts/activate.sh"

if [ "$IMX_SDK_ROOT" = "" ]; then
    if [ "$CROSS_COMPILE" != "aarch64-poky-linux-" ]; then
        echo "i.MX SDK root not aarch64"
        exit -1
    fi
else
    source "$IMX_SDK_ROOT"/environment-setup-cortexa53-crypto-poky-linux
fi
env

PLATFORM_CFLAGS='-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"", "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"'
PKG_CONFIG_SYSROOT_DIR=$PKG_CONFIG_SYSROOT_DIR \
    PKG_CONFIG_LIBDIR="$PKG_CONFIG_PATH" \
    gn gen --check --fail-on-unused-args --root="$1" "$2" --args="target_os=\"linux\" target_cpu=\"arm64\" arm_arch=\"armv8-a\"
import(\"//build_overrides/build.gni\")
target_cflags=[ \"--sysroot=$SDKTARGETSYSROOT\", \"$PLATFORM_CFLAGS\" ]
target_ldflags = [ \"--sysroot=$SDKTARGETSYSROOT\" ]
custom_toolchain=\"\${build_root}/toolchain/custom\"
target_cc=\"$OECORE_NATIVE_SYSROOT/usr/bin/aarch64-poky-linux/aarch64-poky-linux-gcc\"
target_cxx=\"$OECORE_NATIVE_SYSROOT/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++\"
target_ar=\"$OECORE_NATIVE_SYSROOT/usr/bin/aarch64-poky-linux/aarch64-poky-linux-ar\""

ninja -C "$2"
