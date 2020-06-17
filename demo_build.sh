#!/bin/bash

CHIP_ROOT="$(dirname "$0")"

git -C "${CHIP_ROOT}" submodule update --init

source "${CHIP_ROOT}/activate.sh"

gn --root="${CHIP_ROOT}" gen --check "${CHIP_ROOT}/out/debug"
gn --root="${CHIP_ROOT}" gen --check "${CHIP_ROOT}/out/release" --args='is_debug=false'

time ninja -C "${CHIP_ROOT}/out/debug" check

# Release build.
# time ninja -C "${CHIP_ROOT}/out/release" check

# Standalone example app.
# gn --root="${CHIP_ROOT}/examples/lock-app/nrf5" gen --check "${CHIP_ROOT}/out/nrf5_lock_app" --args=''
# time ninja -C "${CHIP_ROOT}/out/nrf5_lock_app"
