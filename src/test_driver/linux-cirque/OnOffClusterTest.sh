#!/bin/bash

set -e

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../../"

chip_tool_dir=$REPO_DIR/examples/chip-tool
chip_light_dir=$REPO_DIR/examples/lighting-app/linux

function build_docker_image() {
    set -x
    cd "$REPO_DIR"
    source scripts/activate.sh
    cd "$chip_tool_dir"
    gn gen out/debug
    ninja -C out/debug
    docker build -t chip_tool -f Dockerfile . >/dev/null 2>&1
}

function build_chip_lighting() {
    cd "$REPO_DIR"
    source scripts/activate.sh
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
    python3 ./test-on-off-cluster.py
}

main
