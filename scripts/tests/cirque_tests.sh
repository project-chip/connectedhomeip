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
# Don't warn about unreachable commands in this file:
# shellcheck disable=SC2317

SOURCE=${BASH_SOURCE[0]}
SOURCE_DIR=$(cd "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)
REPO_DIR=$SOURCE_DIR/../../
TEST_DIR=$REPO_DIR/src/test_driver/linux-cirque

LOG_DIR=${LOG_DIR:-$(mktemp -d)}
GITHUB_ACTION_RUN=${GITHUB_ACTION_RUN:-"0"}

# The image build will clone its own ot-br-posix checkout due to limitations of git submodule.
# Using the same ot-br-posix version as chip
OPENTHREAD=$REPO_DIR/third_party/openthread/repo
OPENTHREAD_CHECKOUT=$(cd "$REPO_DIR" && git rev-parse :third_party/openthread/repo)
OT_BR_POSIX_CHECKOUT=$(cd "$REPO_DIR" && git rev-parse :third_party/ot-br-posix/repo)

CIRQUE_CACHE_PATH=${GITHUB_CACHE_PATH:-"/tmp/cirque-cache/"}
OT_SIMULATION_CACHE="$CIRQUE_CACHE_PATH/ot-simulation-cmake.tgz"
OT_SIMULATION_CACHE_STAMP_FILE="$CIRQUE_CACHE_PATH/ot-simulation.commit"

# Append test name here to add more tests for run_all_tests
CIRQUE_TESTS=(
    "EchoTest"
    "EchoOverTcpTest"
    "FailsafeTest"
    "MobileDeviceTest"
    "CommissioningTest"
    "InteractionModelTest"
    "SplitCommissioningTest"
    "CommissioningFailureTest"
    "CommissioningFailureOnReportTest"
    "PythonCommissioningTest"
    "CommissioningWindowTest"
    "SubscriptionResumptionTest"
    "SubscriptionResumptionCapacityTest"
    "SubscriptionResumptionTimeoutTest"
)

BOLD_GREEN_TEXT="\033[1;32m"
BOLD_YELLOW_TEXT="\033[1;33m"
BOLD_RED_TEXT="\033[1;31m"
RESET_COLOR="\033[0m"

function __cirquetest_start_flask() {
    echo 'Start Flask'
    cd "$REPO_DIR"/third_party/cirque/repo
    # When running the ManualTests, if Ctrl-C is send to the shell, it will stop flask as well.
    # This is not expected. Start a new session to prevent it from receiving signals
    setsid bash -c 'FLASK_APP=cirque/restservice/service.py \
        PATH="'"$PATH"'":"'"$REPO_DIR"'"/third_party/openthread/repo/build/simulation/examples/apps/ncp/ \
        python3 -m flask run >"'"$LOG_DIR"'"/"'"$CURRENT_TEST"'"/flask.log 2>&1' &
    FLASK_PID=$!
    echo "Flask running in backgroud with pid $FLASK_PID"
}

function __cirquetest_clean_flask() {
    echo "Cleanup Flask pid $FLASK_PID"
    kill -SIGTERM -"$FLASK_PID"
    mv "$LOG_DIR/$CURRENT_TEST"/flask.log "$LOG_DIR/$CURRENT_TEST"/flask.log.old
    cat "$LOG_DIR/$CURRENT_TEST"/flask.log.old | sed 's/\\n/\n/g' | sed 's/\\t/ /g' >"$LOG_DIR/$CURRENT_TEST"/flask.log
    rm "$LOG_DIR/$CURRENT_TEST"/flask.log.old
}

function __cirquetest_build_ot() {
    echo -e "[$BOLD_YELLOW_TEXT""INFO""$RESET_COLOR] Cache miss, build openthread simulation."
    script/cmake-build simulation -DOT_THREAD_VERSION=1.2 -DOT_MTD=OFF -DOT_FTD=OFF -DWEB_GUI=0 -DNETWORK_MANAGER=0 -DREST_API=0 -DNAT64=0 -DOT_LOG_OUTPUT=PLATFORM_DEFINED -DOT_LOG_LEVEL=DEBG
    mkdir -p "$(dirname "$OT_SIMULATION_CACHE")"
    tar czf "$OT_SIMULATION_CACHE" build
    echo "$OPENTHREAD_CHECKOUT" >"$OT_SIMULATION_CACHE_STAMP_FILE"
}

function __cirquetest_build_ot_lazy() {
    pushd .
    cd "$REPO_DIR"/third_party/openthread/repo
    ([[ -f "$OT_SIMULATION_CACHE_STAMP_FILE" ]] &&
        [[ "$(cat "$OT_SIMULATION_CACHE_STAMP_FILE")" = "$OPENTHREAD_CHECKOUT" ]] &&
        [[ -f "$OT_SIMULATION_CACHE" ]] &&
        tar zxf "$OT_SIMULATION_CACHE") ||
        __cirquetest_build_ot
    popd
}

function __cirquetest_self_hash() {
    shasum "$SOURCE" | awk '{ print $1 }'
}

function cirquetest_cachekey() {
    echo "$("$REPO_DIR"/integrations/docker/images/stage-2/chip-cirque-device-base/cachekey.sh).openthread.$OPENTHREAD_CHECKOUT.cirque_test.$(__cirquetest_self_hash)"
}

function cirquetest_cachekeyhash() {
    cirquetest_cachekey | shasum | awk '{ print $1 }'
}

function cirquetest_bootstrap() {
    set -ex

    cd "$REPO_DIR"/third_party/cirque/repo
    pip3 uninstall -y setuptools
    pip3 install setuptools==65.7.0
    pip3 install pycodestyle==2.5.0 wheel

    make NO_GRPC=1 install -j

    git config --global --add safe.directory /home/runner/work/connectedhomeip/connectedhomeip

    "$REPO_DIR"/integrations/docker/images/stage-2/chip-cirque-device-base/build.sh --build-arg OT_BR_POSIX_CHECKOUT="$OT_BR_POSIX_CHECKOUT"

    __cirquetest_build_ot_lazy
    pip3 install -r requirements_nogrpc.txt

    echo "OpenThread Version: $OPENTHREAD_CHECKOUT"
    echo "ot-br-posix Version: $OT_BR_POSIX_CHECKOUT"
}

function cirquetest_run_test() {
    # Start Cirque flash server
    export CURRENT_TEST="$1"
    export DEVICE_LOG_DIR="$LOG_DIR/$CURRENT_TEST"/device_logs
    shift
    mkdir -p "$DEVICE_LOG_DIR"
    __cirquetest_start_flask
    sleep 5
    CHIP_CIRQUE_BASE_IMAGE="ghcr.io/project-chip/chip-cirque-device-base" "$TEST_DIR/$CURRENT_TEST.py" "$@"
    exitcode=$?
    __cirquetest_clean_flask
    # TODO: Do docker system prune, we cannot filter which container
    # is created by cirque now. This will be implemented later. Currently, only do this on CI

    # After test finished, the container is perserved and networks will not be deleted
    # This is useful when running tests on local workstation, but not for CI.
    if [[ "$CLEANUP_DOCKER_FOR_CI" = "1" ]]; then
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

    if [[ "$GITHUB_ACTION_RUN" = "1" ]]; then
        echo -e "[$BOLD_YELLOW_TEXT""INFO""$RESET_COLOR] Logs will be uploaded to artifacts."
    fi

    if ((test_pass)); then
        echo -e "[$BOLD_GREEN_TEXT""PASS""$RESET_COLOR] Test finished, test log can be found at $LOG_DIR"
        return 0
    else
        echo -e "[$BOLD_RED_TEXT""FAIL""$RESET_COLOR] Test failed, test log can be found at $LOG_DIR"
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
