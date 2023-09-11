#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# build.sh   - utility for building (and optionally) tagging and pushing
#               the a Docker image
#
# This script expects to output a string that can be used to identify one image.

me=$(basename "$0")
cd "$(dirname "$0")"

SOURCE=${BASH_SOURCE[0]}
SOURCE_DIR=$(cd "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)
REPO_DIR="$SOURCE_DIR/../../../../"

# The image build will clone its own ot-br-posix checkout due to limitations of git submodule.
# Using the same ot-br-posix version as chip
OT_BR_POSIX=$REPO_DIR/third_party/ot-br-posix/repo
OT_BR_POSIX_CHECKOUT=$(cd "$REPO_DIR" && git rev-parse :third_party/ot-br-posix/repo)

filehash() {
    shasum "$1" | awk '{ print $1 }'
}

echo "ot-br-posix.$OT_BR_POSIX_CHECKOUT.dockerfile.$(filehash ./Dockerfile)"
