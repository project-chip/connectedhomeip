#!/bin/bash

#
#    Copyright (c) 2023 Project CHIP Authors
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

HERE="$(dirname "$0")"
CHIP_ROOT="$(realpath "$HERE"/..)"
BUILD_VERSION="latest"
IMAGE_TAG="matter-dev-environment:local"
USER_UID=$UID

function show_usage() {
    cat <<EOF
Usage: $0

Build vscode dev environment docker image.

Options:
    -h,--help        Show this help
    -t,--tag         Image tag - default is matter-dev-environment:local
    -u,--uid         User UIDa - default is the current user ID
    -v,--version     Build version - default is the latest
EOF
}

SHORT=t:,u:,h:,v
LONG=tag:,uid:,help:,version:
OPTS=$(getopt -n build --options "$SHORT" --longoptions "$LONG" -- "$@")

eval set -- "$OPTS"

while :; do
    case "$1" in
        -h | --help)
            show_usage
            exit 0
            ;;
        -t | --tag)
            IMAGE_TAG=$2
            shift 2
            ;;
        -u | --uid)
            USER_UID=$2
            shift 2
            ;;
        -v | --version)
            BUILD_VERSION=$2
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Unexpected option: $1"
            show_usage
            exit 2
            ;;
    esac
done

if [ "$USER_UID" = "0" ]; then
    USER_UID=1000
fi

docker build \
    -t "$IMAGE_TAG" \
    --pull \
    --build-arg USER_UID="$USER_UID" \
    --build-arg BUILD_VERSION="$BUILD_VERSION" \
    --network=host \
    "$HERE"
