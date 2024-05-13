#!/usr/bin/env bash
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

set -e

# Update compilation database in VS Code devcontainer.

CHIP_ROOT="$(dirname "$0")/../.."

source "$CHIP_ROOT/scripts/activate.sh"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=host_gcc
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.gcc.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=host_clang
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.clang.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=host_gcc_mbedtls
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.mbedtls.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=android_x64
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.android_x64.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=android_arm64
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.android_arm64.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=efr32_lock_app
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.efr32.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=tizen_lighting_app
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.tizen_arm.json"
