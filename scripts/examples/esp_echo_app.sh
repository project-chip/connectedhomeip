#!/usr/bin/env bash

env

make -f Makefile-bootstrap repos
source examples/wifi-echo/server/esp32/idf.sh
idf make V=1 -C examples/wifi-echo/server/esp32 defconfig
idf make V=1 -C examples/wifi-echo/server/esp32
