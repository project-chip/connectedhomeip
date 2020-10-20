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

set -ex

mkdir -p "$TEMP_DIR"
export >"$TEMP_DIR/env.sh"

# these should be set by the Xcode project
CHIP_ROOT=$(cd "$here/../../.." && pwd)

[[ -d $CHIP_ROOT ]] || die Please set CHIP_ROOT to the location of the CHIP directory

declare -a DEFINES=()
# lots of environment variables passed by Xcode to this script
read -r -a DEFINES <<<"$GCC_PREPROCESSOR_DEFINITIONS"

declare target_defines=
for define in "${DEFINES[@]}"; do

    # skip over those that GN does for us
    case "$define" in
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
        CHIP_LOGGING_STYLE_*)
            continue
            ;;
    esac
    target_defines+=,\"${define//\"/\\\"}\"
done
target_defines=[${target_defines:1}]

declare target_cflags='"-target","'"$PLATFORM_PREFERRED_ARCH"'-'"$LLVM_TARGET_TRIPLE_VENDOR"'-'"$LLVM_TARGET_TRIPLE_OS_VERSION"'"'

read -r -a archs <<<"$ARCHS"

for arch in "${archs[@]}"; do
    target_cflags+=',"-arch","'"$arch"'"'
done

[[ $ENABLE_BITCODE == YES ]] && {
    target_cflags+=',"-flto"'
}

declare -a args=(
    'chip_crypto="mbedtls"'
    'chip_logging_style="darwin"'
    'chip_build_tools=false'
    'chip_build_tests=false'
    'chip_ble_project_config_include=""'
    'chip_device_project_config_include=""'
    'chip_inet_project_config_include=""'
    'chip_system_project_config_include=""'
    'target_cpu="'"$PLATFORM_PREFERRED_ARCH"'"'
    'target_defines='"$target_defines"
    'default_configs_cosmetic=[]'
    'target_cflags=['"$target_cflags"']'
)

[[ $CONFIGURATION != Debug* ]] && args+='is_debug=true'

[[ $PLATFORM_FAMILY_NAME == iOS ]] && {
    args+=(
        'target_os="ios"'
        'import("//config/ios/args.gni")'
    )
}

[[ $PLATFORM_FAMILY_NAME == macOS ]] && {
    args+=(
        'target_os="mac"'
        'import("//config/standalone/args.gni")'
        'chip_project_config_include_dirs=["'"$CHIP_ROOT"'/config/standalone"]'
    )
}

find_in_ancestors() {
    declare to_find="${1}"
    declare dir="${2:-$(pwd)}"

    while [[ ! -e ${dir}/${to_find} && -n ${dir} ]]; do
        dir=${dir%/*}
    done

    if [[ ! -e ${dir}/${to_find} ]]; then
        printf 'error: find_in_ancestors: %s not found\n' "$to_find" >&2
        return 1
    fi
    printf '%s\n' "$dir/$to_find"
}

(
    cd "$CHIP_ROOT" # pushd and popd because we need the env vars from activate

    if ENV=$(find_in_ancestors chip_xcode_build_connector_env.sh 2>/dev/null); then
        . "$ENV"
    fi

    [[ -n $CHIP_NO_SUBMODULES ]] || git submodule update --init
    if [[ -z $CHIP_NO_ACTIVATE ]]; then
        # first run bootstrap in an external env to build everything
        env -i PW_ENVSETUP_NO_BANNER=1 PW_ENVSETUP_QUIET=1 bash -c '. scripts/activate.sh'
        set +ex
        # now source activate for env vars
        PW_ENVSETUP_NO_BANNER=1 PW_ENVSETUP_QUIET=1 . scripts/activate.sh
        set -ex
    fi

    cd "$TEMP_DIR"
    # [[ -f out/build.ninja ]] ?
    gn --root="$CHIP_ROOT" gen --check out --args="${args[*]}"
    ninja -v -C out
)
