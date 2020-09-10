#!/bin/bash

set -e

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../../"

chip_tool_dir=$REPO_DIR/examples/chip-tool
chip_light_dir=$REPO_DIR/examples/lighting-app/linux

function build_docker_image() {
    # These files should be successfully compiled elsewhere.
    set +e
    source "$REPO_DIR/scripts/activate.sh" > /dev/null
    set -x -e
    cd "$chip_tool_dir"
    gn gen out/debug
    ninja -C out/debug
    docker build -t chip_tool -f Dockerfile . >/dev/null 2>&1
}

function build_chip_lighting() {
    set +e
    source "$REPO_DIR/scripts/activate.sh" > /dev/null
    set -x -e
    cd "$chip_light_dir"
    gn gen out/debug
    ninja -C out/debug
    docker build -t chip_server -f Dockerfile . >/dev/null 2>&1
    set +x
}

function main() {
    pushd .
    build_chip_tool
    build_chip_lighting
    popd
    python3 "$SOURCE_DIR/test-on-off-cluster.py"
}

main
