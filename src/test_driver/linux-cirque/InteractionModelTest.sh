#!/usr/bin/env bash

#
# Copyright (c) 2021 Project CHIP Authors
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

im_source=$REPO_DIR/src/app/tests/integration

function build_image() {
    # These files should be successfully compiled elsewhere.
    source "$REPO_DIR/scripts/activate.sh" >/dev/null
    set -x
    cd "$im_source"
    gn gen out >/dev/null
    run_ninja -C out
    docker build -t chip_im_initiator -f Dockerfile.initiator . 2>&1
    docker build -t chip_im_responder -f Dockerfile.responder . 2>&1
}

function main() {
    pushd .
    build_image
    popd
    python3 "$SOURCE_DIR/test-interaction-model.py"
}

source "$SOURCE_DIR"/shell-helpers.sh
main
