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

function happytest_install_packages() {
    if [ "$(whoami)" != "root" ]; then
        echo "install_packages should be invoked under root"
        return 1
    fi
    echo "Install packages: avahi-daemon bridge-utils iproute2 net-tools python3-lockfile python3-pip python3-psutil python3-setuptools strace"
    apt-get update && apt-get install -y avahi-daemon \
        bridge-utils \
        iproute2 \
        net-tools \
        python3-lockfile \
        python3-pip \
        python3-psutil \
        python3-setuptools \
        strace
}

function happytest_bootstrap() {
    echo "Bootstrapping Happy Test"
    set -e
    service dbus start
    service avahi-daemon start
    # Bootstrap Happy
    cd "$REPO_DIR/third_party/happy/repo"

    python3 setup.py install --user
}

function happytest_run() {
    "$REPO_DIR/scripts/tests/happy_test_wrapper.py" --test-bin-dir "$REPO_DIR/out/$BUILD_TYPE/tests" "$1"
}

subcommand="$1"
shift

case $subcommand in
    *)
        happytest_"$subcommand" "$@"
        ;;
esac
