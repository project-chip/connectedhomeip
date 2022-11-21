#!/usr/bin/env bash

set -x
CURRENT_DIR=$(cd "$(dirname "$0")";pwd)
SDK_ROOT_DIR=$CURRENT_DIR/../

TOOLCHAIN_SETUP_ROOT=~/.bouffalolab/toolchain
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    toolchains_url=(\
        "riscv/Thead_riscv/Linux_x86_64" "https://dev.bouffalolab.com/media/upload/download/toolchain_riscv_thead_linux64.zip" "toolchain_riscv_thead_linux_x86_64" \
        "riscv/Linux" "https://dev.bouffalolab.com/media/upload/download/toolchain_riscv_sifive_linux64.zip" "toolchain_riscv_sifive_linux" \
    )

elif [[ "$OSTYPE" == "darwin"* ]]; then
    toolchains_url=(\
        "riscv/Darwin" "https://dev.bouffalolab.com/media/upload/download/toolchain_riscv_sifive_macos.zip" "toolchain_riscv_sifive_macos"\
    )
else
    echo "Not support for "$OSTYPE
fi

if [ ! -d $TOOLCHAIN_SETUP_ROOT ]; then
    mkdir -p $TOOLCHAIN_SETUP_ROOT
fi
rm -rf $TOOLCHAIN_SETUP_ROOT/*.zip

for(( i=0; i<${#toolchains_url[@]}; i+=3)) do
    path=${toolchains_url[i]}
    url=${toolchains_url[i+1]}
    output=${toolchains_url[i+2]}

    wget -P $TOOLCHAIN_SETUP_ROOT/ $url
    toolchain_zip=`basename $url`
    if [ ! -f $TOOLCHAIN_SETUP_ROOT/$toolchain_zip ]; then
        exit 1
    fi
    rm -rf $TOOLCHAIN_SETUP_ROOT/$path 2>&1 > /dev/null
    mkdir -p $TOOLCHAIN_SETUP_ROOT/$path
    unzip $TOOLCHAIN_SETUP_ROOT/$toolchain_zip -d $TOOLCHAIN_SETUP_ROOT/$path 2>&1 > /dev/null
    mv $TOOLCHAIN_SETUP_ROOT/$path/$output/* $TOOLCHAIN_SETUP_ROOT/$path
    rm -rf $TOOLCHAIN_SETUP_ROOT/$toolchain_zip 2>&1 > /dev/null

    if [ -f $TOOLCHAIN_SETUP_ROOT/$path/chmod755.sh ]; then
        cd $TOOLCHAIN_SETUP_ROOT/$path/
        bash chmod755.sh
    fi
done

cd $SDK_ROOT_DIR
rm -rf toolchain
ln -s $TOOLCHAIN_SETUP_ROOT toolchain

cd $CURRENT_DIR
