#!/usr/bin/env bash
set -e

VERSION=$(cat integrations/docker/images/chip-build/version)
ORGANIZATION="rwalkerapple"
IMAGE="chip-build"
TARGET_SOURCE_PATH="/var/source"

docker run --rm -w $TARGET_SOURCE_PATH -v ${PWD}:$TARGET_SOURCE_PATH "$ORGANIZATION/$IMAGE:$VERSION" ./integrations/docker/run_build.sh
