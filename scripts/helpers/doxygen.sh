#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
