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
TIZEN_SDK_DATA_PATH=~/tizen-sdk-data
TIZEN_VERSION=6.0
SECRET_TOOL=false

SCRIPT_NAME=$(basename -- "$(readlink -f "${BASH_SOURCE:?}")")
SCRIPT_DIR=$(dirname -- "$(readlink -f "${BASH_SOURCE:?}")")
DEPENDENCIES=('cpio' 'openjdk-8-jre-headless' 'obs-build' 'wget' 'zip')

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
    echo "Example: $SCRIPT_NAME --tizen-sdk-path ~/tizen-sdk --tizen-version 6.0 --install-dependencies"
    echo
    echo "Options:"
    echo "  -h, --help                 Display this information"
    echo "  --tizen-sdk-path           Set directory where Tizen will be installed. Default is $TIZEN_SDK_ROOT"
    echo "  --tizen-sdk-data-path      Set directory where Tizen have data. Default is $TIZEN_SDK_DATA_PATH"
    echo "  --install-dependencies     This options install all dependencies."
    echo "  --tizen-version            Select Tizen version. Default is $TIZEN_VERSION"
    echo "  --override-secret-tool     Circumvent the requirement of having functional D-Bus Secrets service."
    echo
    echo "Note:"
    echo "The script should run fully with ubuntu. For other distributions you may have to manually"
    echo "install all needed dependencies. Use the script specifying --tizen-sdk-path with or"
    echo "without --tizen-version. The script will only install Tizen platform for Matter."
}

# ------------------------------------------------------------------------------
# Error print function
function error() {
    echo "$COLOR_RED[ERROR]: $1$COLOR_NONE"
}

# ------------------------------------------------------------------------------
# Info print function
function info() {
    echo "$COLOR_GREEN[INFO]: $1$COLOR_NONE"
}

# ------------------------------------------------------------------------------
# Warning print function
function warning() {
    echo "$COLOR_YELLOW[WARNING]: $1$COLOR_NONE"
}

# ------------------------------------------------------------------------------
# Show dependencies
function show_dependencies() {
    warning "Need dependencies for use this script installation SDK: cpio unrpm unzip wget"
    warning "Need dependencies for Tizen SDK: JAVA JRE >=8.0"
}

# ------------------------------------------------------------------------------
# Function helper massive download
# Usage: download "url_dir_package" ${package_array[@]}
function download() {
    echo "$COLOR_BLUE"

    PKGS=()
    for PKG in "${@:2}"; do
        PKGS+=("-A" "$PKG")
    done
    wget -r -nd --no-parent --progress=dot:mega "${PKGS[@]}" "$1"

    # Check if the files have been downloaded
    for PKG in "${@:2}"; do
        if [[ ! $(find . -type f -name "$PKG") ]]; then
            error "PKG is missing: $PKG"
            return 1
        fi
    done

    echo -n "$COLOR_NONE"
}

# ------------------------------------------------------------------------------
# Function install all dependencies.
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
# Function clean on EXIT
function cleanup() {
    rm -rf "${TMP_DIR:?}"
}

# ------------------------------------------------------------------------------
# Function install tizen sdk.
function install_tizen_sdk() {

    mkdir -p "$TIZEN_SDK_ROOT" || return

    info "Tizen SDK installation directory: $TIZEN_SDK_ROOT"

    TIZEN_SDK_SYSROOT="$TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/mobile/rootstraps/mobile-$TIZEN_VERSION-device.core"

    cd "$TMP_DIR" || return

    # Get Tizen Studio CLI
    info "Downloading Tizen Studio CLI..."

    # Download
    URL="http://download.tizen.org/sdk/tizenstudio/official/binary/"
    PKG_ARR=(
        'certificate-encryptor_1.0.7_ubuntu-64.zip'
        'certificate-generator_0.1.3_ubuntu-64.zip'
        'new-common-cli_2.5.7_ubuntu-64.zip'
        'new-native-cli_2.5.7_ubuntu-64.zip'
        'sdb_4.2.23_ubuntu-64.zip')
    download "$URL" "${PKG_ARR[@]}"

    # Get toolchain
    info "Downloading Tizen toolchain..."

    # Download
    URL="http://download.tizen.org/sdk/tizenstudio/official/binary/"
    PKG_ARR=(
        "cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip"
        "sbi-toolchain-gcc-9.2.cpp.app_2.2.16_ubuntu-64.zip")
    download "$URL" "${PKG_ARR[@]}"

    # Get Tizen sysroot
    info "Downloading Tizen sysroot..."

    # Base sysroot
    # Different versions of Tizen have different rootstrap versions
    URL="http://download.tizen.org/sdk/tizenstudio/official/binary/"
    PKG_ARR=(
        "mobile-$TIZEN_VERSION-core-add-ons_*_ubuntu-64.zip"
        "mobile-$TIZEN_VERSION-rs-device.core_*_ubuntu-64.zip")
    download "$URL" "${PKG_ARR[@]}"

    # Base packages
    URL="http://download.tizen.org/releases/milestone/tizen/base/latest/repos/standard/packages/armv7l/"
    PKG_ARR=(
        'iniparser-*.armv7l.rpm'
        'libblkid-devel-*.armv7l.rpm'
        'libcap-*.armv7l.rpm'
        'libffi-devel-*.armv7l.rpm'
        'liblzma-*.armv7l.rpm'
        'libmount-devel-*.armv7l.rpm'
        'libncurses6-*.armv7l.rpm'
        'libreadline-*.armv7l.rpm'
        'libuuid-*.armv7l.rpm'
        'pcre-devel-*.armv7l.rpm'
        'readline-devel-*.armv7l.rpm'
        'xdgmime-*.armv7l.rpm')
    download "$URL" "${PKG_ARR[@]}"

    # Unified packages
    URL="http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/"
    PKG_ARR=(
        'aul-0*.armv7l.rpm'
        'aul-devel-*.armv7l.rpm'
        'bundle-0*.armv7l.rpm'
        'bundle-devel-*.armv7l.rpm'
        'buxton2-*.armv7l.rpm'
        'cynara-devel-*.armv7l.rpm'
        'dbus-1*.armv7l.rpm'
        'dbus-devel-*.armv7l.rpm'
        'dbus-libs-1*.armv7l.rpm'
        'glib2-devel-2*.armv7l.rpm'
        'json-glib-devel-*.armv7l.rpm'
        'libcynara-client-*.armv7l.rpm'
        'libcynara-commons-*.armv7l.rpm'
        'libdns_sd-*.armv7l.rpm'
        'libjson-glib-*.armv7l.rpm'
        'libsessiond-0*.armv7l.rpm'
        'libsystemd-*.armv7l.rpm'
        'libtzplatform-config-*.armv7l.rpm'
        'parcel-0*.armv7l.rpm'
        'parcel-devel-*.armv7l.rpm'
        'pkgmgr-info-*.armv7l.rpm'
        'vconf-compat-*.armv7l.rpm'
        'vconf-internal-keys-devel-*.armv7l.rpm')
    download "$URL" "${PKG_ARR[@]}"

    # Unified packages (snapshots)
    URL="http://download.tizen.org/snapshots/tizen/unified/latest/repos/standard/packages/armv7l/"
    PKG_ARR=(
        'capi-network-nsd-*.armv7l.rpm'
        'capi-network-thread-*.armv7l.rpm'
        'libnsd-dns-sd-*.armv7l.rpm')
    download "$URL" "${PKG_ARR[@]}"

    # Install all
    info "Installing Tizen SDK..."

    unzip -o '*.zip'
    cp -rf data/* "$TIZEN_SDK_ROOT"

    unrpm *.rpm
    cp -rf lib usr "$TIZEN_SDK_SYSROOT"

    # Install secret tool or not
    if ("$SECRET_TOOL"); then
        info "Overriding secret tool..."
        cp "$SCRIPT_DIR/secret-tool.py" "$TIZEN_SDK_ROOT/tools/certificate-encryptor/secret-tool"
        chmod 0755 "$TIZEN_SDK_ROOT/tools/certificate-encryptor/secret-tool"
    fi

    # Configure Tizen CLI
    echo "TIZEN_SDK_INSTALLED_PATH=$TIZEN_SDK_ROOT" >"$TIZEN_SDK_ROOT/sdk.info"
    echo "TIZEN_SDK_DATA_PATH=$TIZEN_SDK_DATA_PATH" >>"$TIZEN_SDK_ROOT/sdk.info"
    ln -sf "$TIZEN_SDK_DATA_PATH/.tizen-cli-config" "$TIZEN_SDK_ROOT/tools/.tizen-cli-config"

    # Make symbolic links relative
    find "$TIZEN_SDK_SYSROOT/usr/lib" -maxdepth 1 -type l | while IFS= read -r LNK; do
        ln -sf "$(basename "$(readlink "$LNK")")" "$LNK"
    done
    ln -sf ../../lib/libcap.so.2 "$TIZEN_SDK_SYSROOT/usr/lib/libcap.so"
    ln -sf openssl1.1.pc "$TIZEN_SDK_SYSROOT/usr/lib/pkgconfig/openssl.pc"

    info "Done."
    echo

    # Information on necessary environment variables
    warning "Before proceeding with Matter export environment variables as follows:"
    echo -n "$COLOR_YELLOW"
    echo "export TIZEN_VESRSION=\"$TIZEN_VERSION\""
    echo "export TIZEN_SDK_ROOT=\"$(realpath "$TIZEN_SDK_ROOT")\""
    echo "export TIZEN_SDK_TOOLCHAIN=\"\$TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-9.2\""
    echo "export TIZEN_SDK_SYSROOT=\"\$TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/mobile/rootstraps/mobile-$TIZEN_VERSION-device.core\""
    echo "export PATH=\"\$TIZEN_SDK_TOOLCHAIN/bin:\$TIZEN_SDK_ROOT/tools/ide/bin:\$TIZEN_SDK_ROOT/tools:\$PATH\""
    echo -n "$COLOR_NONE"
}

while (($#)); do
    case $1 in
        -h | --help)
            show_help
            exit 0
            ;;
        --tizen-sdk-path)
            TIZEN_SDK_ROOT="$2"
            shift
            ;;
        --tizen-sdk-data-path)
            TIZEN_SDK_DATA_PATH="$2"
            shift
            ;;
        --tizen-version)
            TIZEN_VERSION=$2
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
# Prepare a temporary directory and cleanup
trap cleanup EXIT
TMP_DIR=$(mktemp -d)
info "Created tmp directory $TMP_DIR"

# ------------------------------------------------------------------------------
# Checks if the user need install dependencies
if [ "$INSTALL_DEPENDENCIES" = true ]; then
    if ! install_dependencies; then
        error "Cannot install dependencies, please use this script as sudo user or root. Use --help"
        show_dependencies
        exit 1
    fi
fi

# ------------------------------------------------------------------------------
# Checking dependencies needed to install Tizen platform
info "Checking required tools: cpio, java, unrpm, unzip, wget"
for PKG in 'cpio' 'java' 'unrpm' 'unzip' 'wget'; do
    if ! command -v "$PKG" &>/dev/null; then
        error "Required tool not found: $PKG"
        dep_lost=1
    fi
done
if [[ $dep_lost ]]; then
    echo "[HINT]: sudo apt-get install ${DEPENDENCIES[*]}"
    exit 1
fi

# ------------------------------------------------------------------------------
# Installation Tizen SDK
install_tizen_sdk
