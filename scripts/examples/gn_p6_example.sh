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
    export CY_TOOLS_PATHS="/opt/ModusToolbox/tools_2.3"
elif [ -d "$HOME/ModusToolbox" ]; then
    # Set CY TOOLS PATH
    export CY_TOOLS_PATHS="$HOME/ModusToolbox/tools_2.3"
else
    # Install Modustoolbox
    curl --fail --location --silent --show-error http://dlm.cypress.com.edgesuite.net/akdlm/downloadmanager/software/ModusToolbox/ModusToolbox_2.3/ModusToolbox_2.3.0.4276-linux-install.tar.gz -o /tmp/ModusToolbox_2.3.0.4276-linux-install.tar.gz &&
        tar -C "$HOME" -zxf /tmp/ModusToolbox_2.3.0.4276-linux-install.tar.gz &&
        rm /tmp/ModusToolbox_2.3.0.4276-linux-install.tar.gz

    # Set CY TOOLS PATH
    export CY_TOOLS_PATHS="$HOME/ModusToolbox/tools_2.3"
fi

set -x
env

# Build steps
EXAMPLE_DIR=$1
OUTPUT_DIR=out/example_app
P6_BOARD=CY8CKIT-062S2-43012

if [[ ! -z "$2" ]]; then
    OUTPUT_DIR=$2
fi

gn gen --check --fail-on-unused-args "$OUTPUT_DIR" --root="$EXAMPLE_DIR" --args="p6_board=\"$P6_BOARD\""
ninja -C "$OUTPUT_DIR"
#print stats
arm-none-eabi-size -A "$OUTPUT_DIR"/*.out
