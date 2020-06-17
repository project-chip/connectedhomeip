#!/bin/bash

CHIP_ROOT="$(dirname "$0")"

_chip_red() {
  echo -e "\033[0;31m$*\033[0m"
}

_chip_yellow() {
  echo -e "\033[0;33m$*\033[0m"
}

_chip_banner() {
  _chip_yellow '.--------------------------------'
  _chip_yellow "-- $1"
  _chip_yellow "'--------------------------------"
}

_chip_banner "Environment bringup"

git -C "${CHIP_ROOT}" submodule update --init

source "${CHIP_ROOT}/activate.sh"

_chip_banner "Build: GN configure"

gn --root="${CHIP_ROOT}" gen --check "${CHIP_ROOT}/out/debug"
gn --root="${CHIP_ROOT}" gen --check "${CHIP_ROOT}/out/release" --args='is_debug=false'

_chip_banner "Build: Ninja build"

time ninja -C "${CHIP_ROOT}/out/debug" check

# Release build.
# time ninja -C "${CHIP_ROOT}/out/release" check

# Standalone example app.
# gn --root="${CHIP_ROOT}/examples/lock-app/nrf5" gen --check "${CHIP_ROOT}/out/nrf5_lock_app" --args=''
# time ninja -C "${CHIP_ROOT}/out/nrf5_lock_app"
