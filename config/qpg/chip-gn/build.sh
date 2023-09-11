#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

source "$(dirname "$0")/../../../scripts/activate.sh"
set -ex
env

GN_ROOT_TARGET=$(dirname "$0")
CHIP_ROOT=$GN_ROOT_TARGET/../../../
OUTDIR=$CHIP_ROOT/out

mkdir -p "$OUTDIR"
gn \
    --root="$CHIP_ROOT" \
    --root-target=//config/qpg/chip-gn \
    --dotfile="$GN_ROOT_TARGET/.gn" \
    --script-executable=python3 \
    --export-compile-commands \
    gen \
    --check \
    --fail-on-unused-args \
    "$OUTDIR"
ninja -C "$OUTDIR"
