#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

CHIP_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"/../../../../..
echo "$CHIP_ROOT"
INPUT_ZAP="$CHIP_ROOT/src/controller/data_model/controller-clusters.zap"
INPUT_TEMPLATE="$CHIP_ROOT/src/app/tests/suites/examples/templates/templates.json"
OUTPUT_DIR="$CHIP_ROOT/src/app/tests/suites/examples/out"

source "$CHIP_ROOT/scripts/activate.sh"

mkdir -p "$OUTPUT_DIR"

"$CHIP_ROOT"/scripts/tools/zap/generate.py "$INPUT_ZAP" -t "$INPUT_TEMPLATE" -o "$OUTPUT_DIR"
