#!/usr/bin/env bash

# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Setup NXP Zephyr workspace for local development.
#
# This script initializes the Zephyr workspace at /opt/nxp-zephyr/zephyrproject
# using the same NXP_ZEPHYR_REVISION as the CI runtime setup, so local
# development and CI always use the same upstream Zephyr revision.
#
# Usage: bash scripts/setup/nxp/setup_zephyr_workspace.sh

set -e

CHIP_ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"

# Ensure west is available - add pigweed venv to PATH if needed
if ! command -v west &>/dev/null; then
    for venv_dir in "$CHIP_ROOT"/.environment-vscode/pigweed-venv/bin "$CHIP_ROOT"/.environment/pigweed-venv/bin; do
        if [ -f "$venv_dir/west" ]; then
            export PATH="$venv_dir:$PATH"
            break
        fi
    done
fi

if ! command -v west &>/dev/null; then
    echo "Error: west not found. Please run 'source scripts/activate.sh' first."
    exit 1
fi
ZEPHYR_WS=/opt/nxp-zephyr/zephyrproject

NXP_ZEPHYR_REVISION=$(grep "NXP_ZEPHYR_REVISION:" "$CHIP_ROOT/.github/workflows/examples-nxp.yaml" | awk '{print $2}' | tr -d ' ')
if [ "$NXP_ZEPHYR_REVISION" = "" ]; then
    echo "Error: NXP_ZEPHYR_REVISION not found in .github/workflows/examples-nxp.yaml"
    exit 1
fi

echo "Setting up NXP Zephyr workspace at $ZEPHYR_WS"
echo "Using NXP_ZEPHYR_REVISION: $NXP_ZEPHYR_REVISION"

rm -rf "$ZEPHYR_WS"
mkdir -p "$ZEPHYR_WS/zephyr"
cd "$ZEPHYR_WS/zephyr"
git init
git remote add origin https://github.com/zephyrproject-rtos/zephyr.git
git fetch --depth=1 origin "$NXP_ZEPHYR_REVISION"
git reset --hard "$NXP_ZEPHYR_REVISION"
cd "$ZEPHYR_WS"
unset ZEPHYR_BASE
west init -l zephyr
west update -o=--depth=1 -n
west zephyr-export

echo "NXP Zephyr workspace setup complete."
