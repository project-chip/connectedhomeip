#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
