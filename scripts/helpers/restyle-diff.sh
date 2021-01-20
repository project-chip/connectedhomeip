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

# Use git and restyle-path to help reformat anything that differs from
#  some base edit point.  Use before pushing a PR to make sure everything
#  you've written is kosher to CI
#
# Usage:
#  restyle-diff.sh [ref]
#
# if unspecified, ref defaults to "master"
#

here=${0%/*}

set -e

CHIP_ROOT=$(cd "$here/../.." && pwd)

restyle-paths() {
    url=https://github.com/restyled-io/restyler/raw/master/bin/restyle-path

    sh <(curl --location --proto "=https" --tlsv1.2 "$url" -sSf) "$@"
}

cd "$CHIP_ROOT"
declare -a paths=()

readarray -t paths < <(git diff --ignore-submodules --name-only "${1:-master}")

restyle-paths "${paths[@]}"
