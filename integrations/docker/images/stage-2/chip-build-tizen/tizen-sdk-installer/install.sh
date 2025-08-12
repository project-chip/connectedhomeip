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

# Default settings options
TIZEN_SDK_ROOT=/opt/tizen-sdk
TIZEN_SDK_DATA_PATH=$HOME/tizen-sdk-data
TIZEN_VERSION=9.0
unset SECRET_TOOL
unset PURGE_PKG_CACHE PKG_CACHE_PATH
unset VERBOSE
DEBUG=${DEBUG:-}

[ -n "$DEBUG" ] && set -x
[ -n "$DEBUG" ] && VERBOSE=true

SCRIPT_NAME=$(basename -- "$(readlink -f "${BASH_SOURCE:?}")")
SCRIPT_DIR=$(dirname -- "$(readlink -f "${BASH_SOURCE:?}")")

DEPENDENCIES=('7zip' 'cpio' 'openjdk-8-jre-headless' 'zip' 'wget')
# Commands used to check the presence of the above dependencies
DEPENCENCIES_CMDS=('7z' 'cpio' 'java' 'unzip' 'wget')

# If color is available use colors
if which tput >/dev/null 2>&1 && [[ $(tput -T $TERM colors) -ge 8 ]]; then
    COLOR_NONE="$(tput sgr0)"
    COLOR_RED="$(tput setaf 1)"
    COLOR_GREEN="$(tput setaf 2)"
    COLOR_YELLOW="$(tput setaf 3)"
    COLOR_BLUE="$(tput setaf 4)"
fi

# ------------------------------------------------------------------------------
# Help display function
function show_help() {
    echo "Usage: $SCRIPT_NAME [ options .. ]"
    echo "Example: $SCRIPT_NAME --tizen-sdk-path ~/tizen-sdk --tizen-version 9.0 --install-dependencies"
    echo
    echo "Options:"
    echo "  -h, --help                 Display this information"
    echo "  --verbose                  Verbose output"
    echo "  --cpu                      Comma separated list of CPU architectures. Like arm or arm64"
    echo "  --tizen-sdk-path           Set directory for Tizen SDK installation. Default is $TIZEN_SDK_ROOT"
    echo "  --tizen-sdk-data-path      Set directory for Tizen SDK runtime data. Default is $TIZEN_SDK_DATA_PATH"
    echo "  --purge-pkg-cache          Purge package cache before download and installation"
    echo "  --pkg-cache-path           Set the Tizen SDK package cache path, default is ${XDG_CACHE_HOME:-$HOME/.cache}/tizen-pkgs/$TIZEN_VERSION"
    echo "  --install-dependencies     This option installs all required dependencies"
    echo "  --tizen-version            Select Tizen version. Default is $TIZEN_VERSION"
    echo "  --override-secret-tool     Circumvent the requirement of having functional D-Bus Secrets service"
    echo
    echo "Note:"
    echo "This script does not install full Tizen SDK. It installs only the necessary"
    echo "parts for Matter SDK to be able to build Tizen applications."
    echo "The option '--install-dependencies' should be able to install all required"
    echo "dependencies on any Debian-based distribution. For other distributions the"
    echo "dependencies should be installed manually."
}

# ------------------------------------------------------------------------------
# Error print function
function error() {
    echo "$COLOR_RED[ERROR]: $@$COLOR_NONE"
}

# ------------------------------------------------------------------------------
# Info print function
function info() {
    echo "$COLOR_GREEN[INFO]: $@$COLOR_NONE"
}

# ------------------------------------------------------------------------------
# Warning print function
function warning() {
    echo "$COLOR_YELLOW[WARNING]: $@$COLOR_NONE"
}

# ------------------------------------------------------------------------------
# Show dependencies
function show_dependencies() {
    warning "Required dependencies for Tizen SDK installation: 7z cpio unzip wget"
    warning "Required dependencies for Tizen SDK: JAVA JRE >=8.0"
    warning "To install them on Debian or Ubuntu run: sudo apt install ${DEPENDENCIES[*]}"
}

# ------------------------------------------------------------------------------
# Helper function for downloading packages.
# Usage: download "url_dir_package" ${package_array[@]}
function download() {
    echo "$COLOR_BLUE"
    local url="$1"
    shift

    local pkg pkgs=()
    for pkg in "${@}"; do
        pkgs+=("-A" "$pkg")
    done

    # Skip downloading if no packages are specified
    [[ ${#pkgs[@]} -eq 0 ]] && return

    local verbose_flag="-nv"
    [ -n "$VERBOSE" ] && verbose_flag="-v"
    wget -r "$verbose_flag" -nd --timestamping --no-parent -e robots=off --progress=dot:mega -P "$PKG_CACHE_PATH" "${pkgs[@]}" "$url"

    # Check if the files have been downloaded
    for pkg in "${@}"; do
        if [[ ! $(find "$PKG_CACHE_PATH" -type f -name "$pkg") ]]; then
            error "PKG is missing: $pkg"
            return 1
        fi
    done

    echo -n "$COLOR_NONE"
}

# ------------------------------------------------------------------------------
# Function for installing all dependencies.
function install_dependencies() {
    if ! command -v apt-get &>/dev/null; then
        show_dependencies
        error "Cannot install dependencies - apt package manager not available. Install dependencies manually."
        return 1
    fi

    info "Installing dependencies"
    apt-get update
    DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends "${DEPENDENCIES[@]}" || return
}

# ------------------------------------------------------------------------------
# Function for unpacking RPM filesets. Implements identical logic like the
# unzip_globs function below wrt. globs matching multiple RPMs.
function unrpm_globs() {
    local destdir="$1"
    local fn_glob
    local -n globs="$2"
    for fn_glob in "${globs[@]}"; do
        info "Processing filenames matching '$fn_glob' ..."

        local most_recent_filename="$(find "$PKG_CACHE_PATH" -name "$fn_glob" -printf '%T@ %p\n' |
            sort -nr | head -n 1 | cut -d ' ' -f 2)"

        info "Unpacking '$most_recent_filename' into '$destdir'"
        7z x -so "$most_recent_filename" | cpio --directory="$destdir" -idmu "${VERBOSE:+-v}"
    done
}
# ------------------------------------------------------------------------------
# Function for unpacking ZIP filesets. A fileset specifies a number of filename
# globs. In a situation where multiple zip files matching the same glob are
# present, for example:
# $ ls -l cache/tizen-9.0-core-add-ons_*_ubuntu-64.zip
# -rw-rw-r-- 1 m.grela m.grela 461718 Nov  4  2024 cache/tizen-9.0-core-add-ons_0.0.341_ubuntu-64.zip
# -rw-rw-r-- 1 m.grela m.grela 461722 Apr 16 01:45 cache/tizen-9.0-core-add-ons_0.0.358_ubuntu-64.zip
#
# the function attempts to find the most recent version of a particular zip file.
#
# The function assumes that every glob matches at lease one filename.
function unzip_globs() {
    local destdir="$1"
    local fn_glob
    local -n globs="$2"
    for fn_glob in "${globs[@]}"; do
        info "Processing filenames matching '$fn_glob' ..."

        local most_recent_filename="$(find "$PKG_CACHE_PATH" -name "$fn_glob" -printf '%T@ %p\n' |
            sort -nr | head -n 1 | cut -d ' ' -f 2)"

        info "Unpacking '$most_recent_filename' into '$destdir'"

        # Shellharden puts quotes around ${VERBOSE:+-v} which causes an empty
        # argv item to be passed to unzip when $VERBOSE is unset.
        # unzip does not like that and bails out.
        if [ -n "$VERBOSE" ]; then
            unzip "$most_recent_filename" 'data/*' -d "$destdir"
        else
            unzip -q "$most_recent_filename" 'data/*' -d "$destdir"
        fi
    done
}

# ------------------------------------------------------------------------------
# Function for cleaning up temporary files on exit.
function cleanup() {
    rm "$TIZEN_SDK_ROOT/data" || true
}

# Make symbolic links relative
function fixup_symlinks() {
    while [ "$1" ]; do
        local path="$1"
        shift
        [ -d "$path" ] || continue

        find "$path" -maxdepth 1 -type l | while IFS= read -r lnk; do
            ln -sf "$(basename "$(readlink "$lnk")")" "$lnk"
        done
    done
}

function download_tizen_sdk_common() {
    # Get Tizen Studio CLI
    info "Downloading Tizen Studio CLI..."
    local url

    # Download
    url="http://download.tizen.org/sdk/tizenstudio/official/binary/"
    download "$url" "${COMMON_TIZENSTUDIO_ZIPS[@]}" "${COMMON_TOOLCHAIN_ZIPS[@]}"

    # Tizen Developer Platform Certificate
    url="http://download.tizen.org/sdk/extensions/Tizen_IoT_Headless/binary/"
    # Tizen site does not have this package available in version 8.0.
    # Certificates are the same for 7.0 and 8.0, though.
    download "$url" "${IOT_ZIPS[@]}"
}

# ------------------------------------------------------------------------------
# Function for installing common packages for Tizen SDK.
function install_tizen_sdk_common() {

    info "Tizen SDK installation directory: $TIZEN_SDK_ROOT"

    info "Installing Tizen Studio CLI..."

    unzip_globs "$TIZEN_SDK_ROOT" COMMON_TIZENSTUDIO_ZIPS

    info "Installing common toolchain files ..."
    unzip_globs "$TIZEN_SDK_ROOT" COMMON_TOOLCHAIN_ZIPS

    info "Installing common IoT files ..."
    unzip_globs "$TIZEN_SDK_ROOT" IOT_ZIPS
}

# Take care to provide separate globs for binary and devel RPMs
# The "most-recent-file" detection logic will eat unsuspecting packages
# when for example capi-network-thread-*.armv7l.rpm matches both
# binary and devel RPM files. For example:
# ls -l cache/capi-network-thread-*.armv7l.rpm
# -rw-rw-r-- 1 m.grela m.grela 55133 Oct 30  2024 cache/capi-network-thread-0.9.0-2.armv7l.rpm
# -rw-rw-r-- 1 m.grela m.grela 17377 Oct 30  2024 cache/capi-network-thread-devel-0.9.0-2.armv7l.rpm
# -rw-rw-r-- 1 m.grela m.grela 25965 Oct 30  2024 cache/capi-network-thread-test-0.9.0-2.armv7l.rpm

TIZEN_SDK_BASE_RPMS=(
    'iniparser-[0-9]*'
    'iniparser-devel-*'
    'libblkid-devel-*'
    'libcap-[0-9]*'
    'libcap-devel-*'
    'libffi-devel-*'
    'liblzma-[0-9]*'
    'libmount-devel-*'
    'libncurses6-[0-9]*'
    'libreadline-[0-9]*'
    'libuuid-[0-9]*'
    'libuuid-devel-*'
    'pcre-devel-*'
    'readline-devel-*'
    'xdgmime-[0-9]*'
    'xdgmime-devel-*'
)

# this could have been a function but shellharden does not allow us to
declare -a TIZEN_SDK_ARM_BASE_RPMS TIZEN_SDK_ARM64_BASE_RPMS
for rpm in "${TIZEN_SDK_BASE_RPMS[@]}"; do
    TIZEN_SDK_ARM_BASE_RPMS+=("$rpm.armv7l.rpm")
    TIZEN_SDK_ARM64_BASE_RPMS+=("$rpm.aarch64.rpm")
done

TIZEN_SDK_UNIFIED_RPMS=(
    'app-core-common-[0-9]*'
    'app-core-common-devel-*'
    'aul-[0-9]*'
    'aul-devel-*'
    'bluetooth-frwk-0*'
    'bluetooth-frwk-devel-*'
    'bundle-0*'
    'bundle-devel-*'
    'buxton2-[0-9]*'
    'buxton2-devel-*'
    'capi-network-bluetooth-0*'
    'capi-network-bluetooth-devel-*'
    'capi-network-nsd-[0-9]*'
    'capi-network-nsd-devel-*'
    'capi-network-thread-[0-9]*'
    'capi-network-thread-devel-*'
    'capi-system-peripheral-io-[0-9]*'
    'capi-system-peripheral-io-devel-*'
    'capi-system-resource-1*'
    'capi-system-resource-devel-*'
    'cynara-devel-*'
    'dbus-1*'
    'dbus-devel-*'
    'dbus-libs-1*'
    'glib2-devel-2*'
    'pcre2-devel-[0-9]*'
    'hal-api-common-[0-9]*'
    'hal-api-common-devel-*'
    'hal-api-sensor-[0-9]*'
    'hal-api-sensor-devel-*'
    'json-glib-devel-*'
    'libcynara-client-[0-9]*'
    'libcynara-commons-[0-9]*'
    'libdns_sd-[0-9]*'
    'libjson-glib-[0-9]*'
    'libnsd-dns-sd-[0-9]*'
    'libsessiond-0*'
    'libsystemd-[0-9]*'
    'libtzplatform-config-[0-9]*'
    'libtzplatform-config-devel-*'
    'parcel-0*'
    'parcel-devel-*'
    'pkgmgr-info-[0-9]*'
    'pkgmgr-info-devel-*'
    'sensord-[0-9]*'
    'sensord-devel-*'
    'sensord-dummy-[0-9]*'
    'vconf-compat-[0-9]*'
    'vconf-compat-devel*'
    'vconf-internal-keys-devel-*'
)

for rpm in "${TIZEN_SDK_UNIFIED_RPMS[@]}"; do
    TIZEN_SDK_ARM_UNIFIED_RPMS+=("$rpm.armv7l.rpm")
    TIZEN_SDK_ARM64_UNIFIED_RPMS+=("$rpm.aarch64.rpm")
done

function download_tizen_sdk_arm() {
    # Get toolchain
    info "Downloading Tizen ARM toolchain and sysroot ..."
    local url

    # Download
    url="http://download.tizen.org/sdk/tizenstudio/official/binary/"
    download "$url" "${SDK_ARM_TIZENSTUDIO_ZIPS[@]}"

    # Base packages
    url="http://download.tizen.org/releases/milestone/TIZEN/Tizen-$TIZEN_VERSION/Tizen-$TIZEN_VERSION-Base/latest/repos/standard/packages/armv7l/"
    download "$url" "${TIZEN_SDK_ARM_BASE_RPMS[@]}"

    # Unified packages
    url="http://download.tizen.org/releases/milestone/TIZEN/Tizen-$TIZEN_VERSION/Tizen-$TIZEN_VERSION-Unified/latest/repos/standard/packages/armv7l/"
    download "$url" "${TIZEN_SDK_ARM_UNIFIED_RPMS[@]}"
}

# Function for installing Tizen SDK (armv7l).
function install_tizen_sdk_arm() {

    local sysroot="$TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/tizen/rootstraps/tizen-$TIZEN_VERSION-device.core"

    info "Installing Tizen ARM SDK..."

    unzip_globs "$TIZEN_SDK_ROOT" SDK_ARM_TIZENSTUDIO_ZIPS

    info "Installing Tizen ARM sysroot..."

    unrpm_globs "$sysroot" TIZEN_SDK_ARM_BASE_RPMS
    unrpm_globs "$sysroot" TIZEN_SDK_ARM_UNIFIED_RPMS

    fixup_symlinks "$sysroot"/usr/{lib,lib64}

    # TODO: Is this required???
    ln -sf openssl3.pc "$sysroot/usr/lib/pkgconfig/openssl.pc"
}

function download_tizen_sdk_arm64() {
    info "Downloading Tizen ARM64 toolchain and sysroot ..."
    local url

    # Download
    url="http://download.tizen.org/sdk/tizenstudio/official/binary/"
    download "$url" "${SDK_ARM64_TIZENSTUDIO_ZIPS[@]}"

    # Base packages
    url="http://download.tizen.org/releases/milestone/TIZEN/Tizen-$TIZEN_VERSION/Tizen-$TIZEN_VERSION-Base/latest/repos/standard/packages/aarch64/"
    download "$url" "${TIZEN_SDK_ARM64_BASE_RPMS[@]}"

    # Unified packages
    url="http://download.tizen.org/releases/milestone/TIZEN/Tizen-$TIZEN_VERSION/Tizen-$TIZEN_VERSION-Unified/latest/repos/standard/packages/aarch64/"
    download "$url" "${TIZEN_SDK_ARM64_UNIFIED_RPMS[@]}"
}

# Function for installing Tizen SDK (arm64).
function install_tizen_sdk_arm64() {

    local sysroot="$TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/tizen/rootstraps/tizen-$TIZEN_VERSION-device64.core"

    info "Installing Tizen ARM64 SDK..."

    unzip_globs "$TIZEN_SDK_ROOT" SDK_ARM64_TIZENSTUDIO_ZIPS

    info "Installing Tizen ARM64 sysroot..."

    unrpm_globs "$sysroot" TIZEN_SDK_ARM64_BASE_RPMS
    unrpm_globs "$sysroot" TIZEN_SDK_ARM64_UNIFIED_RPMS

    fixup_symlinks "$sysroot"/usr/{lib,lib64}

    # TODO: Is this required???
    ln -sf openssl3.pc "$sysroot/usr/lib64/pkgconfig/openssl.pc"
}

function install_tizen_sdk_finalize() {

    # Install secret tool or not
    if [ -n "$SECRET_TOOL" ]; then
        info "Overriding secret tool..."
        install "$SCRIPT_DIR/secret-tool.py" "$TIZEN_SDK_ROOT/tools/certificate-encryptor/secret-tool"
    fi

    # Configure Tizen CLI
    echo "TIZEN_SDK_INSTALLED_PATH=$TIZEN_SDK_ROOT" >"$TIZEN_SDK_ROOT/sdk.info"
    echo "TIZEN_SDK_DATA_PATH=$TIZEN_SDK_DATA_PATH" >>"$TIZEN_SDK_ROOT/sdk.info"
    ln -sf "$TIZEN_SDK_DATA_PATH/.tizen-cli-config" "$TIZEN_SDK_ROOT/tools/.tizen-cli-config"

    # Use Tizen developer platform certificate as defaults
    cp "$TIZEN_SDK_ROOT"/tools/certificate-generator/certificates/distributor/sdk-platform/* \
        "$TIZEN_SDK_ROOT"/tools/certificate-generator/certificates/distributor/

    info "Done."
    echo

    warning "Before proceeding with Matter export environment variables as follows:"
    echo -n "$COLOR_YELLOW"
    echo "export TIZEN_VERSION=\"$TIZEN_VERSION\""
    echo "export TIZEN_SDK_ROOT=\"$(realpath "$TIZEN_SDK_ROOT")\""

    if [ "$INSTALL_ARM" = true ]; then
        echo "export TIZEN_SDK_TOOLCHAIN=\"\$TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-9.2\""
        echo "export TIZEN_SDK_SYSROOT=\"\$TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/tizen/rootstraps/tizen-$TIZEN_VERSION-device.core\""
        echo "export PATH=\"\$TIZEN_SDK_TOOLCHAIN/bin:\$TIZEN_SDK_ROOT/tools/ide/bin:\$TIZEN_SDK_ROOT/tools:\$PATH\""
    fi

    if [ "$INSTALL_ARM64" = true ]; then
        echo "export TIZEN_SDK_TOOLCHAIN_ARM64=\"\$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gcc-9.2\""
        echo "export TIZEN_SDK_SYSROOT_ARM64=\"\$TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/tizen/rootstraps/tizen-$TIZEN_VERSION-device64.core\""
        echo "export PATH=\"\$TIZEN_SDK_TOOLCHAIN_ARM64/bin:\$TIZEN_SDK_ROOT/tools/ide/bin:\$TIZEN_SDK_ROOT/tools:\$PATH\""
    fi

    echo -n "$COLOR_NONE"
}

while (($#)); do
    case $1 in
        -h | --help)
            show_help
            exit 0
            ;;
        --verbose)
            VERBOSE=true
            ;;
        --cpu)
            IFS=',' read -r -a array <<<"$2"
            for cpu in "${array[@]}"; do
                if [ "$cpu" == "arm" ]; then
                    INSTALL_ARM=true
                elif [ "$cpu" == "arm64" ]; then
                    INSTALL_ARM64=true
                else
                    error "Invalid CPU: $cpu. Use --help"
                    exit 1
                fi
            done
            shift
            ;;
        --tizen-sdk-path)
            TIZEN_SDK_ROOT="$2"
            shift
            ;;
        --tizen-sdk-data-path)
            TIZEN_SDK_DATA_PATH="$2"
            shift
            ;;
        --purge-pkg-cache)
            PURGE_PKG_CACHE=true
            ;;
        --pkg-cache-path)
            PKG_CACHE_PATH="$2"
            shift
            ;;
        --tizen-version)
            TIZEN_VERSION="$2"
            shift
            ;;
        --install-dependencies)
            INSTALL_DEPENDENCIES=true
            ;;
        --override-secret-tool)
            SECRET_TOOL=true
            ;;
        *)
            error "Wrong options usage!"
            exit 1
            ;;
    esac
    shift
done

# ------------------------------------------------------------------------------
# Verify passed arguments and flags
if [ -z "$INSTALL_ARM" ] && [ -z "$INSTALL_ARM64" ]; then
    error "No CPU architecture provided. Use --help."
    exit 1
fi

[ -z "$PKG_CACHE_PATH" ] && PKG_CACHE_PATH=${XDG_CACHE_HOME:-$HOME/.cache}/tizen-pkgs/$TIZEN_VERSION
info "Using package cache '$PKG_CACHE_PATH'"

[ ! -d "$PKG_CACHE_PATH" ] && mkdir -p "$PKG_CACHE_PATH"
if [ -n "$PURGE_PKG_CACHE" ]; then
    warning "Purging package cache in '$PKG_CACHE_PATH'"
    rm -rf "${VERBOSE:+-v}" "$PKG_CACHE_PATH"/*
fi

# ------------------------------------------------------------------------------
# Checks if the user need install dependencies
if [ "$INSTALL_DEPENDENCIES" = true ]; then
    if ! install_dependencies; then
        error "Cannot install dependencies, please use this script as sudo user or root."
        show_dependencies
        exit 1
    fi
fi
# ------------------------------------------------------------------------------
# Checking dependencies needed to install Tizen platform
info "Checking required tools: ${DEPENDENCIES_CMDS[@]}"
for cmd in "${DEPENDENCIES_CMDS[@]}"; do
    if ! command -v "$cmd" &>/dev/null; then
        error "Required tool not found: $cmd"
        show_dependencies
        exit 1
    fi
done

# ------------------------------------------------------------------------------
# Generate zip and RPM lists whose names are dependent upon $TIZEN_VERSION

COMMON_TIZENSTUDIO_ZIPS=(
    'certificate-encryptor_1.0.10_ubuntu-64.zip'
    'certificate-generator_0.1.4_ubuntu-64.zip'
    'new-common-cli_2.5.64_ubuntu-64.zip'
    'new-native-cli_2.5.64_ubuntu-64.zip'
    'sdb_4.2.23_ubuntu-64.zip'
    "tizen-$TIZEN_VERSION-core-add-ons_*_ubuntu-64.zip"
)

IOT_ZIPS=(
    "7.0-iot-things-add-ons_*_ubuntu-64.zip"
)

COMMON_TOOLCHAIN_ZIPS=(
    'sbi-toolchain-gcc-9.2.cpp.app_2.2.16_ubuntu-64.zip'
)

SDK_ARM_TIZENSTUDIO_ZIPS=(
    "cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip"
    # Base sysroot
    "tizen-$TIZEN_VERSION-rs-device.core_*_ubuntu-64.zip"
)

SDK_ARM64_TIZENSTUDIO_ZIPS=(
    "cross-aarch64-gcc-9.2_0.1.9_ubuntu-64.zip"
    # Base sysroot
    "tizen-$TIZEN_VERSION-rs-device64.core_*_ubuntu-64.zip"
)

# ------------------------------------------------------------------------------
# Installation Tizen SDK
mkdir -p "$TIZEN_SDK_ROOT"

if [ -e "$TIZEN_SDK_ROOT"/data ]; then
    error "'$TIZEN_SDK_ROOT' is not empty, bailing out"
    exit 1
fi
# Trick unzip into junking the first path component:
# Then unzip -o '*.zip' 'data/*' -d "$TIZEN_SDK_ROOT" creates:
# $TIZEN_SDK_ROOT/platforms/*
# not
# $TIZEN_SDK_ROOT/data/platforms/*
# Kudos: https://askubuntu.com/a/1088207
(
    cd "$TIZEN_SDK_ROOT"
    [ -L data ] || ln -s . data
)

download_tizen_sdk_common
[ "$INSTALL_ARM" = "true" ] && download_tizen_sdk_arm
[ "$INSTALL_ARM64" = "true" ] && download_tizen_sdk_arm64

install_tizen_sdk_common
[ "$INSTALL_ARM" = "true" ] && install_tizen_sdk_arm
[ "$INSTALL_ARM64" = "true" ] && install_tizen_sdk_arm64

install_tizen_sdk_finalize

[ -L "$TIZEN_SDK_ROOT/data" ] && rm "$TIZEN_SDK_ROOT/data"
