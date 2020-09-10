#!/bin/bash

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
TEST_DIR=$(readlink -f "$REPO_DIR"/src/test_driver/linux-cirque)

# Append test name here to add more tests for run_all_tests
CIRQUE_TESTS=(
    "OnOffClusterTest"
)

function __flask_clean() {
    flask_pid=$(ps aux | grep "[f]lask run" | grep -v "sudo" | awk '{print $2}' | sort -k2 -rn)
    if [ ! -z "$flask_pid" ]; then
        for pid in "$flask_pid"; do
            sudo kill -2 "$pid"
        done
    fi
}

function __socat_clean() {
    socat_pid=$(ps aux | grep "[s]ocat" | awk '{print $2}')
    if [ ! -z "$socat_pid" ]; then
        for pid in "$socat_pid"; do
            sudo kill -2 "$pid"
        done
    fi
}

function __virtual_thread_clean() {
    vthread_pid=$(ps aux | grep "[o]t-ncp-ftd" | awk '{print $2}')
    if [ ! -z "$vthread_pid" ]; then
        for pid in "$vthread_pid"; do
            sudo kill -2 "$pid"
        done
    fi
}

function __cirquetest_start_flask() {
    echo "Start Flask"
    cd "$REPO_DIR"/third_party/cirque/repo
    sudo FLASK_APP='cirque/restservice/service.py' \
        PATH="$PATH":"$REPO_DIR"/third_party/cirque/repo/openthread/output/x86_64-unknown-linux-gnu/bin/ \
        python3 -m flask run >/dev/null 2>/dev/null
}

function __cirquetest_clean_flask() {
    echo "Cleanup Flask"
    __flask_clean
    __socat_clean
    __virtual_thread_clean
}

function cirquetest_bootstrap() {
    set -e
    cd "$REPO_DIR"/third_party/cirque/repo
    sudo apt-get install -y bazel socat psmisc tigervnc-standalone-server tigervnc-viewer python3-pip python3-venv python3-setuptools libdbus-glib-1-dev libgirepository1.0-dev
    pip3 install pycodestyle==2.5.0 wheel
    make NO_GRPC=1 install -j
    ./dependency_modules.sh
    pip3 install -r requirements_nogrpc.txt

    # Call activate here so the later tests can be faster
    # set -e will cause error if activate.sh is sourced twice
    # this is an expected behavior caused by pigweed/activate.sh
    set +e
    source "$REPO_DIR/scripts/bootstrap.sh"
    source "$REPO_DIR/scripts/activate.sh"
}

function cirquetest_run_test() {
    # Start Cirque flash server
    __cirquetest_start_flask &
    sleep 5
    cd "$TEST_DIR"
    ./"$1.sh"
    exitcode=$?
    __cirquetest_clean_flask
    # Cleanup containers and possibly networks to run more tests
    echo "Do docker system prune"
    (yes | docker system prune) >/dev/null 2>/dev/null
    return "$exitcode"
}

function cirquetest_run_all_tests() {
    # shellharden requires quotes around variables, which will break for-each loops
    # This is the workaround
    for i in "${!CIRQUE_TESTS[@]}"; do
        test_name="${CIRQUE_TESTS[$i]}"
        echo "Run $test_name"
        cirquetest_run_test "$test_name"
        exitcode=$?
        if [ "$exitcode" = 0 ]; then
            echo "[SUCC] $test_name"
        else
            echo "[FAIL] $test_name"
            return "$exitcode"
        fi
    done
}

subcommand="$1"
shift

case $subcommand in
    *)
        cirquetest_"$subcommand" "$@"
        exitcode=$?
        if [ "$exitcode" = 127 ]; then
            echo "Unknown command: $subcommand" >&2
        fi
        exit "$exitcode"
        ;;
esac
