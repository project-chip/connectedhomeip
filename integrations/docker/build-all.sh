#!/usr/bin/env bash

#
# Copyright (c) 2020 Project CHIP Authors
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

#
# temporary wrapper build script until we can build something better
#  https://github.com/project-chip/connectedhomeip/issues/710
#

set -e

SUB_PATH="*"
ALL_ARGS=""
SKIP_BUILT=0

for i in "$@"; do
    case $i in
    -h | --help)
       echo "Supported arguments:"
       echo "  -h/--help       This help text"
       echo "  -p/--path       sub-path in integrations/docker/images to build (defaults to * for everything)"
       echo "  -skip-built     Do not rebuild images already found as built"
       exit 0
       ;;
    -p=* | --path=*)
        SUB_PATH="${i#*=}"
        ;;
    --skip-built)
        SKIP_BUILT=1
	;;
    *)
        ALL_ARGS="$ALL_ARGS ${i#*=}"
        ;;
    esac
done

VERSION=${DOCKER_BUILD_VERSION:-$(sed 's/ .*//' "$(git rev-parse --show-toplevel)"/integrations/docker/images/base/chip-build/version)}

echo "SUB_PATH: $SUB_PATH"
echo "VERSION: $VERSION"

function build_image() {
    PARSE_PATH=$1
    ARGS_TO_PASS=$2

    echo "PARSE_PATH: $PARSE_PATH"
    echo "ARGS_TO_PASS: $ARGS_TO_PASS"

    find "$(git rev-parse --show-toplevel)"/integrations/docker/images/$PARSE_PATH -name Dockerfile ! -path "*chip-cert-bins/*" | sort | while read -r dockerfile; do
        # Images are of the form `ghcr.io/project-chip/{name}` and tagged as "${VERSION}"
        DOCKER_PATH=$(dirname $dockerfile) # Drop the file name
        IMAGE_NAME="ghcr.io/project-chip/${DOCKER_PATH##*/}:${VERSION}"     # Drop directory prefix

        if [ $SKIP_BUILT -ne 0 ] && docker image inspect "${IMAGE_NAME}" >/dev/null 2>&1; then
          echo "Image ${IMAGE_NAME} already exists. Skipping build"
          continue
        fi

        echo "BUILDING $(dirname "$dockerfile") (i.e. ${IMAGE_NAME})"
        pushd "$(dirname "$dockerfile")" >/dev/null
        ./build.sh "$ARGS_TO_PASS"
        popd >/dev/null
    done
}

build_image "$SUB_PATH" "$ALL_ARGS"
