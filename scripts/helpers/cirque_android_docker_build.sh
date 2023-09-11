#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# cirque_android_docker_build.sh - utility for building and (optionally)
#                        pushing the Cirque Android Emulator Docker image.
#
# Example usage:
# $ ./cirque_android_docker_build.sh --push
#

set -ex

CHIP_ROOT="$(dirname "$0")/../.."
CIRQUE_RESOURCES=$CHIP_ROOT/third_party/cirque/repo/cirque/resources

ORG=connectedhomeip
IMAGE=android-emulator
TAG=latest
ARGS=()

usage() {
    echo "Usage: $0 [--tag <tag> --push]" >&2
    exit 0
}

while (($#)); do
    case "$1" in
        --tag)
            TAG="$2"
            shift
            ;;
        --push) ARGS+=("--push") ;;
        --help) usage ;;
    esac
    shift
done

docker buildx build -t "$ORG/$IMAGE:$TAG" -f "$CIRQUE_RESOURCES"/Dockerfile.android_emulator "${ARGS[@]}" "$CIRQUE_RESOURCES"
