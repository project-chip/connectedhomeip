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

set -e

# Build script for GN examples GitHub workflow.

source "$(dirname "$0")/../../scripts/activate.sh"

if [ -z "$TIZEN_HOME" ]; then
    echo "TIZEN_HOME not set!"
    exit 1
fi

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
            echo >&2 "You can use -v for ninja args, and '=' or 'import' for gn args"
            exit 2
            ;;
    esac
done

set -x
env

PKG_CONFIG_SYSROOT_DIR=$TIZEN_HOME \
PKG_CONFIG_LIBDIR=$TIZEN_HOME/usr/lib/pkgconfig \
PKG_CONFIG_PATH=$TIZEN_HOME/usr/lib/pkgconfig \
gn gen --check --fail-on-unused-args --root="$EXAMPLE_DIR" "$OUTPUT_DIR" --args=''$GN_ARGS'
    target_os="tizen"
    target_cpu="arm" arm_arch="armv7-a"
    import("//build_overrides/build.gni")
    target_cflags=[ "--sysroot='$TIZEN_HOME'" ]
    target_ldflags=[ "--sysroot='$TIZEN_HOME'" ]
    custom_toolchain="${build_root}/toolchain/custom"
    target_cc="'$TIZEN_HOME'/bin/arm-linux-gnueabi-gcc"
    target_cxx="'$TIZEN_HOME'/bin/arm-linux-gnueabi-g++"
    target_ar="'$TIZEN_HOME'/bin/arm-linux-gnueabi-ar"'

ninja -C "$OUTPUT_DIR" "${NINJA_ARGS[@]}"
