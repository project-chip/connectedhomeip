#!/usr/bin/env bash

set -x
env

root=examples/wifi-echo/server/esp32/

make -j -f Makefile-bootstrap repos
source "$root"/idf.sh

SDKCONFIG_DEFAULTS=$root/sdkconfig_devkit.defaults idf make -j V=1 -C "$root" defconfig
idf make -j V=1 -C "$root" || {
    echo 'build DevKit-C failed'
    exit 1
}

SDKCONFIG_DEFAULTS=$root/sdkconfig_m5stack.defaults idf make -j V=1 -C "$root" defconfig
idf make -j V=1 -C "$root" || {
    echo 'build M5Stack failed'
    exit 1
}
