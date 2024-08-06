#!/usr/bin/env bash

CURRENT_DIR=$(
    cd "$(dirname "$0")"
    pwd
)

user=$(stat -c %U "$0")

SDK_ROOT=/opt/bouffalolab_sdk

echo "Please input path to install toolchain, or type Enter to install under $SDK_ROOT"
read TOOLCHAIN_SETUP_ROOT
if [[ ${TOOLCHAIN_SETUP_ROOT} == "" ]]; then
    TOOLCHAIN_SETUP_ROOT=$SDK_ROOT
fi
echo "Toolchain will install under $TOOLCHAIN_SETUP_ROOT"
flash_tool_postfix=

flash_tool=BouffaloLabDevCube-v1.9.0
flash_tool_url=https://dev.bouffalolab.com/media/upload/download/$flash_tool.zip
thead_toolchain=gcc_t-head_v2.6.1
thead_toolchain_url=https://codeload.github.com/bouffalolab/toolchain_gcc_t-head_linux/zip/c4afe91cbd01bf7dce525e0d23b4219c8691e8f0
thead_toolchain_unzip=toolchain_gcc_t-head_linux-c4afe91cbd01bf7dce525e0d23b4219c8691e8f0

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    toolchains_url=(
        "toolchain/riscv" "https://dev.bouffalolab.com/media/upload/download/toolchain_riscv_sifive_linux64.zip" Linux toolchain_riscv_sifive_linux
        "toolchain/t-head-riscv" "$thead_toolchain_url" "$thead_toolchain" "$thead_toolchain_unzip"
        "flashtool" "$flash_tool_url" "$flash_tool" ""
    )
    flash_tool_postfix=ubuntu
elif [[ "$OSTYPE" == "darwin"* ]]; then
    toolchains_url=(
        "toolchain/riscv/Darwin" "https://dev.bouffalolab.com/media/upload/download/toolchain_riscv_sifive_macos.zip"
    )
    flash_tool_postfix=macos
else
    echo "Not support for ""$OSTYPE"
fi

if [ ! -d "$TOOLCHAIN_SETUP_ROOT" ]; then
    mkdir -p "$TOOLCHAIN_SETUP_ROOT"
fi
rm -rf "$TOOLCHAIN_SETUP_ROOT"/*.zip

for ((i = 0; i < ${#toolchains_url[@]}; i += 4)); do
    path=${toolchains_url[i]}
    url=${toolchains_url[i + 1]}
    out=${toolchains_url[i + 2]}
    unzip_name=${toolchains_url[i + 3]}

    if [ -d "$TOOLCHAIN_SETUP_ROOT/$path/$out" ]; then
        continue
    fi
    rm -rf "$TOOLCHAIN_SETUP_ROOT/$path"
    mkdir -p "$TOOLCHAIN_SETUP_ROOT/$path"

    wget -P "$TOOLCHAIN_SETUP_ROOT"/ "$url"
    toolchain_zip=$(basename "$url")
    toolchain_zip=$(find "$TOOLCHAIN_SETUP_ROOT" -maxdepth 1 -name *"$toolchain_zip"*)
    toolchain_zip=$(basename "$toolchain_zip")
    if [ ! -f "$TOOLCHAIN_SETUP_ROOT/$toolchain_zip" ]; then
        exit 1
    fi

    unzip -q "$TOOLCHAIN_SETUP_ROOT/$toolchain_zip" -d "$TOOLCHAIN_SETUP_ROOT/$path/tmp"
    mv "$TOOLCHAIN_SETUP_ROOT/$path/tmp/$unzip_name" "$TOOLCHAIN_SETUP_ROOT/$path/$out"

    rm -rf "$TOOLCHAIN_SETUP_ROOT/$path"/tmp
    rm -rf "$TOOLCHAIN_SETUP_ROOT/$toolchain_zip"

    if [ -f "$TOOLCHAIN_SETUP_ROOT/$path"/"$out"/chmod755.sh ]; then
        cd "$TOOLCHAIN_SETUP_ROOT/$path"/"$out"
        bash chmod755.sh
        cd "$CURRENT_DIR"
    fi
done

chmod +x "$TOOLCHAIN_SETUP_ROOT/flashtool/$flash_tool/BLDevCube-$flash_tool_postfix"
chmod +x "$TOOLCHAIN_SETUP_ROOT/flashtool/$flash_tool/bflb_iot_tool-$flash_tool_postfix"

if [[ "$user" == "root" ]]; then
    chmod a+wr "$TOOLCHAIN_SETUP_ROOT/flashtool/$flash_tool" -R
else
    chown "$user" "$TOOLCHAIN_SETUP_ROOT/flashtool/$flash_tool"/ -R
fi
