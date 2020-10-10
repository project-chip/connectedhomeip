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

CHIP_ROOT="$(dirname "$0")/../.."
CHIP_NAME="Connected Home over IP"
CHIP_VERSION="$(git rev-parse --short HEAD)"

die() {
    echo " *** ERROR: $*"
    exit 1
}

install_doxygen() {
    if ! doxygen --version >/dev/null 2>&1; then
        echo "doxygen not installed. Installing..."
        case "$(uname)" in
            "Darwin")
                brew install doxygen
                ;;
            "Linux")
                sudo apt-get update
                sudo apt-get install -y doxygen
                ;;
            *)
                die
                ;;
        esac
    fi
}

install_graphviz() {
    if ! dot -V >/dev/null 2>&1; then
        echo "graphviz not installed. Installing..."
        case "$(uname)" in
            "Darwin")
                brew install graphviz
                ;;
            "Linux")
                sudo apt-get update
                sudo apt-get install -y graphviz
                ;;
            *)
                die
                ;;
        esac
    fi
}

install_doxygen
install_graphviz

cd "$CHIP_ROOT" || exit 1

CHIP_NAME=$CHIP_NAME CHIP_VERSION=$CHIP_VERSION doxygen docs/Doxyfile || die "doxygen failed"

exit 0
