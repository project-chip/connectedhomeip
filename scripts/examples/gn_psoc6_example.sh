#!/usr/bin/env bash

#
#    Copyright (c) 2021 Project CHIP Authors
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

set -e
# Build script for GN P6 examples GitHub workflow.
source "$(dirname "$0")/../../scripts/activate.sh"

# Install required software
if [ -d "/opt/ModusToolbox" ]; then
    export CY_TOOLS_PATHS="/opt/ModusToolbox/tools_2.4"
elif [ -d "$HOME/ModusToolbox" ]; then
    # Set CY TOOLS PATH
    export CY_TOOLS_PATHS="$HOME/ModusToolbox/tools_2.4"
else
    # Install Modustoolbox
    curl --fail --location --silent --show-error https://itoolspriv.infineon.com/itbhs/api/packages/com.ifx.tb.tool.modustoolbox/Versions/2.4.0.5972-public/artifacts/ModusToolbox_2.4.0.5972-linux-install.tar.gz/download?noredirect -o /tmp/ModusToolbox_2.4.0.5972-linux-install.tar.gz &&
        tar -C "$HOME" -zxf /tmp/ModusToolbox_2.4.0.5972-linux-install.tar.gz &&
        rm /tmp/ModusToolbox_2.4.0.5972-linux-install.tar.gz

    # Set CY TOOLS PATH
    export CY_TOOLS_PATHS="$HOME/ModusToolbox/tools_2.4"
fi

set -x
env

# Build steps
EXAMPLE_DIR=$1
shift
OUTPUT_DIR=out/example_app
PSOC6_BOARD=CY8CKIT-062S2-43012

if [[ ! -z "$1" ]]; then
    OUTPUT_DIR=$1
    shift
fi

GN_ARGS=()
NINJA_ARGS=()
for arg; do
    case $arg in
        -v)
            NINJA_ARGS+=(-v)
            ;;
        *=*)
            GN_ARGS+=("$arg")
            ;;
        *import*)
            GN_ARGS+=("$arg")
            ;;
        *)
            echo >&2 "invalid argument: $arg"
            exit 2
            ;;
    esac
done

gn gen --check --fail-on-unused-args "$OUTPUT_DIR" --root="$EXAMPLE_DIR" --args="psoc6_board=\"$PSOC6_BOARD\" ${GN_ARGS[*]}"
ninja -C "$OUTPUT_DIR" "${NINJA_ARGS[@]}"
#print stats
arm-none-eabi-size -A "$OUTPUT_DIR"/*.out
