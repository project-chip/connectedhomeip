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

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../"

set -x
env

# Bootstrap Happy
(cd "$REPO_DIR/third_party/happy/repo" && \
    sudo apt-get update && \
    sudo apt-get install -y bridge-utils \
                        net-tools \
                        python3-lockfile \
                        python3-pip \
                        python3-psutil \
                        python3-setuptools && \
    make)

# Run Tests
export TEST_BIN_DIR="$REPO_DIR/out/$BUILD_TYPE"

python3 src/test_driver/happy/tests/standalone/inet/test_inet_multicast_five_nodes_on_wifi.py
