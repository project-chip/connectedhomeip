#!/bin/bash
#
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

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
