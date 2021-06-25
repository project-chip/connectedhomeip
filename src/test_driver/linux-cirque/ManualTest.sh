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

# This is the wrapper for setting up testing topology in cirque.
# You can modify build_* part to build your own docker image.
# This test should not be executed over CHIP.
#
# You SHOULD run this script by cirque test wrapper:
#   ./scripts/tests/cirque_tests.sh run_test ManualTest
# under CHIP checkout.

set -ex

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../../"

python3 "$SOURCE_DIR/test-manual.py" "$@"
