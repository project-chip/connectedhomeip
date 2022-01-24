#!/usr/bin/env bash

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

# Build script for GN EFT32 examples GitHub workflow.

set -e
source "$(dirname "$0")/../../scripts/activate.sh"

set -x
env
USE_WF200=0
USE_RS911X=0
USE_WIFI=0

ROOT=$1
arg2=$2
shift
shift
while [ $# -gt 0 ]; do
    case $1 in
        --wifi)
            if [ -z "$2" ]; then
                echo "--efr requires BRDxxxx"
                exit 1
            fi
            if [ X"$2" = "Xrs911x" ]; then
                WIFI_ARGS="use_rs911x=true"
            elif [ "$2" = wf200 ]; then
                WIFI_ARGS="use_wf200=true"
            else
                echo "Wifi usage: --wifi rs911x|wf200"
                exit 1
            fi
            shift
            shift

            ;;
        --efr)
            if [ -z "$2" ]; then
                echo "--efr requires BRDxxxx"
                exit 1
            fi
            EFR32_BOARD=$2
            shift
            shift
            ;;

        --gnargs)
            echo "-gnargs - Not yet implemented"
            exit 1
            GNARGS=$2
            shift
            shift
            ;;

        *)
            EFR32_BOARD=$1
            shift
            if [ X"$1" != "X" ]; then
                GNARGS=$1
                shift
            fi
            ;;
    esac
done

if [ X"$EFR32_BOARD" = "X" ]; then
    echo "EFR32_BOARD not defined"
    exit 1
fi
BUILD_DIR=$arg2/$EFR32_BOARD
echo BUILD_DIR="$BUILD_DIR"
if [ "X$WIFI_ARGS" != "X" ]; then
    gn gen --check --fail-on-unused-args --root="$ROOT" --dotfile="$ROOT"/build_for_wifi_gnfile.gn --args="efr32_board=\"$EFR32_BOARD\" $WIFI_ARGS" "$BUILD_DIR"
else
    # thread build
    #
    if [ -z "$GNARGS" ]; then
        gn gen --check --fail-on-unused-args --root="$ROOT" --args="efr32_board=\"$EFR32_BOARD\"" "$BUILD_DIR"
    else
        gn gen --check --fail-on-unused-args --root="$ROOT" --args="efr32_board=\"$EFR32_BOARD\" $GNARGS" "$BUILD_DIR"
    fi
fi
ninja -v -C "$BUILD_DIR"/
#print stats
arm-none-eabi-size -A "$BUILD_DIR"/*.out
