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
EOF
}

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

    EXAMPLE_EXE_PATH="$BUILD_PATH/chip-openiotsdk-$EXAMPLE-example.elf"
    # Check if executable file exists
    if ! [ -f "$EXAMPLE_EXE_PATH" ]; then
        echo "Error: $EXAMPLE_EXE_PATH does not exist." >&2
        exit 1
    fi

    OPTIONS="-C mps3_board.visualisation.disable-visualisation=1 -C mps3_board.smsc_91c111.enabled=1 -C mps3_board.hostbridge.userNetworking=1 -C cpu0.semihosting-enable=1 -C mps3_board.telnetterminal0.start_telnet=0 -C mps3_board.uart0.out_file="-"  -C mps3_board.uart0.unbuffered_output=1 --stat  -C mps3_board.DISABLE_GATING=1"

    if $DEBUG; then
        OPTIONS="${OPTIONS} -I"
    fi

    $FVP_BIN $OPTIONS --application $EXAMPLE_EXE_PATH
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

case "$1" in
shell)
    EXAMPLE=$1
    ;;
*)
    echo "Wrong example name"
    show_usage
    exit 2
    ;;
esac

case "$COMMAND" in
build | run | build-run) ;;
*)
    echo "Wrong command definition"
    show_usage
    exit 2
    ;;
esac

TOOLCHAIN_PATH="toolchains/toolchain-$TOOLCHAIN.cmake"
EXAMPLE_PATH="$CHIP_ROOT/examples/$EXAMPLE/openiotsdk"
if [ -z "${BUILD_PATH}" ]; then
    BUILD_PATH="$EXAMPLE_PATH/build"
fi

if [[ "$COMMAND" == *"build"* ]]; then
    build_with_cmake
fi

if [[ "$COMMAND" == *"run"* ]]; then
    run_fvp
fi
