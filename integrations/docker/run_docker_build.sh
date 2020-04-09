#!/usr/bin/env bash
# Note: this needs to be run from the root directory presently. 
# https://github.com/project-chip/connectedhomeip/issues/273

set -e

VERSION=$(cat integrations/docker/images/chip-build/version)
ORGANIZATION="connectedhomeip"
IMAGE="chip-build"
TARGET_SOURCE_PATH="/var/source"

docker run --rm -w $TARGET_SOURCE_PATH -v ${PWD}:$TARGET_SOURCE_PATH "$ORGANIZATION/$IMAGE:$VERSION" ./integrations/docker/run_build.sh
