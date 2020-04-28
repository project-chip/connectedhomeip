#!/bin/bash

source examples/wifi-echo/esp32/idf.sh
idf make -C examples/wifi-echo/esp32 defconfig
idf make -C examples/wifi-echo/esp32
