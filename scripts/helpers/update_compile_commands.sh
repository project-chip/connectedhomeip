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

# Update compilation database in VS Code devcontainer.

set -e

CHIP_ROOT="$(dirname "$0")/../.."

set +e
source "$CHIP_ROOT/scripts/activate.sh"
set -e

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=all_host_gcc
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.gcc.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=all_host_clang
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.clang.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=all_host_gcc_mbedtls
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.mbedtls.json"

gn --root="$CHIP_ROOT" gen "$CHIP_ROOT/out/debug" --export-compile-commands=nrf5_lock_app,nrf5_lighting_app
mv "$CHIP_ROOT/out/debug/compile_commands.json" "$CHIP_ROOT/out/debug/compile_commands.nrf5.json"
