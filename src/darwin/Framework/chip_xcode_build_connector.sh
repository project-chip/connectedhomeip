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

export >"$TEMP_DIR/env.sh"

export PKG_CONFIG_PATH="/usr/local/opt/openssl@1.1/lib/pkgconfig"

set -ex

if [ "$ARCHS" = arm64 ]; then
    target=aarch64-apple-darwin
else
    target=$ARCHS-apple-darwin
fi

# these should be set by the Xcode project
CHIP_ROOT=${CHIP_ROOT:-"$SRCROOT/../../.."}
CHIP_ROOT=$(cd "$CHIP_ROOT" && pwd)
CHIP_PREFIX=${CHIP_PREFIX:-"$BUILT_PRODUCTS_DIR"}

[[ -d ${CHIP_ROOT} ]] || die Please set CHIP_ROOT to the location of the CHIP directory

DEFINES=()
# lots of environment variables passed by xcode to this script
if [[ ${CONFIGURATION} == Debug* ]]; then
    configure_OPTIONS+=(--enable-debug)
    DEFINES+=(-UNDEBUG)
else
    DEFINES+=(-UDEBUG)
fi

read -r -a GCC_PREPROCESSOR_DEFINITIONS <<<"$GCC_PREPROCESSOR_DEFINITIONS"

DEFINES+=("${GCC_PREPROCESSOR_DEFINITIONS[@]/#/-D}")

ARCH_FLAGS="-arch $ARCHS"
SYSROOT_FLAGS="-isysroot $SDK_DIR"
COMPILER_FLAGS="$ARCH_FLAGS $SYSROOT_FLAGS ${DEFINES[*]}"

configure_OPTIONS+=(
    CPP="cc -E"
    CPPFLAGS="$COMPILER_FLAGS"
    CFLAGS="$COMPILER_FLAGS"
    CXXFLAGS="$COMPILER_FLAGS"
    OBJCFLAGS="$COMPILER_FLAGS"
    OBJCXXFLAGS="$COMPILER_FLAGS"
    LDFLAGS="$ARCH_FLAGS"
)

[[ ${PLATFORM_FAMILY_NAME} == iOS ]] && {
    configure_OPTIONS+=(--with-chip-project-includes="$CHIP_ROOT"/config/ios)
}

[[ ${PLATFORM_FAMILY_NAME} == macOS ]] && {
    configure_OPTIONS+=(--with-chip-project-includes="$CHIP_ROOT"/config/standalone)
}

configure_OPTIONS+=(
    --prefix="$CHIP_PREFIX"
    --target="$target"
    --host="$target"
    --disable-docs
    --disable-java
    --disable-python
    --disable-shared
    --disable-tests
    --disable-tools
    --with-device-layer=darwin
    --with-logging-style=external
    --with-chip-system-project-includes=no
    --with-chip-inet-project-includes=no
    --with-chip-ble-project-includes=no
    --with-chip-warm-project-includes=no
    --with-chip-device-project-includes=no
    --with-crypto=mbedtls
)

(
    cd "$TEMP_DIR"

    if [[ ! -x config.status || ${here}/${me} -nt config.status ]]; then
        "$CHIP_ROOT"/bootstrap-configure -C "${configure_OPTIONS[@]}"
    else
        while IFS= read -r -d '' makefile_am; do
            [[ ${makefile_am} -ot ${makefile_am/.am/.in} ]] || {
                "$CHIP_ROOT"/bootstrap -w make
                break
            }
        done < <(find "$CHIP_ROOT" -name Makefile.am)
    fi

    make V=1 install-data                   # all the headers
    make V=1 -C src/lib install             # libCHIP.a
    make V=1 -C src/platform install        # libDeviceLayer.a
    make V=1 -C src/setup_payload install   # libSetupPayload.a
    make V=1 -C third_party/mbedtls install # libmbedtls.a
)
