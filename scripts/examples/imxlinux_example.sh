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
if [ "$#" != 2 && "$#" != 3 ]; then
    exit -1
fi

source "$(dirname "$0")/../../scripts/activate.sh"

if [ "$IMX_SDK_ROOT" = "" -o ! -d "$IMX_SDK_ROOT" ]; then
    echo "the Yocto SDK path is not specified with the shell env IMX_SDK_ROOT or an invalid path is specified"
    exit -1
fi
env

entries="$(echo "$(ls "$IMX_SDK_ROOT")" | tr -s '\n' ',')"
IFS=',' read -ra entry_array <<<"$entries"
for entry in "${entry_array[@]}"; do
    if [ "$(echo "$entry" | grep -E "^environment-setup-")" != "" ]; then
        env_setup_script=$entry
        break
    fi
done

if [ -z "$env_setup_script" ]; then
    echo "The SDK environment setup script is not found, make sure the env IMX_SDK_ROOT is correctly set."
    exit 1
fi

while read line; do
    # trim the potential whitespaces
    line=$(echo "$line" | xargs)

    if [ "$(echo "$line" | grep -E "^export SDKTARGETSYSROOT=")" != "" ]; then
        sdk_target_sysroot=${line#"export SDKTARGETSYSROOT="}
    fi

    if [ "$(echo "$line" | grep -E "^export CC=")" != "" ]; then
        cc=${line#"export CC="}
        cc=${cc#"\""}
        cc=${cc%"\""}
        cc=${cc/"\$SDKTARGETSYSROOT"/$sdk_target_sysroot}
    fi

    if [ "$(echo "$line" | grep -E "^export CXX=")" != "" ]; then
        cxx=${line#"export CXX="}
        cxx=${cxx#"\""}
        cxx=${cxx%"\""}
        cxx=${cxx/"\$SDKTARGETSYSROOT"/$sdk_target_sysroot}
    fi

    if [ "$(echo "$line" | grep -E "^export ARCH=")" != "" ]; then
        target_cpu=${line#"export ARCH="}

        if [ "$target_cpu" = "arm64" ]; then
            arm_arch="armv8-a"
        elif [ "$target_cpu" = "arm" ]; then
            arm_arch="armv7ve"
        else
            echo "ARCH should be arm64 or arm in the SDK environment setup script."
            exit 1
        fi
    fi

    if [ "$(echo "$line" | grep -E "^export CROSS_COMPILE=")" != "" ]; then
        cross_compile=${line#"export CROSS_COMPILE="}
        cross_compile=${cross_compile%"-"}
    fi
done <"$IMX_SDK_ROOT/$env_setup_script"

if [ -z "$sdk_target_sysroot" ]; then
    echo "SDKTARGETSYSROOT is not found in the SDK environment setup script."
    exit 1
fi

if [ -z "$cc" -o -z "$cxx" ]; then
    echo "CC and/or CXX are not found in the SDK environment setup script."
    exit 1
fi

if [ -z "$target_cpu" -o -z "$cross_compile" ]; then
    echo "ARCH and/or CROSS_COMPILE are not found in the SDK environment setup script."
    exit 1
fi

release_build=true
if [ "$3" = "debug" ]; then
    release_build=false
fi

PLATFORM_CFLAGS='-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"", "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"'
gn gen --check --fail-on-unused-args --root="$1" "$2" --args="target_os=\"linux\" target_cpu=\"$target_cpu\" arm_arch=\"$arm_arch\"
treat_warnings_as_errors=false
import(\"//build_overrides/build.gni\")
sysroot=\"$sdk_target_sysroot\"
target_cflags=[ \"$PLATFORM_CFLAGS\" ]
custom_toolchain=\"\${build_root}/toolchain/custom\"
target_cc=\"$IMX_SDK_ROOT/sysroots/x86_64-pokysdk-linux/usr/bin/$cross_compile/$cc\"
target_cxx=\"$IMX_SDK_ROOT/sysroots/x86_64-pokysdk-linux/usr/bin/$cross_compile/$cxx\"
target_ar=\"$IMX_SDK_ROOT/sysroots/x86_64-pokysdk-linux/usr/bin/$cross_compile/$cross_compile-ar\"
$(if [ "$release_build" = "true" ]; then echo "is_debug=false"; else echo "optimize_debug=true"; fi)"

ninja -C "$2"
