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
