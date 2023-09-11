#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# Temporary transition script to remove submodules added with previous names
# and re-add them with the new names.

CHIP_ROOT="$(dirname "$0")/../.."

if [[ ! -e "$CHIP_ROOT/.git" ]]; then
    exit 0
fi

# Transitional: Remove keys for submodules with mismatched names
OLD_SUBMODULE_KEYS='submodule\.third_party/.*|submodule\.examples/.*'

if ! git -C "$CHIP_ROOT" config --get-regexp "$OLD_SUBMODULE_KEYS" >&/dev/null; then
    exit 0
fi

# Remove renamed submodules from config
git -C "$CHIP_ROOT" config --get-regexp "$OLD_SUBMODULE_KEYS" | while read key value; do
    git -C "$CHIP_ROOT" config --unset "$key"
done

# Re-init with new submodule names.
git submodule update --init
