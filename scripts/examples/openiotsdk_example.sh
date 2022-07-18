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

function show_usage {
    cat <<EOF
Usage: $0 [options] example

Build and/or run the Open IoT SDK example.

Options:
    -h,--help                       Show this help
    -c,--clean                      Clean target build
    -s,--scratch                    Remove build directory at all before building
    -C,--command    <command>       Action to execute <build-run | run | build - default>
    -d,--debug      <debug_enable>  Build in debug mode <true | false - default>
    -p,--path       <build_path>    Build path <build_path - default is example_dir/build>

Examples:
    shell
    unit-tests

You can run individual test suites of unit tests by using their names with the run command:

EOF
    cat /workspaces/connectedhomeip/src/test_driver/openiotsdk/unit-tests/testnames.txt
    echo ""
}

readarray -t TEST_NAMES </workspaces/connectedhomeip/src/test_driver/openiotsdk/unit-tests/testnames.txt

export FAST_MODEL_PLUGINS_PATH=/workspaces/connectedhomeip
IS_TEST=0
NAME="$(basename "$0")"
HERE="$(dirname "$0")"
CHIP_ROOT="$(realpath $HERE/../..)"
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
GDB_PLUGIN="${FAST_MODEL_PLUGINS_PATH}/GDBRemoteConnection.so"
FVP_CONFIG_FILE="${CHIP_ROOT}/examples/platform/openiotsdk/fvp/cs300.conf"
TELNET_TERMINAL_PORT=5000
FAILED_TESTS=0

function build_with_cmake {
    CMAKE="$(which cmake)"
    if [[ ! -f "$CMAKE" ]]; then
        echo "${NAME}: cmake is not in PATH" >&2
        exit 1
    fi

    set -e

    mkdir -p $BUILD_PATH

    if [[ $CLEAN -ne 0 ]]; then
        echo "Clean build" >&2
        if compgen -G "${BUILD_PATH}/CMake*" >/dev/null; then
            cmake --build $BUILD_PATH --target clean
            rm -rf $BUILD_PATH/CMake*
        fi
    fi

    if [[ $SCRATCH -ne 0 ]]; then
        echo "Remove building directory" >&2
        rm -rf $BUILD_PATH
    fi

    BUILD_OPTIONS="-DCMAKE_SYSTEM_PROCESSOR=cortex-m55"
    if $DEBUG; then
        BUILD_OPTIONS="${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Debug"
    fi

    # Remove old artifacts to force linking
    rm -rf "$BUILD_PATH/chip-"*

    cmake -G Ninja -S $EXAMPLE_PATH -B $BUILD_PATH --toolchain=$TOOLCHAIN_PATH $BUILD_OPTIONS
    cmake --build $BUILD_PATH
}

function run_fvp {

    set -e

    # Check if FVP exists
    if ! [ -x "$(command -v ${FVP_BIN})" ]; then
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

    OPTIONS="-C mps3_board.telnetterminal0.start_port=$TELNET_TERMINAL_PORT"

    if $DEBUG; then
        OPTIONS="${OPTIONS} --allow-debug-plugin --plugin $GDB_PLUGIN"
    fi

    $FVP_BIN $OPTIONS -f $FVP_CONFIG_FILE --application $EXAMPLE_EXE_PATH >/dev/null 2>&1 &
    sleep 1

    trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT
    expect <<EOF
    set timeout 1200
    spawn telnet localhost ${TELNET_TERMINAL_PORT}
    expect -re {Test status: (\d+)}
    set retcode \$expect_out(1,string)
    expect "Open IoT SDK unit-tests completed"
    sleep 1
    puts "$EXAMPLE status: \$retcode"
    exit \$retcode
EOF
    FAILED_TESTS=$(expr $FAILED_TESTS + $?)
}

SHORT=C:,p:,d:,c,s,h
LONG=command:,path:,debug:,clean,scratch,help
OPTS=$(getopt -n build --options $SHORT --longoptions $LONG -- "$@")

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

if [[ "$EXAMPLE" == "unit-tests" ]]; then
    IS_TEST=1
fi

if [[ " ${TEST_NAMES[*]} " =~ " $EXAMPLE " ]]; then
    echo "treating example as unit test name"
    if [[ "$COMMAND" == *"build"* ]]; then
        echo "Test suites can only accept --command run"
        show_usage
        exit 2
    fi
    IS_TEST=1
fi

if [[ $IS_TEST -eq 0 ]]; then
    if [[ ! "$EXAMPLE" == "shell" ]]; then
        echo "Wrong example name"
        show_usage
        exit 2
    fi
fi

case "$COMMAND" in
build | run | build-run) ;;
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
fi

if [ -z "${BUILD_PATH}" ]; then
    BUILD_PATH="$EXAMPLE_PATH/build"
fi

if [[ "$COMMAND" == *"build"* ]]; then
    build_with_cmake
fi

if [[ "$COMMAND" == *"run"* ]]; then
    # if user wants to run unit-tests we need to loop through all test names
    if [[ "$EXAMPLE" == "unit-tests" ]]; then

        for NAME in "${TEST_NAMES[@]}"; do
            EXAMPLE=$NAME
            run_fvp
        done
    else
        run_fvp
    fi

    echo "Failed tests total: $FAILED_TESTS"
fi

exit $FAILED_TESTS
