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

# This file is used to test a single happy test using (almost) the same
# method as ninja does. Suggest passing $HAPPY_LOG_DIR to set path for logs.

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../"

export HAPPY_LOG_DIR="${HAPPY_LOG_DIR:-$(mktemp -d)}"

set -x
env

function happytest_bootstrap() {
    echo "Bootstrapping Happy Test"
    set -e
    # Bootstrap Happy
    cd "$REPO_DIR/third_party/happy/repo"

    # Override happy log dir config.
    python3 -c 'import json, os; file = open("happy/conf/main_config.json"); data = json.load(file); data["log_directory"] = os.environ["HAPPY_LOG_DIR"]; data["default_happy_log_dir"] = os.environ["HAPPY_LOG_DIR"]; out = open("happy/conf/main_config.json", "w"); json.dump(data, out);'

    echo "Happy Main Config"
    cat happy/conf/main_config.json

    apt-get update
    apt-get install -y bridge-utils \
        iproute2 \
        net-tools \
        python3-lockfile \
        python3-pip \
        python3-psutil \
        python3-setuptools \
        strace \
        sudo
    make

    echo "Happy Log dir set to $HAPPY_LOG_DIR"
}

function happytest_run() {
    "$REPO_DIR/scripts/tests/happy_test_wrapper.py" --test-bin-dir "$REPO_DIR/out/$BUILD_TYPE/tests" "$1"
}

function happytest_ninjarun() {
    # This function is used for run happy tests under GN / Ninja
    if [ "$(whoami)" != "root" ]; then
        echo "ninjarun should be invoked under root / user namespace"
        echo 'Try `unshare --map-root-user -n -m scripts/tests/happy_tests.sh ninjarun`'
        return 1
    fi
    mount --make-private /
    mount -t tmpfs tmpfs /run
    ninja -v -C "$REPO_DIR/out/$BUILD_TYPE" -k 0 happy_tests
}

subcommand="$1"
shift

case $subcommand in
    *)
        happytest_"$subcommand" "$@"
        ;;
esac
