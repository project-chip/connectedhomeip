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

# This script executes the command given as an argument within
# CHIP_ROOT with a build environment activated.

if [[ -n ${BASH_SOURCE[0]} ]]; then
    source "${BASH_SOURCE[0]%/*}/activate.sh"
else
    source "${0%/*}/activate.sh"
fi

set -e

echo "Executing in build environment: $*"
bash -c -- "$@"
