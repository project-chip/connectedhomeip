#!/bin/bash

SOURCE=${BASH_SOURCE[0]}
SOURCE_DIR=$(cd "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)
REPO_DIR="$SOURCE_DIR/../../../../"

# The image build will clone its own ot-br-posix checkout due to limitations of git submodule.
# Using the same ot-br-posix version as chip
OT_BR_POSIX=$REPO_DIR/third_party/ot-br-posix/repo
OT_BR_POSIX_CHECKOUT=$(git rev-parse :third_party/ot-br-posix/repo)

docker build -t chip-cirque-device-base -f "$SOURCE_DIR"/Dockerfile --build-arg OT_BR_POSIX_CHECKOUT="$OT_BR_POSIX_CHECKOUT" "$SOURCE_DIR"
