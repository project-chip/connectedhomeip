#!/usr/bin/env bash
#
# Copyright (c) 2022 Project CHIP Authors
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

# This script runs all targets taht are generating code
# in the given output directory
# CHIP_ROOT with a build environment activated.

set -e

OUT_DIR="$1"

if [ ! -d "$OUT_DIR" ]; then
    echo "Input directory '$OUT_DIR' does not exist. "
    echo "USAGE: $0 <build_dir>"
    exit 1
fi

# Code generation for build config files and asn1. Captures things like:
#
# gen_additional_data_payload_buildconfig
# gen_app_buildconfig
# gen_asn1oid
# gen_ble_buildconfig
# ...
#
# Most are buildconfig rules, but asn1oid is special
for name in $(ninja -C "$OUT_DIR" -t targets | grep -E '^gen_' | sed 's/: .*//'); do
    echo "Generating $name ..."
    ninja -C "$OUT_DIR" "$name"
done

# Code generation (based on zap/matter)
for name in $(ninja -C "$OUT_DIR" -t targets | grep -E -v '_no_codegen:' | grep -E '_codegen:' | sed 's/: .*//'); do
    echo "Generating $name ..."
    ninja -C "$OUT_DIR" "$name"
done

# Linus targets: dbus generate hdeaders
for name in $(ninja -C "$OUT_DIR" -t targets | grep -E 'dbus.*codegen:' | sed 's/: .*//'); do
    echo "Generating $name ..."
    ninja -C "$OUT_DIR" "$name"
done

# TLV decoding metadata
for name in $(ninja -C "$OUT_DIR" -t targets | grep -E '_generate' | sed 's/: .*//'); do
    echo "Generating $name ..."
    ninja -C "$OUT_DIR" "$name"
done
