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

SOURCE=${BASH_SOURCE[0]}
SOURCE_DIR=$(cd "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)
REPO_DIR=$SOURCE_DIR/../../
TEST_DIR=$REPO_DIR/src/test_driver/linux-cirque

LOG_DIR=${LOG_DIR:-$(mktemp -d)}

# Append test name here to add more tests for run_all_tests
CIRQUE_TESTS=(
    "EchoTest"
    "InteractionModelTest"
    "OnOffClusterTest"
)

BOLD_GREEN_TEXT="\033[1;32m"
BOLD_YELLOW_TEXT="\033[1;33m"
BOLD_RED_TEXT="\033[1;31m"
RESET_COLOR="\033[0m"

function __kill_grep() {
    ps aux | grep "$1" | awk '{print $2}' | sort -k2 -rn |
        while read -r pid; do
            kill -2 "$pid"
        done
}

function __flask_clean() {
    __kill_grep 'flask run'
}

function __socat_clean() {
    __kill_grep 'socat'
}

function __virtual_thread_clean() {
    __kill_grep 'ot-ncp-ftd'
}

function __cirquetest_start_flask() {
    echo 'Start Flask'
    cd "$REPO_DIR"/third_party/cirque/repo
    FLASK_APP='cirque/restservice/service.py' \
        PATH="$PATH":"$REPO_DIR"/third_party/openthread/repo/output/x86_64-unknown-linux-gnu/bin/ \
        python3 -m flask run >"$LOG_DIR/$CURRENT_TEST/flask.log" 2>&1
}

function __cirquetest_clean_flask() {
    echo 'Cleanup Flask'
    __flask_clean
    __socat_clean
    __virtual_thread_clean
}

function __cirquetest_build_ot() {
    pushd .
    cd "$REPO_DIR"/third_party/openthread/repo
    ./bootstrap
    make -f examples/Makefile-simulation
    popd
}

function cirquetest_bootstrap() {
    set -e
    cd "$REPO_DIR"/third_party/cirque/repo
    pip3 install pycodestyle==2.5.0 wheel
    make NO_GRPC=1 install -j
    "$REPO_DIR"/integrations/docker/images/chip-cirque-device-base/build.sh
    __cirquetest_build_ot
    pip3 install -r requirements_nogrpc.txt

    # Call activate here so the later tests can be faster
    # set -e will cause error if activate.sh is sourced twice
    # this is an expected behavior caused by pigweed/activate.sh
    source "$REPO_DIR/scripts/bootstrap.sh"
}

function cirquetest_run_test() {
    # Start Cirque flash server
    export CURRENT_TEST="$1"
    export DEVICE_LOG_DIR="$LOG_DIR/$CURRENT_TEST"/device_logs
    mkdir -p "$DEVICE_LOG_DIR"
    __cirquetest_start_flask &
    sleep 5
    cd "$TEST_DIR"
    ./"$1.sh"
    exitcode=$?
    __cirquetest_clean_flask
    # TODO: Do docker system prune, we cannot filter which container
    # is created by cirque now. This will be implemented later. Currently, only do this on CI

    # After test finished, the container is perserved and networks will not be deleted
    # This is useful when running tests on local workstation, but not for CI.
    if [[ "x$CLEANUP_DOCKER_FOR_CI" = "x1" ]]; then
        echo "Do docker container and network prune"
        # TODO: Filter cirque containers ?
        if ! grep docker.sock /proc/1/mountinfo; then
            docker ps -aq | xargs docker stop >/dev/null 2>&1
        fi
        docker container prune -f >/dev/null 2>&1
        docker network prune -f >/dev/null 2>&1
    fi
    echo "Test log can be found at $DEVICE_LOG_DIR"
    return "$exitcode"
}

function cirquetest_run_all_tests() {
    # shellharden requires quotes around variables, which will break for-each loops
    # This is the workaround
    echo "Logs will be stored at $LOG_DIR"
    test_pass=1
    mkdir -p "$LOG_DIR"
    for test_name in "${CIRQUE_TESTS[@]}"; do
        echo "[ RUN] $test_name"
        if cirquetest_run_test "$test_name" >"$LOG_DIR/$test_name.log" 2>&1; then
            echo -e "[$BOLD_GREEN_TEXT""PASS""$RESET_COLOR] $test_name"
        else
            echo -e "[$BOLD_RED_TEXT""FAIL""$RESET_COLOR] $test_name (Exitcode: $exitcode)"
            test_pass=0
        fi
    done

    if ((test_pass)); then
        echo -e "[$BOLD_GREEN_TEXT""PASS""$RESET_COLOR] Test finished, test log can be found at artifacts"
        return 0
    else
        echo -e "[$BOLD_RED_TEXT""FAIL""$RESET_COLOR] Test failed, test log can be found at artifacts"
        return 1
    fi
}

subcommand=$1
shift

case $subcommand in
    *)
        cirquetest_"$subcommand" "$@"
        exitcode=$?
        if ((exitcode == 127)); then
            echo "Unknown command: $subcommand" >&2
        fi
        exit "$exitcode"
        ;;
esac
