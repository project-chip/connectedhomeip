#!/usr/bin/env bash

#
# Copyright (c) 2021 Project CHIP Authors
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

source "$(dirname "$0")/../../../scripts/activate.sh"
set -ex
env

GN_ROOT_TARGET=$(dirname "$0")
CHIP_ROOT=$GN_ROOT_TARGET/../../../
OUTDIR=$CHIP_ROOT/out
GN_ARGS="qpg_target_ic=\"qpg6105\" qpg_flavour=\"_ext_flash\""

mkdir -p "$OUTDIR"
gn \
    --root="$CHIP_ROOT" \
    --root-target=//config/qpg/chip-gn \
    --dotfile="$GN_ROOT_TARGET/.gn" \
    --script-executable=python3 \
    --export-compile-commands \
    gen \
    --check \
    --args="$GN_ARGS" \
    --fail-on-unused-args \
    "$OUTDIR"
ninja -C "$OUTDIR"
