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

set -x

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../"

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
            kill -2 "$pid"
        done
    fi
}

function __virtual_thread_clean() {
    vthread_pid=$(ps aux | grep "[o]t-ncp-ftd" | awk '{print $2}')
    if [ ! -z "$vthread_pid" ]; then
        for pid in "$vthread_pid"; do
            kill -2 "$pid"
        done
    fi
}

__cirquetest_start_flask() {
    cd "$REPO_DIR"/third_party/cirque/repo
    sudo FLASK_APP='cirque/restservice/service.py' \
        PATH="$PATH":"$REPO_DIR"/third_party/cirque/repo/openthread/output/x86_64-unknown-linux-gnu/bin/ \
        python3 -m flask run
}

__cirquetest_clean_flask() {
    __flask_clean
    __socat_clean
    __virtual_thread_clean
}

cirquetest_bootstrap() {
    set -e
    cd "$REPO_DIR"/third_party/cirque/repo
    sudo apt-get install -y bazel socat psmisc tigervnc-standalone-server tigervnc-viewer python3-pip python3-venv python3-setuptools libdbus-glib-1-dev libgirepository1.0-dev
    pip3 install pycodestyle==2.5.0 wheel
    make NO_GRPC=1 install -j
    ./dependency_modules.sh
    pip3 install -r requirements_nogrpc.txt
}

cirquetest_run_test() {
    # Start Cirque flash server
    __cirquetest_start_flask &
    sleep 5
    cd "$REPO_DIR"/src/test_driver/linux-cirque
    ./do_on-off-cluster-test.sh
    exit_code=$?
    __cirquetest_clean_flask
    # Cleanup containers and possibly networks to run more tests
    yes | docker system prune
    return "$exit_code"
}

subcommand="$1"
shift

case $subcommand in
    *)
        cirquetest_"$subcommand" "$@"
        exitcode=$?
        if [ $? = 127 ]; then
            echo "Unknown command: $subcommand" >&2
        fi
        exit "$exitcode"
        ;;
esac
