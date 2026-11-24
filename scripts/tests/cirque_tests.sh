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
export PYTHONPATH="$REPO_DIR/third_party/cirque/repo${PYTHONPATH:+:$PYTHONPATH}"

LOG_DIR=${LOG_DIR:-$(mktemp -d)}
GITHUB_ACTION_RUN=${GITHUB_ACTION_RUN:-"0"}
# Upper bound for a single test, in seconds.
CIRQUE_TEST_TIMEOUT=${CIRQUE_TEST_TIMEOUT:-900}
# Every container Cirque creates for this invocation carries this label, so
# cleanup only removes containers that belong to this run.
CIRQUE_RUN_LABEL="org.project-chip.cirque-test-run=${CIRQUE_RUN_ID:-$$}"

# The image build will clone its own ot-br-posix checkout due to limitations of git submodule.
# Using the same ot-br-posix version as chip
OPENTHREAD=$REPO_DIR/third_party/openthread/repo
OPENTHREAD_CHECKOUT=$(cd "$REPO_DIR" && git rev-parse :third_party/openthread/repo)
OT_BR_POSIX_CHECKOUT=$(cd "$REPO_DIR" && git rev-parse :third_party/ot-br-posix/repo)

CIRQUE_CACHE_PATH=${GITHUB_CACHE_PATH:-"/tmp/cirque-cache/"}
OT_SIMULATION_CACHE="$CIRQUE_CACHE_PATH/ot-simulation-cmake.tgz"
OT_SIMULATION_CACHE_STAMP_FILE="$CIRQUE_CACHE_PATH/ot-simulation.commit"

# Append test name here to add more tests for run_all_tests
#
# NOTE:
#   "InteractionModelTest" is currently disabled due to it overriding
#   internal data model methods (for example it says "CommandExists" for
#   paths where endpoint/cluster do not)
CIRQUE_TESTS=(
    "EchoTest"
    "EchoOverTcpTest"
    "FailsafeTest"
    "MobileDeviceTest"
    "CommissioningTest"
    "IcdDeviceTest"
    "SplitCommissioningTest"
    "CommissioningFailureTest"
    "CommissioningFailureOnReportTest"
    "PythonCommissioningTest"
    "CommissioningWindowTest"
    "SubscriptionResumptionTest"
    "SubscriptionResumptionCapacityTest"
    "SubscriptionResumptionTimeoutTest"
    "BleMobileDeviceTest:thread"
    "BleMobileDeviceTest:wifi"
    "TvCastingTest"
)

BOLD_GREEN_TEXT="\033[1;32m"
BOLD_YELLOW_TEXT="\033[1;33m"
BOLD_RED_TEXT="\033[1;31m"
RESET_COLOR="\033[0m"

function __cirquetest_port_5000_open() {
    python3 -c '
import socket, sys
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.settimeout(1.0)
    sys.exit(0 if s.connect_ex(("127.0.0.1", 5000)) == 0 else 1)
'
}

function __cirquetest_pid_owns_port_5000() {
    python3 -c '
import os, sys

pid = sys.argv[1]
listen_locals_v4 = {"0100007F:1388", "00000000:1388"}
listen_locals_v6 = {
    "00000000000000000000000001000000:1388",
    "00000000000000000000000000000000:1388",
    "0000000000000000FFFF00000100007F:1388",
}

inodes = set()
for path, locals_set in (
    ("/proc/net/tcp", listen_locals_v4),
    ("/proc/net/tcp6", listen_locals_v6),
):
    try:
        with open(path, encoding="utf-8") as fh:
            next(fh, None)
            for line in fh:
                parts = line.split()
                if len(parts) >= 10 and parts[3] == "0A" and parts[1].upper() in locals_set:
                    inodes.add(f"socket:[{parts[9]}]")
    except OSError:
        continue

if not inodes:
    sys.exit(1)

fd_dir = f"/proc/{pid}/fd"
try:
    for entry in os.listdir(fd_dir):
        try:
            if os.readlink(os.path.join(fd_dir, entry)) in inodes:
                sys.exit(0)
        except OSError:
            continue
except OSError:
    sys.exit(1)

sys.exit(1)
' "$1"
}

function __cirquetest_start_flask() {
    echo 'Start Flask'
    FLASK_PID=""
    FLASK_CLEANED=0
    if __cirquetest_port_5000_open; then
        echo "Port 5000 on 127.0.0.1 is already in use; refusing to probe a stale Flask instance" >&2
        return 1
    fi
    cd "$REPO_DIR"/third_party/cirque/repo
    # When running the ManualTests, if Ctrl-C is send to the shell, it will stop flask as well.
    # This is not expected. Start a new session to prevent it from receiving signals
    setsid bash -c 'exec env FLASK_APP=cirque/restservice/service.py \
        FLASK_DEBUG=0 \
        CIRQUE_DOCKER_LABELS="'"$CIRQUE_RUN_LABEL"'" \
        CIRQUE_SKIP_GLOBAL_PRUNE="1" \
        PATH="'"$PATH"'":"'"$REPO_DIR"'"/third_party/openthread/repo/build/simulation/examples/apps/ncp/ \
        python3 -m flask run >"'"$LOG_DIR"'"/"'"$CURRENT_TEST"'"/flask.log 2>&1' &
    FLASK_PID=$!
    echo "Flask running in backgroud with pid $FLASK_PID"
    for _ in $(seq 1 30); do
        if ! kill -0 "$FLASK_PID" >/dev/null 2>&1; then
            echo "Flask process $FLASK_PID exited before becoming ready"
            return 1
        fi
        if __cirquetest_pid_owns_port_5000 "$FLASK_PID" &&
            curl -sf --max-time 5 http://127.0.0.1:5000/get_homes >/dev/null 2>&1; then
            return 0
        fi
        sleep 1
    done
    echo "Flask did not answer after 30 readiness probes"
    return 1
}

function __cirquetest_clean_flask() {
    if [[ "${FLASK_CLEANED:-0}" = "1" ]]; then
        return 0
    fi
    FLASK_CLEANED=1
    local timed_out="${1:-0}"
    echo "Cleanup Flask pid $FLASK_PID"
    local cid
    if [[ "$timed_out" = "1" && -n "$DEVICE_LOG_DIR" && -d "$DEVICE_LOG_DIR" ]]; then
        while IFS= read -r cid; do
            [[ -n "$cid" ]] || continue
            docker logs "$cid" >"$DEVICE_LOG_DIR/timeout_${cid:0:12}.log" 2>&1 || true
        done < <(docker ps -aq --filter "label=$CIRQUE_RUN_LABEL" 2>/dev/null)
    fi
    if [[ -n "$FLASK_PID" ]]; then
        if kill -0 "$FLASK_PID" >/dev/null 2>&1; then
            local homes_json home_id
            homes_json=$(curl -sf --max-time 5 http://127.0.0.1:5000/get_homes 2>/dev/null || true)
            if [[ -n "$homes_json" ]]; then
                while IFS= read -r home_id; do
                    [[ -n "$home_id" ]] || continue
                    curl -sf --max-time 30 "http://127.0.0.1:5000/destroy_home/$home_id" >/dev/null 2>&1 || true
                done < <(python3 -c '
import json, sys
try:
    data = json.load(sys.stdin)
except Exception:
    sys.exit(0)
if isinstance(data, (dict, list)):
    for home_id in data:
        if isinstance(home_id, str) and home_id:
            print(home_id)
' <<<"$homes_json")
            fi
            kill -SIGTERM -"$FLASK_PID" >/dev/null 2>&1 || true
            local waited=0
            while kill -0 "$FLASK_PID" >/dev/null 2>&1 && ((waited < 10)); do
                sleep 1
                waited=$((waited + 1))
            done
            if kill -0 "$FLASK_PID" >/dev/null 2>&1; then
                kill -SIGKILL -"$FLASK_PID" >/dev/null 2>&1 || true
            fi
        fi
        wait "$FLASK_PID" 2>/dev/null || true
    fi
    # Remove containers left behind by this run (e.g. when a test timed out
    # or Flask exited before destroying its home) along with any non-default
    # Docker networks attached to them.
    local labeled_cids net_name
    local -a networks_to_remove=()
    labeled_cids=$(docker ps -aq --filter "label=$CIRQUE_RUN_LABEL" 2>/dev/null || true)
    if [[ -n "$labeled_cids" ]]; then
        while IFS= read -r net_name; do
            case "$net_name" in
                "" | bridge | host | none) ;;
                *) networks_to_remove+=("$net_name") ;;
            esac
        done < <(
            # shellcheck disable=SC2086
            docker inspect --format '{{range $k,$v := .NetworkSettings.Networks}}{{$k}} {{end}}' $labeled_cids 2>/dev/null |
                tr ' ' '\n' | sort -u || true
        )
        # shellcheck disable=SC2086
        docker rm -f $labeled_cids >/dev/null 2>&1 || true
        if ((${#networks_to_remove[@]} > 0)); then
            docker network rm "${networks_to_remove[@]}" >/dev/null 2>&1 || true
        fi
    fi
    FLASK_PID=""
    if [[ -f "$LOG_DIR/$CURRENT_TEST"/flask.log ]]; then
        mv "$LOG_DIR/$CURRENT_TEST"/flask.log "$LOG_DIR/$CURRENT_TEST"/flask.log.old
        cat "$LOG_DIR/$CURRENT_TEST"/flask.log.old | sed 's/\\n/\n/g' | sed 's/\\t/ /g' >"$LOG_DIR/$CURRENT_TEST"/flask.log
        rm "$LOG_DIR/$CURRENT_TEST"/flask.log.old
    fi
}

function __cirquetest_on_interrupt() {
    local sig="$1"
    local child_pid="${2:-}"
    trap - EXIT INT TERM
    if [[ -n "$child_pid" ]] && kill -0 "$child_pid" >/dev/null 2>&1; then
        kill -"$sig" "$child_pid" >/dev/null 2>&1 || true
        wait "$child_pid" 2>/dev/null || true
    fi
    __cirquetest_clean_flask 0
    if [[ "$sig" = "TERM" ]]; then
        exit 143
    fi
    exit 130
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
    pip3 install --break-system-packages pycodestyle==2.5.0 wheel

    make NO_GRPC=1 install -j

    git config --global --add safe.directory /home/runner/work/connectedhomeip/connectedhomeip

    local cirque_base_dir="$REPO_DIR/integrations/docker/images/stage-2/chip-cirque-device-base"
    local base_version
    base_version=${DOCKER_BUILD_VERSION:-$(sed 's/ .*//' "$cirque_base_dir/version")}
    local image_ref="ghcr.io/${DOCKER_BUILD_ORG:-project-chip}/${DOCKER_BUILD_IMAGE:-chip-cirque-device-base}"
    local build_args=(--latest --build-arg "OT_BR_POSIX_CHECKOUT=$OT_BR_POSIX_CHECKOUT")
    local build_version="$base_version"
    local have_base_image=false
    if docker image inspect "$image_ref:$base_version" >/dev/null 2>&1 || docker pull "$image_ref:$base_version" >/dev/null 2>&1; then
        have_base_image=true
    fi

    local dirty_status
    dirty_status=$(cd "$REPO_DIR" && git status --porcelain -- integrations/docker/images/stage-2/chip-cirque-device-base)

    if [[ -n "$dirty_status" ]]; then
        local dirty_hash
        dirty_hash=$({
            "$cirque_base_dir/cachekey.sh"
            printf '%s\n' "$dirty_status"
        } | shasum | awk '{ print substr($1, 1, 12) }')
        build_version="${base_version}-dirty-${dirty_hash}"
    else
        local qualify_tag=false
        if [[ "$have_base_image" = true ]]; then
            local baked_checkout
            baked_checkout=$(docker image inspect --format '{{range .Config.Env}}{{println .}}{{end}}' "$image_ref:$base_version" 2>/dev/null | sed -n 's/^OT_BR_POSIX_CHECKOUT=//p' | head -n 1 || true)
            if [[ "$baked_checkout" != "$OT_BR_POSIX_CHECKOUT" ]]; then
                qualify_tag=true
            fi
        else
            # Initial inspect/pull failed, so OT_BR_POSIX_CHECKOUT on
            # $image_ref:$base_version could not be verified. Qualify the tag so
            # build.sh rebuilds instead of reusing an unverified tag if a later
            # pull succeeds.
            qualify_tag=true
        fi
        if [[ "$qualify_tag" = true ]]; then
            local cache_hash
            cache_hash=$("$cirque_base_dir/cachekey.sh" | shasum | awk '{ print substr($1, 1, 12) }')
            build_version="${base_version}-${cache_hash}"
        fi
    fi

    if [[ "$build_version" != "$base_version" ]]; then
        if [[ "$have_base_image" = true ]]; then
            build_args+=(--cache-from "$image_ref:$base_version")
        elif [[ "$base_version" =~ ^[0-9]+$ ]] && ((10#$base_version > 0)); then
            local prev_image="$image_ref:$((10#$base_version - 1))"
            if docker image inspect "$prev_image" >/dev/null 2>&1 || docker pull "$prev_image" >/dev/null 2>&1; then
                build_args+=(--cache-from "$prev_image")
            fi
        fi
    fi

    DOCKER_BUILD_VERSION="$build_version" "$cirque_base_dir/build.sh" "${build_args[@]}"

    __cirquetest_build_ot_lazy
    pip3 install --break-system-packages -r requirements_nogrpc.txt

    echo "OpenThread Version: $OPENTHREAD_CHECKOUT"
    echo "ot-br-posix Version: $OT_BR_POSIX_CHECKOUT"
}

function cirquetest_run_test() {
    # Start Cirque flash server
    local test_spec="$1"
    local test_script="$test_spec"
    local extra_args=()
    shift
    if [[ "$test_spec" == *:* ]]; then
        test_script="${test_spec%%:*}"
        local test_network="${test_spec#*:}"
        export CURRENT_TEST="${test_script}_${test_network}"
        extra_args=(--network "$test_network")
    else
        export CURRENT_TEST="$test_spec"
    fi
    export DEVICE_LOG_DIR="$LOG_DIR/$CURRENT_TEST"/device_logs
    mkdir -p "$DEVICE_LOG_DIR"
    FLASK_PID=""
    FLASK_CLEANED=0
    trap '__cirquetest_on_interrupt INT' INT
    trap '__cirquetest_on_interrupt TERM' TERM
    trap '__cirquetest_clean_flask 0' EXIT
    if ! __cirquetest_start_flask; then
        trap - EXIT INT TERM
        exitcode=1
        __cirquetest_clean_flask 0
        echo "Test log can be found at $DEVICE_LOG_DIR"
        return 1
    fi
    local test_child_pid=""
    CHIP_CIRQUE_BASE_IMAGE="ghcr.io/project-chip/chip-cirque-device-base" \
        timeout --kill-after=30s "${CIRQUE_TEST_TIMEOUT}s" "$TEST_DIR/$test_script.py" "${extra_args[@]}" "$@" &
    test_child_pid=$!
    trap '__cirquetest_on_interrupt INT "'"$test_child_pid"'"' INT
    trap '__cirquetest_on_interrupt TERM "'"$test_child_pid"'"' TERM
    wait "$test_child_pid"
    exitcode=$?
    local timed_out=0
    if [[ $exitcode -eq 124 || $exitcode -eq 137 ]]; then
        echo "$CURRENT_TEST timed out after ${CIRQUE_TEST_TIMEOUT}s"
        timed_out=1
    fi
    trap - EXIT INT TERM
    __cirquetest_clean_flask "$timed_out"

    # After test finished, the container is preserved and networks will not be deleted
    # when running locally, which aids debugging, while CI sets CLEANUP_DOCKER_FOR_CI=1.
    if [[ "$CLEANUP_DOCKER_FOR_CI" = "1" ]]; then
        echo "Do docker network prune"
        # Cirque creates HomeLan networks before attaching containers and does not
        # apply CIRQUE_DOCKER_LABELS to Docker networks, so prune any remaining
        # unattached networks in CI after __cirquetest_clean_flask removes containers.
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
        if cirquetest_run_test "$test_name" >"$LOG_DIR/${test_name//:/_}.log" 2>&1; then
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
