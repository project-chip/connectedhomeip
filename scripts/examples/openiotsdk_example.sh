#!/bin/bash

#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Build and/or run Open IoT SDK examples.

IS_TEST=0
NAME="$(basename "$0")"
HERE="$(dirname "$0")"
CHIP_ROOT="$(realpath "$HERE"/../..)"
COMMAND=build
PLATFORM=corstone300
CLEAN=0
SCRATCH=0
EXAMPLE_PATH=""
BUILD_PATH=""
TOOLCHAIN=arm-none-eabi-gcc
DEBUG=false
EXAMPLE=""
FVP_BIN=FVP_Corstone_SSE-300_Ethos-U55
GDB_PLUGIN="$FAST_MODEL_PLUGINS_PATH/GDBRemoteConnection.so"
OIS_CONFIG="$CHIP_ROOT/config/openiotsdk"
FVP_CONFIG_FILE="$OIS_CONFIG/fvp/cs300.conf"
EXAMPLE_TEST_PATH="$CHIP_ROOT/src/test_driver/openiotsdk/integration-tests"
TELNET_TERMINAL_PORT=5000
FAILED_TESTS=0
FVP_NETWORK="user"

readarray -t TEST_NAMES <"$CHIP_ROOT"/src/test_driver/openiotsdk/unit-tests/testnames.txt

function show_usage() {
    cat <<EOF
Usage: $0 [options] example [test_name]

Build, run or test the Open IoT SDK examples and unit-tests.

Options:
    -h,--help                       Show this help
    -c,--clean                      Clean target build
    -s,--scratch                    Remove build directory at all before building
    -C,--command    <command>       Action to execute <build-run | run | test | build - default>
    -d,--debug      <debug_enable>  Build in debug mode <true | false - default>
    -p,--path       <build_path>    Build path <build_path - default is example_dir/build>
    -n,--network    <network_name>  FVP network interface name <network_name - default is "user" which means user network mode>

Examples:
    shell
    lock-app
    unit-tests

You can run individual test suites of unit tests by using their names [test_name] with the run command:

EOF
    cat "$CHIP_ROOT"/src/test_driver/openiotsdk/unit-tests/testnames.txt
    echo ""
    cat <<EOF
Or you can use all tests suites with <all> parameter as [test_name]

The "test" command can be used for all supported examples expect the unit-tests.

EOF
}

function build_with_cmake() {
    CMAKE="$(which cmake)"
    if [[ ! -f "$CMAKE" ]]; then
        echo "$NAME: cmake is not in PATH" >&2
        exit 1
    fi

    set -e

    mkdir -p "$BUILD_PATH"

    if [[ $CLEAN -ne 0 ]]; then
        echo "Clean build" >&2
        if compgen -G "$BUILD_PATH/CMake*" >/dev/null; then
            cmake --build "$BUILD_PATH" --target clean
            find "$BUILD_PATH" -name 'CMakeCache.txt' -delete
        fi
    fi

    if [[ $SCRATCH -ne 0 ]]; then
        echo "Remove building directory" >&2
        rm -rf "$BUILD_PATH"
    fi

    BUILD_OPTIONS=(-DCMAKE_SYSTEM_PROCESSOR=cortex-m55)
    if "$DEBUG"; then
        BUILD_OPTIONS+=(-DCMAKE_BUILD_TYPE=Debug)
    fi

    # Remove old artifacts to force linking
    rm -rf "$BUILD_PATH/chip-"*

    # Activate Matter environment
    source "$CHIP_ROOT"/scripts/activate.sh
    # Remove access to ARM GCC toolchain from Matter environment, use higher version from OIS environment
    PATH=$(echo "$PATH" | sed 's/:/\n/g' | grep -v "$PW_ARM_CIPD_INSTALL_DIR" | xargs | tr ' ' ':')

    cmake -G Ninja -S "$EXAMPLE_PATH" -B "$BUILD_PATH" --toolchain="$TOOLCHAIN_PATH" "${BUILD_OPTIONS[@]}"
    cmake --build "$BUILD_PATH"
}

function run_fvp() {

    set -e

    # Check if FVP exists
    if ! [ -x "$(command -v "$FVP_BIN")" ]; then
        echo "Error: $FVP_BIN not installed." >&2
        exit 1
    fi

    if [[ $IS_TEST -eq 0 ]]; then
        EXAMPLE_EXE_PATH="$BUILD_PATH/chip-openiotsdk-$EXAMPLE-example.elf"
    else
        EXAMPLE_EXE_PATH="$BUILD_PATH/$EXAMPLE.elf"
    fi

    # Check if executable file exists
    if ! [ -f "$EXAMPLE_EXE_PATH" ]; then
        echo "Error: $EXAMPLE_EXE_PATH does not exist." >&2
        exit 1
    fi

    RUN_OPTIONS=(-C mps3_board.telnetterminal0.start_port="$TELNET_TERMINAL_PORT")
    RUN_OPTIONS+=(--quantum=25)

    if "$DEBUG"; then
        RUN_OPTIONS+=(--allow-debug-plugin --plugin "$GDB_PLUGIN")
    fi

    if [[ $FVP_NETWORK == "user" ]]; then
        RUN_OPTIONS+=(-C mps3_board.hostbridge.userNetworking=1)
    else
        RUN_OPTIONS+=(-C mps3_board.hostbridge.interfaceName="$FVP_NETWORK")
    fi

    echo "Running $EXAMPLE_EXE_PATH with options: ${RUN_OPTIONS[@]}"

    "$FVP_BIN" "${RUN_OPTIONS[@]}" -f "$FVP_CONFIG_FILE" --application "$EXAMPLE_EXE_PATH" >/dev/null 2>&1 &
    FVP_PID=$!
    sleep 1

    if [[ $IS_TEST -eq 1 ]]; then
        set +e
        expect <<EOF
        set timeout 1800
        set retcode -1
        spawn telnet localhost ${TELNET_TERMINAL_PORT}
        expect -re {Test status: (-?\d+)} {
            set retcode \$expect_out(1,string)
        }
        expect "Open IoT SDK unit-tests completed"
        set retcode [expr -1*\$retcode]
        exit \$retcode
EOF
        RETCODE=$?
        FAILED_TESTS=$(expr "$FAILED_TESTS" + "$RETCODE")
        echo "$(jq '. += {($testname): {failed: $result}}' --arg testname "$EXAMPLE" --arg result "$RETCODE" "$EXAMPLE_PATH"/test_report.json)" >"$EXAMPLE_PATH"/test_report.json
    else
        telnet localhost "$TELNET_TERMINAL_PORT"
    fi

    # stop the fvp
    kill -9 "$FVP_PID" || true
    set -e
    sleep 1
}

function run_test() {

    EXAMPLE_EXE_PATH="$BUILD_PATH/chip-openiotsdk-$EXAMPLE-example.elf"
    # Check if executable file exists
    if ! [ -f "$EXAMPLE_EXE_PATH" ]; then
        echo "Error: $EXAMPLE_EXE_PATH does not exist." >&2
        exit 1
    fi

    # Check if FVP exists
    if ! [ -x "$(command -v "$FVP_BIN")" ]; then
        echo "Error: $FVP_BIN not installed." >&2
        exit 1
    fi

    # Activate Matter environment with pytest
    source "$CHIP_ROOT"/scripts/activate.sh

    # Check if pytest exists
    if ! [ -x "$(command -v pytest)" ]; then
        echo "Error: pytest not installed." >&2
        exit 1
    fi

    TEST_OPTIONS=()

    if [[ $FVP_NETWORK ]]; then
        TEST_OPTIONS+=(--networkInterface="$FVP_NETWORK")
    fi

    if [[ -f $EXAMPLE_TEST_PATH/$EXAMPLE/test_report.json ]]; then
        rm -rf "$EXAMPLE_TEST_PATH/$EXAMPLE"/test_report.json
    fi

    set +e
    pytest --json-report --json-report-summary --json-report-file="$EXAMPLE_TEST_PATH/$EXAMPLE"/test_report.json --binaryPath="$EXAMPLE_EXE_PATH" --fvp="$FVP_BIN" --fvpConfig="$FVP_CONFIG_FILE" "${TEST_OPTIONS[@]}" "$EXAMPLE_TEST_PATH/$EXAMPLE"/test_app.py
    set -e

    if [[ ! -f $EXAMPLE_TEST_PATH/$EXAMPLE/test_report.json ]]; then
        exit 1
    else
        if [[ $(jq '.summary | has("failed")' $EXAMPLE_TEST_PATH/$EXAMPLE/test_report.json) == true ]]; then
            FAILED_TESTS=$(jq '.summary.failed' "$EXAMPLE_TEST_PATH/$EXAMPLE"/test_report.json)
        fi
    fi
}

SHORT=C:,p:,d:.n:,c,s,h
LONG=command:,path:,debug:.network:,clean,scratch,help
OPTS=$(getopt -n build --options "$SHORT" --longoptions "$LONG" -- "$@")

eval set -- "$OPTS"

while :; do
    case "$1" in
        -h | --help)
            show_usage
            exit 0
            ;;
        -c | --clean)
            CLEAN=1
            shift
            ;;
        -s | --scratch)
            SCRATCH=1
            shift
            ;;
        -C | --command)
            COMMAND=$2
            shift 2
            ;;
        -d | --debug)
            DEBUG=$2
            shift 2
            ;;
        -p | --path)
            BUILD_PATH=$CHIP_ROOT/$2
            shift 2
            ;;
        -n | --network)
            FVP_NETWORK=$2
            shift 2
            ;;
        -* | --*)
            shift
            break
            ;;
        *)
            echo "Unexpected option: $1"
            show_usage
            exit 2
            ;;
    esac
done

if [[ $# -lt 1 ]]; then
    show_usage >&2
    exit 1
fi

case "$1" in
    shell | unit-tests | lock-app)
        EXAMPLE=$1
        ;;
    *)
        echo "Wrong example name"
        show_usage
        exit 2
        ;;
esac

if [[ "$EXAMPLE" == "unit-tests" ]]; then
    if [ ! -z "$2" ]; then
        if [[ " ${TEST_NAMES[*]} " =~ " $2 " ]]; then
            if [[ "$COMMAND" != *"run"* ]]; then
                echo "Test suites can only accept --command run"
                show_usage
                exit 2
            fi
            EXAMPLE=$2
            echo "Run specific unit test $EXAMPLE"
        elif [[ "$2" == "all" ]]; then
            echo "Use all unit tests"
        else
            echo " Wrong unit test name"
            show_usage
            exit 2
        fi
    else
        echo "Use all unit tests"
    fi
    IS_TEST=1
fi

case "$COMMAND" in
    build | run | test | build-run) ;;
    *)
        echo "Wrong command definition"
        show_usage
        exit 2
        ;;
esac

TOOLCHAIN_PATH="toolchains/toolchain-$TOOLCHAIN.cmake"

if [[ $IS_TEST -eq 0 ]]; then
    EXAMPLE_PATH="$CHIP_ROOT/examples/$EXAMPLE/openiotsdk"
else
    EXAMPLE_PATH="$CHIP_ROOT/src/test_driver/openiotsdk/unit-tests"
    if [[ -f $EXAMPLE_PATH/test_report.json ]]; then
        rm -rf "$EXAMPLE_PATH"/test_report.json
    fi
    echo "{}" >"$EXAMPLE_PATH"/test_report.json
fi

if [ -z "$BUILD_PATH" ]; then
    BUILD_PATH="$EXAMPLE_PATH/build"
fi

if [[ "$COMMAND" == *"build"* ]]; then
    build_with_cmake
fi

if [[ "$COMMAND" == *"run"* ]]; then
    # If user wants to run unit-tests we need to loop through all test names
    if [[ "$EXAMPLE" == "unit-tests" ]]; then
        if "$DEBUG"; then
            echo "You have to specify the test suites to run in debug mode"
            show_usage
            exit 2
        else
            for NAME in "${TEST_NAMES[@]}"; do
                EXAMPLE=$NAME
                echo "$EXAMPLE_PATH"
                echo "Run specific unit test $EXAMPLE"
                run_fvp
            done
            echo "Failed tests total: $FAILED_TESTS"
        fi
    else
        run_fvp
    fi
fi

if [[ "$COMMAND" == *"test"* ]]; then
    if [[ "$EXAMPLE" == "unit-tests" ]]; then
        echo "The test command can not be applied to the unit-tests example"
        show_usage
        exit 2
    else
        IS_TEST=1
        run_test
    fi
fi

if [[ $IS_TEST -eq 1 ]]; then
    exit "$FAILED_TESTS"
fi
