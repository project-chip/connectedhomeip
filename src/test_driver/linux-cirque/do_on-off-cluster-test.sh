#!/bin/bash

set -x

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../../"

chip_tool_dir=$REPO_DIR/examples/chip-tool

build_docker_image() {
    cd "$REPO_DIR"
    source scripts/activate.sh
    cd "$chip_tool_dir"
    gn gen out/debug
    ninja -C out/debug
    docker build -t chip_tool -f Dockerfile.tool .
    docker build -t chip_server -f Dockerfile.server .
}

main() {
    pushd .
    build_docker_image
    popd
    python3 ./test-on-off-cluster.py
}

main
