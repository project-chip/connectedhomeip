#!/usr/bin/env bash

set -x
env

make -j -f Makefile-bootstrap repos
source examples/wifi-echo/server/esp32/idf.sh
idf make -j V=1 -C examples/wifi-echo/server/esp32 defconfig
idf make -j V=1 -C examples/wifi-echo/server/esp32
