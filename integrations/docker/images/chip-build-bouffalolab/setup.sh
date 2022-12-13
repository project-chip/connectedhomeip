#!/usr/bin/env bash

set -x
CURRENT_DIR=$(
    cd "$(dirname "$0")"
    pwd
)

SDK_ROOT=/opt/bouffalolab_sdk
# Currently, only setup toolchain under $SDK_ROOT
TOOLCHAIN_SETUP_ROOT=$SDK_ROOT/toolchain

TOOLCHAIN_SYMBOLIC_LINK_PATH=""
git -C . rev-parse 2>/dev/null
if [[ "$?" == "0" ]]; then
    # Examples in Bouffalo Lab IOT SDK repo expect toolchain under repo,
    # let's create a symbolic link to Bouffalo Lab toolchain,
    # if this script runs under repo
    TOOLCHAIN_SYMBOLIC_LINK_PATH=$CURRENT_DIR/../toolchain
fi

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    toolchains_url=(
        "riscv/Thead_riscv/Linux_x86_64" "https://dev.bouffalolab.com/media/upload/download/toolchain_riscv_thead_linux64.zip" "toolchain_riscv_thead_linux_x86_64"
        "riscv/Linux" "https://dev.bouffalolab.com/media/upload/download/toolchain_riscv_sifive_linux64.zip" "toolchain_riscv_sifive_linux"
    )
elif [[ "$OSTYPE" == "darwin"* ]]; then
    toolchains_url=(
        "riscv/Darwin" "https://dev.bouffalolab.com/media/upload/download/toolchain_riscv_sifive_macos.zip" "toolchain_riscv_sifive_macos"
    )
else
    echo "Not support for ""$OSTYPE"
fi

if [ ! -d "$TOOLCHAIN_SETUP_ROOT" ]; then
    mkdir -p "$TOOLCHAIN_SETUP_ROOT"
fi
rm -rf "$TOOLCHAIN_SETUP_ROOT"/*.zip

for ((i = 0; i < ${#toolchains_url[@]}; i += 3)); do
    path=${toolchains_url[i]}
    url=${toolchains_url[i + 1]}
    output=${toolchains_url[i + 2]}

    wget -P "$TOOLCHAIN_SETUP_ROOT"/ "$url"
    toolchain_zip=$(basename "$url")
    if [ ! -f "$TOOLCHAIN_SETUP_ROOT/$toolchain_zip" ]; then
        exit 1
    fi
    rm -rf "$TOOLCHAIN_SETUP_ROOT/$path"
    mkdir -p "$TOOLCHAIN_SETUP_ROOT/$path"
    unzip "$TOOLCHAIN_SETUP_ROOT/$toolchain_zip" -d "$TOOLCHAIN_SETUP_ROOT/$path"
    mv "$TOOLCHAIN_SETUP_ROOT/$path/$output"/* "$TOOLCHAIN_SETUP_ROOT/$path"
    rm -rf "$TOOLCHAIN_SETUP_ROOT/$toolchain_zip"

    if [ -f "$TOOLCHAIN_SETUP_ROOT/$path"/chmod755.sh ]; then
        cd "$TOOLCHAIN_SETUP_ROOT/$path"/
        bash chmod755.sh
        cd "$CURRENT_DIR"
    fi
done

if [[ "$TOOLCHAIN_SYMBOLIC_LINK_PATH" != "" ]]; then
    rm -rf "$TOOLCHAIN_SYMBOLIC_LINK_PATH"
    ln -s "$TOOLCHAIN_SETUP_ROOT" "$TOOLCHAIN_SYMBOLIC_LINK_PATH"
fi
