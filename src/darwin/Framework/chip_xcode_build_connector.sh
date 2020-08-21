#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
#    All rights reserved.
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

here=$(cd "${0%/*}" && pwd)
me=${0##*/}

die() {
    echo "$me: *** ERROR: $*"
    exit 1
}

set -e

mkdir -p "$TEMP_DIR"
export >"$TEMP_DIR/env.sh"

set -x

# these should be set by the Xcode project
CHIP_ROOT=${CHIP_ROOT:-"$SRCROOT/../../.."}
CHIP_ROOT=$(cd "$CHIP_ROOT" && pwd)

[[ -d ${CHIP_ROOT} ]] || die Please set CHIP_ROOT to the location of the CHIP directory

declare -a DEFINES=()
# lots of environment variables passed by Xcode to this script
read -r -a DEFINES <<<"$GCC_PREPROCESSOR_DEFINITIONS"

declare target_defines=
for define in "${DEFINES[@]}"; do

    # skip over those that GN does for us
    case "$define" in
        CHIP_LOGGING_STYLE*)
            continue
            ;;
        CHIP_DEVICE_LAYER*)
            continue
            ;;
        CHIP_*_CONFIG_INCLUDE)
            continue
            ;;
        CHIP_SYSTEM_CONFIG_*)
            continue
            ;;
        CONFIG_NETWORK_LAYER*)
            continue
            ;;
        CHIP_CRYPTO_*)
            continue
            ;;
    esac
    target_defines+=,\"${define//\"/\\\"}\"
done
target_defines=[${target_defines:1}]

declare -a args=(
    'chip_crypto="mbedtls"'
    'chip_build_tools=false'
    'chip_build_tests=false'
    'chip_logging_style="external"'
    'chip_ble_project_config_include=""'
    'chip_device_project_config_include=""'
    'chip_inet_project_config_include=""'
    'chip_system_project_config_include=""'
    'target_cpu="'"$ARCHS"'"'
    'target_defines='"$target_defines"
    'default_configs_cosmetic=[]'
    'target_cflags=["-target","'"$ARCHS"'-'"$LLVM_TARGET_TRIPLE_VENDOR"'-'"$LLVM_TARGET_TRIPLE_OS_VERSION"'", "-DNDEBUG"]'
)

if [[ ${CONFIGURATION} != Debug* ]]; then
    args+='is_debug=true'
fi

[[ ${PLATFORM_FAMILY_NAME} == iOS ]] && {
    args+=(
        'target_os="ios"'
        'import("//config/ios/args.gni")'
    )
}

[[ ${PLATFORM_FAMILY_NAME} == macOS ]] && {
    args+=(
        'target_os="mac"'
        'chip_project_config_include_dirs=["'"$CHIP_ROOT"'/config/standalone"]'
        'import("'"$CHIP_ROOT"'/config/standalone/args.gni")'
    )
}

(
    # activate.sh isn't 'set -e' safe
    cd "$CHIP_ROOT" # pushd and popd because we need the env vars from activate
    set +ex
    PW_ENVSETUP_QUIET=1 . scripts/activate.sh
    set -ex

    cd "$TEMP_DIR"
    # [[ -f out/build.ninja ]] ?
    gn --root="$CHIP_ROOT" gen --check out --args="${args[*]}"
    ninja -v -C out
)
