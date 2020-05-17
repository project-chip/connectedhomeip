#!/usr/bin/env bash

me=${0##*/}

die() {
    echo "$me: *** ERROR: $*"
    exit 1
}

set -ex

if [[ ${ARCHS} = arm64 ]]; then
    target=aarch64-apple-darwin
else
    target=$ARCHS-apple-darwin
fi

# TODO: set in the xcode project?
CHIP_ROOT=$(cd "$SRCROOT/../../.." && pwd)

[[ -d ${CHIP_ROOT} ]] || die Please set CHIP_ROOT to the location of the CHIP directory

# lots of environment variables passed by xcode to this script
if [[ ${CONFIGURATION} == Debug ]]; then
    configure_OPTIONS+=(--enable-debug)
    DEFINES+="-DDEBUG=1 -UNDEBUG"
else
    DEFINES+="-DNDEBUG=1 -UDEBUG"
fi

ARCH_FLAGS="-arch $ARCHS"
SYSROOT_FLAGS="-isysroot $SDK_DIR"
COMPILER_FLAGS="$ARCH_FLAGS $SYSROOT_FLAGS $DEFINES"

configure_OPTIONS+=(
    CPP="cc -E"
    CPPFLAGS="$COMPILER_FLAGS"
    CFLAGS="$COMPILER_FLAGS"
    CXXFLAGS="$COMPILER_FLAGS"
    OBJCFLAGS="$COMPILER_FLAGS"
    OBJCXXFLAGS="$COMPILER_FLAGS"
    LDFLAGS="$ARCH_FLAGS"
)

configure_OPTIONS+=(--prefix="$BUILT_PRODUCTS_DIR")

[[ ${PLATFORM_NAME} == iphoneos ]] && configure_OPTIONS+=(--with-chip-project-includes="$CHIP_ROOT"/config/ios)

configure_OPTIONS+=(
    --target="$target"
    --host="$target"
    lt_cv_ld_exported_symbols_list=yes
    ac_cv_func_clock_gettime=no
    ac_cv_have_decl_clock_gettime=no
    --with-logging-style=external
    --enable-cocoa
    --disable-docs
    --disable-java
    --disable-python
    --disable-shared
    --disable-tests
    --disable-tools
    --with-chip-system-project-includes=no
    --with-chip-inet-project-includes=no
    --with-chip-ble-project-includes=no
    --with-chip-warm-project-includes=no
    --with-chip-device-project-includes=no
    CROSS_TOP="$PLATFORM_DIR"
    CROSS_SDK="iPhoneOS$SDK_VERSION.sdk"
)

(
    cd "$TEMP_DIR"

    if [[ ! -x config.status ]]; then
        "$CHIP_ROOT"/bootstrap-configure -C "${configure_OPTIONS[@]}"
    else
        for makefile_am in "$(find "$CHIP_ROOT" -name Makefile.am)"; do
            [[ ${makefile_am} -ot ${makefile_am/.am/.in} ]] || {
                "$CHIP_ROOT"/bootstrap -w make
                break
            }
        done
    fi

    make install
)
