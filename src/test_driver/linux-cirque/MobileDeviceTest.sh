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

set -e

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../../"

test_script_dir=$REPO_DIR/src/controller/python
chip_light_dir=$REPO_DIR/examples/lighting-app/linux

function build_chip_controller() {
    # These files should be successfully compiled elsewhere.
    source "$REPO_DIR/scripts/activate.sh" >/dev/null
    set -x
    cd "$test_script_dir"
    gn gen --check --fail-on-unused-args out/debug >/dev/null
    run_ninja -C out/debug
    mkdir -p "$test_script_dir/test/whl/"
    cp -R "$test_script_dir/out/debug/controller/python/" "$test_script_dir/test/whl/"
    cd "$test_script_dir/test"
    docker build -t chip_mobile_device -f Dockerfile . 2>&1
}

function build_chip_lighting() {
    source "$REPO_DIR/scripts/activate.sh" >/dev/null
    set -x
    cd "$chip_light_dir"
    gn gen --check --fail-on-unused-args out/debug
    run_ninja -C out/debug
    docker build -t chip_server -f Dockerfile . 2>&1
    set +x
}

function main() {
    pushd .
    build_chip_controller
    build_chip_lighting
    popd
    python3 "$SOURCE_DIR/test-mobile-device.py"
}

source "$SOURCE_DIR"/shell-helpers.sh
main
