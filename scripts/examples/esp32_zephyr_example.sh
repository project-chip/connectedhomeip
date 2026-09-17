#!/usr/bin/env bash

#
#    Copyright (c) 2026 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Builds an ESP32 Zephyr example (examples/<application>/esp32/zephyr) with west.
#
# Usage: scripts/examples/esp32_zephyr_example.sh <application> <board> [west build args...]
#
# Environment:
#   ESP32_ZEPHYR_BASE     Zephyr tree of a west workspace with the hal_espressif
#                         blobs fetched. Falls back to ZEPHYR_BASE.
#   ESP32_ZEPHYR_SDK_DIR  Optional Zephyr SDK location.
#   ESP32_ZEPHYR_VENV     Optional Python 3.12+ venv providing west. Defaults to
#                         .venv in the west workspace root. Zephyr 4.4 rejects
#                         the Python 3.11 of the Matter build environment.
#
# The output lands in examples/<application>/esp32/zephyr/build.

set -e

cd "$(dirname "$0")/../.."

APP="$1"
BOARD="$2"
shift 2 || true

if [[ ! -f "examples/$APP/esp32/zephyr/CMakeLists.txt" || -z "$BOARD" ]]; then
    echo "Usage: $0 <application> <board> [west build args...]" >&2
    echo "Applications:" >&2
    ls examples/*/esp32/zephyr/CMakeLists.txt | awk -F/ '{print "  "$2}' >&2
    echo "Boards: esp32c6_devkitc/esp32c6/hpcore" >&2
    exit 1
fi

ZEPHYR_BASE="${ESP32_ZEPHYR_BASE:-${ZEPHYR_BASE:-}}"
if [[ -z "$ZEPHYR_BASE" ]]; then
    echo "Set ESP32_ZEPHYR_BASE (or ZEPHYR_BASE) to the Zephyr tree of your west workspace" >&2
    exit 1
fi
export ZEPHYR_BASE

if [[ -n "${ESP32_ZEPHYR_SDK_DIR:-}" ]]; then
    export ZEPHYR_SDK_INSTALL_DIR="$ESP32_ZEPHYR_SDK_DIR"
fi

VENV="${ESP32_ZEPHYR_VENV:-$ZEPHYR_BASE/../.venv}"
if [[ -x "$VENV/bin/west" ]]; then
    export PATH="$VENV/bin:$PATH"
fi

set -x
west build -p auto -b "$BOARD" -d "examples/$APP/esp32/zephyr/build" "examples/$APP/esp32/zephyr" "$@"
