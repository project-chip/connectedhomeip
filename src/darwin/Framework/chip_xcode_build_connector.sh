#!/bin/bash -e

#
#    Copyright (c) 2020-2024 Project CHIP Authors
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

# This script connects Xcode's "Run Script" build phase to a build of CHIP for Apple's environments.
#
# Conventions used in this script:
#  * Variables in upper case supplied by Xcode (or other executor), are exported to subprocesses, or
#      are upper-case elsewhere in CHIP scripts (e.g. CHIP_ROOT is only used locally,
#      but is all uppper). Variables defined herein and used locally are lower-case
#

CHIP_ROOT=$(cd "$(dirname "$0")/../../.." && pwd)

function format_gn_str() {
    local val="$1"
    val="${val//\\/\\\\}"         # escape '\'
    val="${val//\$/\\\$}"         # escape '$'
    echo -n "\"${val//\"/\\\"}\"" # escape '"'
}

function format_gn_list() {
    local val sep=
    echo -n "["
    for val in "$@"; do
        echo -n "$sep"
        format_gn_str "$val"
        sep=", "
    done
    echo "]"
}

# We only have work to do for the `installapi` and `build` phases
[[ "$ACTION" == installhdrs ]] && exit 0

mkdir -p "$TEMP_DIR"

# For debugging, save off environment that Xcode gives us, can source it to
# retry/repro failures from a bash terminal
#export >"$TEMP_DIR/env.sh"
#set -x

# Forward defines from Xcode (GCC_PREPROCESSOR_DEFINITIONS)
declare -a target_defines=()
read -r -a xcode_defines <<<"$GCC_PREPROCESSOR_DEFINITIONS"
for define in "${xcode_defines[@]}"; do
    # skip over those that GN does for us
    case "$define" in
        CHIP_HAVE_CONFIG_H) continue ;;
    esac
    target_defines+=("$define")
done

# Forward C/C++ flags (OTHER_C*FLAGS)
read -r -a target_cflags_c <<<"$OTHER_CFLAGS"
read -r -a target_cflags_cc <<<"$OTHER_CPLUSPLUSFLAGS"

# Handle target OS and arch
declare target_arch=
declare target_cpu=
declare -a target_cflags=()
declare current_arch="$(uname -m)"
declare deployment_target="$LLVM_TARGET_TRIPLE_OS_VERSION$LLVM_TARGET_TRIPLE_SUFFIX"
declare deployment_variant=
if [[ "$IS_ZIPPERED" == YES ]]; then
    if [[ "$CLANG_TARGET_TRIPLE_VARIANTS" != *-apple-* ]]; then
        echo "Unable to determine target variant for zippered build" >&2
        exit 1
    fi
    deployment_variant="${CLANG_TARGET_TRIPLE_VARIANTS/*-apple-/}"
fi

read -r -a archs <<<"$ARCHS"
for arch in "${archs[@]}"; do
    if [ -z "$target_arch" ] || [ "$arch" = "$current_arch" ]; then
        target_arch="$arch"
        case "$arch" in
            x86_64) target_cpu="x64" ;;
            *) target_cpu="$arch" ;;
        esac
    fi
    [[ "${#archs[@]}" -gt 1 ]] && target_cflags+=(-arch "$arch")
    [[ -n "$deployment_variant" ]] && target_cflags+=(-target-variant "$arch-apple-$deployment_variant")
done

# Translate other options
[[ $CHIP_ENABLE_ENCODING_SENTINEL_ENUM_VALUES == YES ]] && target_defines+=("CHIP_CONFIG_IM_ENABLE_ENCODING_SENTINEL_ENUM_VALUES=1")
[[ $ENABLE_BITCODE == YES ]] && target_cflags+=("-flto")

declare -a args=(
    'default_configs_cosmetic=[]' # suppress colorization
    'chip_crypto="boringssl"'
    'chip_build_controller_dynamic_server=false'
    'chip_build_tools=false'
    'chip_build_tests=false'
    'chip_enable_wifi=false'
    'chip_enable_python_modules=false'
    'chip_device_config_enable_dynamic_mrp_config=true'
    'chip_log_message_max_size=4096' # might as well allow nice long log messages
    'chip_logging_backend="none"'    # os_log() is integrated via CHIP_SYSTEM_CONFIG_PLATFORM_LOG
    'chip_disable_platform_kvs=true'
    'enable_fuzz_test_targets=false'
    "target_cpu=\"$target_cpu\""
    "mac_target_arch=\"$target_arch\""
    "mac_deployment_target=\"$deployment_target\""
    "target_defines=$(format_gn_list "${target_defines[@]}")"
    "target_cflags=$(format_gn_list "${target_cflags[@]}")"
    "target_cflags_c=$(format_gn_list "${target_cflags_c[@]}")"
    "target_cflags_cc=$(format_gn_list "${target_cflags_cc[@]}")"
)

case "$CONFIGURATION" in
    Debug) args+=('is_debug=true') ;;
    Release) args+=('is_debug=false') ;;
esac

[[ $PLATFORM_FAMILY_NAME != macOS ]] && {
    args+=(
        'target_os="ios"'
        'import("//config/ios/args.gni")'
    )
}

[[ $PLATFORM_FAMILY_NAME == macOS ]] && {
    args+=(
        'target_os="mac"'
    )
}

[[ $CHIP_INET_CONFIG_ENABLE_IPV4 == NO ]] && {
    args+=(
        'chip_inet_config_enable_ipv4=false'
    )
}

[[ $CHIP_IS_ASAN == YES || $ENABLE_ADDRESS_SANITIZER == YES ]] && {
    args+=(
        'is_asan=true'
    )
}

[[ $CHIP_IS_UBSAN == YES || $ENABLE_UNDEFINED_BEHAVIOR_SANITIZER == YES ]] && {
    args+=(
        'is_ubsan=true'
    )
}

[[ $CHIP_IS_TSAN == YES || $ENABLE_THREAD_SANITIZER == YES ]] && {
    args+=(
        'is_tsan=true'
        # The system stats stuff races on the stats in various ways,
        # so just disable it when using TSan.
        'chip_system_config_provide_statistics=false'
    )
}

[[ $CHIP_IS_CLANG == YES ]] && {
    args+=(
        'is_clang=true'
    )
}

[[ $CHIP_IS_BLE == NO ]] && {
    args+=(
        'chip_config_network_layer_ble=false'
    )
}

[[ $CHIP_ENABLE_ENCODING_SENTINEL_ENUM_VALUES == YES ]] && {
    args+=(
        'enable_encoding_sentinel_enum_values=true'
    )
}

# search current (or $2) and its parent directories until
#  a name match is found, which is output on stdout
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

# actual build stuff
{
    cd "$CHIP_ROOT"

    if ENV=$(find_in_ancestors chip_xcode_build_connector_env.sh 2>/dev/null); then
        . "$ENV"
    fi

    # there are environments where these bits are unwanted, unnecessary, or impossible
    [[ -n $CHIP_NO_SUBMODULES ]] || scripts/checkout_submodules.py --shallow --platform darwin
    if [[ -z $CHIP_NO_ACTIVATE ]]; then
        # first run bootstrap/activate in an external env to build everything
        env -i PW_ENVSETUP_NO_BANNER=1 PW_ENVSETUP_QUIET=1 bash -c '. scripts/activate.sh'
        # now source activate for env vars
        opts="$(set +o)"
        set +ex
        PW_ENVSETUP_NO_BANNER=1 PW_ENVSETUP_QUIET=1 . scripts/activate.sh
        eval "$opts"
    fi

    # put build intermediates in TEMP_DIR
    cd "$TEMP_DIR"

    # generate and build
    set -x
    gn --root="$CHIP_ROOT" gen --check out --args="${args[*]}"
    ninja -C out -v
    ninja -C out -t missingdeps
}
