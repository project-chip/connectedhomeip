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

# Helper script to update all initialized submodules to the current tip of the
# branch specified in .gitmodules.

set -e

CHIP_ROOT="$(dirname "$0")/../.."

if ! git -C "$CHIP_ROOT" diff-index --cached HEAD; then
    echo >&2 "Staged changes, please reset or commit first."
    exit 1
fi

UPDATES=$(git -C "$CHIP_ROOT" submodule --quiet foreach "bash \$toplevel/scripts/helpers/pull_submodule.sh")

git -C "$CHIP_ROOT" add .gitmodules

git -C "$CHIP_ROOT" commit -m "Pull submodules

Via scripts/helpers/pull_submodules.sh

Updates:
$UPDATES"
