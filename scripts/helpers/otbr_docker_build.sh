#!/usr/bin/env bash

#
# Copyright (c) 2021 Project CHIP Authors
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

# otbr_docker_build.sh - utility for building (and optionally) tagging and pushing
#                        the otbr Docker image.
#
# Example usage:
# $ ./otbr_docker_build.sh --rev 15d556e --tag TE3 --push
#

set -ex

# Default parameters
ORG=connectedhomeip
IMAGE=otbr
PLATFORMS=linux/amd64,linux/arm/v7,linux/arm64
ARGS=()

# Temporary directory used, within $DIR
OTBR_DIR=$(mktemp -d)

if [ ! -e "$OTBR_DIR" ]; then
    echo "Error while creating the temporary directory" >&2
    exit 1
fi

# Ensure temporary folder is removed on exit
trap 'rm -rf "$OTBR_DIR"' EXIT

usage() {
    echo "Usage: $0 --rev <otbr_revision> [--org <organization> --image <image> --tag <tag> --push]" >&2
    exit 0
}

while (($#)); do
    case "$1" in
        --path)
            OTBR_PATH="$2"
            shift
            ;;
        --rev)
            REVISION="$2"
            shift
            ;;
        --org)
            ORG="$2"
            shift
            ;;
        --image)
            IMAGE="$2"
            shift
            ;;
        --tag)
            TAG="$2"
            shift
            ;;
        --push) ARGS+=("--push") ;;
        --help) usage ;;
    esac
    shift
done

[[ -n "$REVISION" ]] || usage
[[ -n "$TAG" ]] && ARGS+=("-t" "$ORG/$IMAGE:$TAG")

VERSION=otbr-${REVISION:0:7}

# Checkout ot-br-posix to the provided revision
git -C "$OTBR_DIR" clone https://github.com/openthread/ot-br-posix.git .
git -C "$OTBR_DIR" reset --hard "$REVISION"

# Build docker image
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
docker buildx create --use --name otbr_builder --node otbr_node --driver docker-container --platform "$PLATFORMS"
docker buildx build --no-cache -t "$ORG/$IMAGE:$VERSION" -f "$OTBR_DIR/etc/docker/Dockerfile" --platform "$PLATFORMS" "${ARGS[@]}" "$OTBR_DIR"

exit 0
