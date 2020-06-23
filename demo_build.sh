#!/bin/bash -e

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

set +e
source "${CHIP_ROOT}/activate.sh"
set -e

_chip_banner "Build: GN configure"

gn --root="${CHIP_ROOT}" gen --check "${CHIP_ROOT}/out/debug" --args='target_os="all"'
gn --root="${CHIP_ROOT}" gen --check "${CHIP_ROOT}/out/release" --args='target_os="all" is_debug=false'

_chip_banner "Build: Ninja build"

time ninja -C "${CHIP_ROOT}/out/debug" check

echo
echo 'To re-bootstrap build environment in your shell, run:'
echo source "${CHIP_ROOT}/bootstrap.sh"
echo

echo 'To activate existing build environment in your shell, run (do this first):'
echo source "${CHIP_ROOT}/activate.sh"

echo
echo 'To build a debug build:'
echo gn gen "${CHIP_ROOT}/out/debug" --args=\''target_os="all"'\'
echo ninja -C "${CHIP_ROOT}/out/debug"

echo
echo 'To run tests (idempotent):'
echo ninja -C "${CHIP_ROOT}/out/debug" check

echo
echo 'To build & test an optimized build:'
echo gn gen "${CHIP_ROOT}/out/release" --args=\''target_os="all" is_debug=false'\'
echo ninja -C "${CHIP_ROOT}/out/release" check

echo
echo 'To build a custom build (for help run "gn args --list out/debug")'
echo gn args "${CHIP_ROOT}/out/custom"
echo ninja -C "${CHIP_ROOT}/out/custom"

echo
echo 'To build the nRF5 lock sample as a standalone project':
echo "(cd ${CHIP_ROOT}/examples/lock-app/nrf5; gn gen out/debug; ninja -C out/debug)"
