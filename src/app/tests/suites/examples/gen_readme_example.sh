#!/usr/bin/env bash

#
#    Copyright (c) 2022 Project CHIP Authors
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

CHIP_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"/../../../../..
echo "$CHIP_ROOT"
INPUT_ZAP="$CHIP_ROOT/src/controller/data_model/controller-clusters.zap"
INPUT_TEMPLATE="$CHIP_ROOT/src/app/tests/suites/examples/templates/templates.json"
OUTPUT_DIR="$CHIP_ROOT/src/app/tests/suites/examples/out"

source "$CHIP_ROOT/scripts/activate.sh"

mkdir -p "$OUTPUT_DIR"

"$CHIP_ROOT"/scripts/tools/zap/generate.py "$INPUT_ZAP" -t "$INPUT_TEMPLATE" -o "$OUTPUT_DIR"
