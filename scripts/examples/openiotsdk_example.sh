#!/bin/bash

#
#    Copyright (c) 2022-2023 Project CHIP Authors
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
LWIP_DEBUG=false
EXAMPLE=""
FVP_BIN=FVP_Corstone_SSE-300_Ethos-U55
GDB_PLUGIN="$FAST_MODEL_PLUGINS_PATH/GDBRemoteConnection.so"
OIS_CONFIG="$CHIP_ROOT/config/openiotsdk"
FVP_CONFIG_FILE="$OIS_CONFIG/fvp/cs300.conf"
EXAMPLE_TEST_PATH="$CHIP_ROOT/src/test_driver/openiotsdk/integration-tests"
TELNET_TERMINAL_PORT=5000
TELNET_CONNECTION_PORT=""
FAILED_TESTS=0
IS_UNIT_TEST=0
FVP_NETWORK="user"
KVS_STORAGE_FILE=""
NO_ACTIVATE=""
CRYPTO_BACKEND="mbedtls"
APP_VERSION="1"
APP_VERSION_STR="0.0.1"

declare -A ps_storage_param=([instance]=qspi_sram [memspace]=0 [address]=0x660000 [size]=0x12000)

readarray -t SUPPORTED_APP_NAMES <"$CHIP_ROOT"/examples/platform/openiotsdk/supported_examples.txt
SUPPORTED_APP_NAMES+=("unit-tests")

readarray -t TEST_NAMES <"$CHIP_ROOT"/src/test_driver/openiotsdk/unit-tests/test_components.txt
readarray -t TEST_NAMES_NL <"$CHIP_ROOT"/src/test_driver/openiotsdk/unit-tests/test_components_nl.txt
TEST_NAMES+=("${TEST_NAMES_NL[@]}")

function show_usage() {
    cat <<EOF
Usage: $0 [options] example [test_name]

Build, run or test the Open IoT SDK examples and unit-tests.

Options:
    -h,--help                           Show this help
    -c,--clean                          Clean target build
    -s,--scratch                        Remove build directory at all before building
    -C,--command    <command>           Action to execute <build-run | run | test | build - default>
    -d,--debug      <debug_enable>      Build in debug mode <true | false - default>
    -l,--lwipdebug  <lwip_debug_enable> Build with LwIP debug logs support <true | false - default>
    -b,--backend    <crypto_backend)    Select crypto backend <psa | mbedtls - default>
    -p,--path       <build_path>        Build path <build_path - default is example_dir/build>
    -K,--kvsfile    <kvs_storage_file>  Path to KVS storage file which will be used to ensure persistence <kvs_storage_file - default is empty which means disable persistence>
    -n,--network    <network_name>      FVP network interface name <network_name - default is "user" which means user network mode>
    -v,--version    <version_number>    Application version number <version_number - default is 1>
    -V,--versionStr <version_str>       Application version string <version_strr - default is "0.0.1">
    --no-activate                       Do not activate the chip build environment
Examples:
EOF

    for app in "${SUPPORTED_APP_NAMES[@]}"; do
        echo "    $app"
    done

    cat <<EOF

You run or test individual test suites of unit tests by using their names [test_name] with the specified command:

EOF

    for test in "${TEST_NAMES[@]}"; do
        echo "    $test"
    done

    cat <<EOF

Use "test" command without a specific test name, runs all supported unit tests.

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
    BUILD_OPTIONS+=(-DCONFIG_CHIP_OPEN_IOT_SDK_SOFTWARE_VERSION="$APP_VERSION")
    BUILD_OPTIONS+=(-DTFM_NS_APP_VERSION="$APP_VERSION_STR")

    if "$DEBUG"; then
        BUILD_OPTIONS+=(-DCMAKE_BUILD_TYPE=Debug)
    else
        BUILD_OPTIONS+=(-DCMAKE_BUILD_TYPE=Release)
    fi

    if "$LWIP_DEBUG"; then
        BUILD_OPTIONS+=(-DCONFIG_CHIP_OPEN_IOT_SDK_LWIP_DEBUG=YES)
    fi

    BUILD_OPTIONS+=(-DCONFIG_CHIP_CRYPTO="$CRYPTO_BACKEND")

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

    # Check if executable file exists
    if ! [ -f "$EXAMPLE_EXE_PATH" ]; then
        echo "Error: $EXAMPLE_EXE_PATH does not exist." >&2
        exit 1
    fi

    # Check if FVP GDB plugin file exists
    if "$DEBUG" && ! [ -f "$GDB_PLUGIN" ]; then
        echo "Error: $GDB_PLUGIN does not exist." >&2
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

    if [ -n "$KVS_STORAGE_FILE" ]; then
        declare -n storage_param=ps_storage_param
        if [ -f "$KVS_STORAGE_FILE" ]; then
            RUN_OPTIONS+=(--data "mps3_board.${storage_param[instance]}=$KVS_STORAGE_FILE@${storage_param[memspace]}:${storage_param[address]}")
        fi
        RUN_OPTIONS+=(--dump "mps3_board.${storage_param[instance]}=$KVS_STORAGE_FILE@${storage_param[memspace]}:${storage_param[address]},${storage_param[size]}")
    fi

    echo "Running $EXAMPLE_EXE_PATH with options: ${RUN_OPTIONS[@]}"

    # Run the FVP
    "$FVP_BIN" "${RUN_OPTIONS[@]}" -f "$FVP_CONFIG_FILE" --application "$EXAMPLE_EXE_PATH" 2>&1 >/tmp/FVP_run_$$ &
    FVP_PID=$!

    # Wait for FVP to start and exist the output file
    timeout=0
    while [ ! -e /tmp/FVP_run_$$ ]; do
        timeout=$((timeout + 1))
        if [ "$timeout" -ge 5 ]; then
            echo "Error: FVP start failed" >&2
            break
        fi
        sleep 1
    done

    while IFS= read -t 5 -r line; do
        if [[ $line == *"Listening for serial connection on port"* ]]; then
            TELNET_CONNECTION_PORT="${line##* }"
            break
        fi
    done </tmp/FVP_run_$$

    if [ -n "$TELNET_CONNECTION_PORT" ]; then
        # Connect FVP via telnet client
        telnet localhost "$TELNET_CONNECTION_PORT"
    else
        echo "Error: FVP start failed" >&2
    fi

    # Stop the FVP
    kill -SIGTERM "$FVP_PID"
    # Wait for the FVP stop
    while kill -0 "$FVP_PID"; do
        sleep 1
    done
    rm -rf /tmp/FVP_run_$$
}

function run_test() {

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

    # Check if pytest exists
    if ! [ -x "$(command -v pytest)" ]; then
        echo "Error: pytest not installed." >&2
        exit 1
    fi

    TEST_OPTIONS=()

    if [[ $FVP_NETWORK ]]; then
        TEST_OPTIONS+=(--networkInterface="$FVP_NETWORK")
    fi

    if [[ "$EXAMPLE" == "ota-requestor-app" ]]; then
        TEST_OPTIONS+=(--updateBinaryPath="${EXAMPLE_EXE_PATH/elf/"ota"}")
        # Check if OTA provider exists, if so get the path to it
        OTA_PROVIDER_APP=$(find . -type f -name "chip-ota-provider-app")
        if [ -z "$OTA_PROVIDER_APP" ]; then
            echo "Error: OTA provider application does not exist." >&2
            exit 1
        fi
        TEST_OPTIONS+=(--otaProvider="$OTA_PROVIDER_APP")
        TEST_OPTIONS+=(--softwareVersion="$APP_VERSION:$APP_VERSION_STR")
    fi

    if [[ -f $EXAMPLE_TEST_PATH/test_report_$EXAMPLE.json ]]; then
        rm -rf "$EXAMPLE_TEST_PATH/test_report_$EXAMPLE".json
    fi

    set +e
    pytest --log-level=INFO --json-report --json-report-summary --json-report-file="$EXAMPLE_TEST_PATH"/test_report_"$EXAMPLE".json --binaryPath="$EXAMPLE_EXE_PATH" --fvp="$FVP_BIN" --fvpConfig="$FVP_CONFIG_FILE" "${TEST_OPTIONS[@]}" "$EXAMPLE_TEST_PATH"/test_app.py
    set -e

    if [[ ! -f $EXAMPLE_TEST_PATH/test_report_$EXAMPLE.json ]]; then
        exit 1
    else
        if [[ $(jq '.summary | has("failed")' $EXAMPLE_TEST_PATH/test_report_$EXAMPLE.json) == true ]]; then
            FAILED_TESTS=$((FAILED_TESTS + $(jq '.summary.failed' "$EXAMPLE_TEST_PATH"/test_report_"$EXAMPLE".json)))
        fi
    fi
}

SHORT=C:,p:,d:,l:,b:,n:,k:,K:,v:,V:,c,s,h
LONG=command:,path:,debug:,lwipdebug:,backend:,network:,kvsstore:,kvsfile:,version:,versionStr:,clean,scratch,help,no-activate
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
        -l | --lwipdebug)
            LWIP_DEBUG=$2
            shift 2
            ;;
        -K | --kvsfile)
            KVS_STORAGE_FILE=$2
            shift 2
            ;;
        -b | --backend)
            CRYPTO_BACKEND=$2
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
        --no-activate)
            NO_ACTIVATE='YES'
            shift
            ;;
        -v | --version)
            APP_VERSION=$2
            shift 2
            ;;
        -V | --versionStr)
            APP_VERSION_STR=$2
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

EXAMPLE=$1

if [[ ! " ${SUPPORTED_APP_NAMES[@]} " =~ " ${EXAMPLE} " ]]; then
    echo "Wrong example name"
    show_usage
    exit 2
fi

case "$COMMAND" in
    build | run | test | build-run) ;;
    *)
        echo "Wrong command definition"
        show_usage
        exit 2
        ;;
esac

if [[ "$EXAMPLE" == "unit-tests" ]]; then
    if [ ! -z "$2" ]; then
        if [[ " ${TEST_NAMES[*]} " =~ " $2 " ]]; then
            EXAMPLE=$2
            echo "Use specific unit test $EXAMPLE"
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
    EXAMPLE_PATH="$CHIP_ROOT/src/test_driver/openiotsdk/unit-tests"
    IS_UNIT_TEST=1
else
    EXAMPLE_PATH="$CHIP_ROOT/examples/$EXAMPLE/openiotsdk"
fi

case "$CRYPTO_BACKEND" in
    psa | mbedtls) ;;
    *)
        echo "Wrong crypto type definition"
        show_usage
        exit 2
        ;;
esac

TOOLCHAIN_PATH="toolchains/toolchain-$TOOLCHAIN.cmake"

if [ -z "$BUILD_PATH" ]; then
    BUILD_PATH="$EXAMPLE_PATH/build"
fi

if [ -z "$NO_ACTIVATE" ]; then
    # Activate Matter environment
    source "$CHIP_ROOT"/scripts/activate.sh
fi

if [[ $IS_UNIT_TEST -eq 0 ]]; then
    EXAMPLE_EXE_PATH="$BUILD_PATH/chip-openiotsdk-$EXAMPLE-example.elf"
    EXAMPLE_TEST_PATH+="/$EXAMPLE"
else
    EXAMPLE_EXE_PATH="$BUILD_PATH/$EXAMPLE.elf"
    EXAMPLE_TEST_PATH+="/unit-tests"
fi

if [[ "$COMMAND" == *"build"* ]]; then
    build_with_cmake
fi

if [[ "$COMMAND" == *"run"* ]]; then
    if [[ "$EXAMPLE" == "unit-tests" ]]; then
        echo "You have to specify the test suites to run"
        show_usage
        exit 2
    else
        run_fvp
    fi
fi

if [[ "$COMMAND" == *"test"* ]]; then
    if [[ "$EXAMPLE" == "unit-tests" ]]; then
        for NAME in "${TEST_NAMES[@]}"; do
            EXAMPLE=$NAME
            EXAMPLE_EXE_PATH="$BUILD_PATH/$EXAMPLE.elf"
            echo "Test specific unit test $EXAMPLE"
            run_test
        done

    else
        run_test
    fi
    echo "Failed tests total: $FAILED_TESTS"
    exit "$FAILED_TESTS"
fi
