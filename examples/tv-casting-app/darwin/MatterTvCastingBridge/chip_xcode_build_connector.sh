#!/usr/bin/env bash

#
#    Copyright (c) 2022 Project CHIP Authors
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

here=$(cd "${0%/*}" && pwd)
me=${0##*/}

CHIP_ROOT=$(cd "$here/../../../.." && pwd)

die() {
    echo "$me: *** ERROR: $*"
    exit 1
}

# lotsa debug output :-)
set -ex

# helpful debugging, save off environment that Xcode gives us, can source it to
#  retry/repro failures from a bash terminal
mkdir -p "$TEMP_DIR"
export >"$TEMP_DIR/env.sh"

declare -a defines=()
# lots of environment variables passed by Xcode to this script
read -r -a defines <<<"$GCC_PREPROCESSOR_DEFINITIONS"

declare target_defines=
for define in "${defines[@]}"; do

    # skip over those that GN does for us
    case "$define" in
        CHIP_HAVE_CONFIG_H)
            continue
            ;;
    esac
    target_defines+=,\"${define//\"/\\\"}\"
done
target_defines=[${target_defines:1}]

declare target_arch=
declare target_cpu=
declare target_cflags=
declare current_arch="$(uname -m)"

read -r -a archs <<<"$ARCHS"
for arch in "${archs[@]}"; do
    if [ -z "$target_arch" ] || [ "$arch" = "$current_arch" ]; then
        target_arch="$arch"
        case "$arch" in
            x86_64) target_cpu="x64" ;;
            *) target_cpu="$arch" ;;
        esac
    fi
    if [ -n "$target_cflags" ]; then
        target_cflags+=','
    fi
    target_cflags+='"-arch","'"$arch"'"'
done

[[ $ENABLE_BITCODE == YES ]] && {
    if [ -n "$target_cflags" ]; then
        target_cflags+=','
    fi
    target_cflags+='"-flto"'
}

target_cflags+=',"-fno-c++-static-destructors"'

declare -a args=(
    'default_configs_cosmetic=[]' # suppress colorization
    'chip_crypto="mbedtls"'
    'chip_build_tools=false'
    'chip_build_tests=false'
    'target_cpu="'"$target_cpu"'"'
    'target_defines='"$target_defines"
    'target_cflags=['"$target_cflags"']'
    "mac_target_arch=\"$target_arch\""
    "mac_deployment_target=\"$LLVM_TARGET_TRIPLE_OS_VERSION$LLVM_TARGET_TRIPLE_SUFFIX\""
    'build_tv_casting_common_a=true'
)

[[ $CONFIGURATION != Debug* ]] && args+='is_debug=true'

[[ $PLATFORM_FAMILY_NAME != macOS ]] && {
    args+=(
        'target_os="ios"'
        'import("//examples/tv-casting-app/darwin/args.gni")'
    )
}

[[ $PLATFORM_FAMILY_NAME == macOS ]] && {
    args+=(
        'target_os="mac"'
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
(
    cd "$CHIP_ROOT" # pushd and popd because we need the env vars from activate

    if ENV=$(find_in_ancestors chip_xcode_build_connector_env.sh 2>/dev/null); then
        . "$ENV"
    fi

    # there are environments where these bits are unwanted, unnecessary, or impossible
    [[ -n $CHIP_NO_SUBMODULES ]] || scripts/checkout_submodules.py --shallow --platform darwin
    if [[ -z $CHIP_NO_ACTIVATE ]]; then
        # first run bootstrap/activate in an external env to build everything
        env -i PW_ENVSETUP_NO_BANNER=1 PW_ENVSETUP_QUIET=1 bash -c '. scripts/activate.sh'
        set +ex
        # now source activate for env vars
        PW_ENVSETUP_NO_BANNER=1 PW_ENVSETUP_QUIET=1 . scripts/activate.sh
        set -ex
    fi

    # put build intermediates in TEMP_DIR
    cd "$TEMP_DIR"

    # gnerate and build
    gn --root="$CHIP_ROOT" gen --check out --args="${args[*]}"
    ninja -v -C out
)
