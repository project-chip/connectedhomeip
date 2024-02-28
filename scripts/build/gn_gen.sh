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

# GN gen script for GN build GitHub workflow.

CHIP_ROOT="$(dirname "$0")/../.."
INPUT_ARGS="$@"
EXTRA_ARGS=""
if [[ -z "${CHIP_ROOT_PATH}" ]]; then
    CHIP_ROOT_PATH=""
    # If no app, build everything (e.g. tests or sample apps that do not care about the data model)
    # with a fixed "include everything" data model
    EXTRA_ARGS="chip_static_data_model_BACKEND=\"//src/controller/data_model:data_model\""
fi

if [[ -n "EXTRA_ARGS" ]]; then
    echo "INPUT:"
    echo "BEFORE: $INPUT_ARGS"
    if [[ "$INPUT_ARGS" == *"--args="* ]]; then
        INPUT_ARGS="${INPUT_ARGS/--args=/--args=$EXTRA_ARGS }"
    else
        INPUT_ARGS="--args=$EXTRA_ARGS $INPUT_ARGS"
    fi
    echo "AFTER:  $INPUT_ARGS"
fi

source "$CHIP_ROOT/scripts/activate.sh"

set -x

env

gn --root="$CHIP_ROOT/$CHIP_ROOT_PATH" gen --check --fail-on-unused-args "$CHIP_ROOT/out/$BUILD_TYPE" "$INPUT_ARGS"
