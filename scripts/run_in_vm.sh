#!/usr/bin/env bash
#
# Copyright (c) 2024 Project CHIP Authors
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

# This script executes the command given as an argument after
# activating the given python virtual environment
set -e

PROJECT_PATH=$(dirname "$(dirname "$(realpath "$0")")")

echo "$@" >"$PROJECT_PATH/runner.sh"
chmod +x "$PROJECT_PATH/runner.sh"

echo "CMD:"
cat "$PROJECT_PATH/runner.sh"

"$PROJECT_PATH/integrations/docker/images/stage-2/chip-build-linux-qemu/run-img.sh"

if [ -f "$PROJECT_PATH/runner_status" ]; then
    exit "$(cat "$PROJECT_PATH/runner_status")"
else
    exit 1
fi
