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

# build.sh   - utility for building (and optionally) tagging and pushing
#               the a Docker image
#
# This script expects to find a Dockerfile next to $0, so symlink
#  in an image name directory is the expected use case.

set -xe

me=$(basename "$0")
SOURCE=${BASH_SOURCE[0]}
cd "$(dirname "$SOURCE")"

SOURCE_DIR=$PWD

ORG=${DOCKER_BUILD_ORG:-connectedhomeip}

# directory name is
IMAGE=${DOCKER_BUILD_IMAGE:-$(basename "$(pwd)")}

# version
VERSION=${DOCKER_BUILD_VERSION:-$(sed 's/ .*//' version)}

GITHUB_ACTION_RUN=${GITHUB_ACTION_RUN:-"0"}

REPO_DIR="$SOURCE_DIR/../../../../"

if [[ "$GITHUB_ACTION_RUN" = "1" ]]; then
    # Note: This script will be invoked in docker on CI, We should ensure CHIP repo to safe directory to silent git error messages.
    git config --global --add safe.directory /home/runner/work/connectedhomeip/connectedhomeip
fi

# The image build will clone its own ot-br-posix checkout due to limitations of git submodule.
# Using the same ot-br-posix version as chip
OT_BR_POSIX=$REPO_DIR/third_party/ot-br-posix/repo
OT_BR_POSIX_CHECKOUT=$(cd "$REPO_DIR" && git rev-parse :third_party/ot-br-posix/repo)

# For chip-cirque-device-base image we use the checkout of ot-br-posix repo
# This is used for not pollute master branch checkout when someone pulls ot-br-posix
# If this fails, the script will still rebuild the image.
CIRQUE_CACHE_PATH=${GITHUB_CACHE_PATH:-"/tmp/cirque-cache/"}
IMAGE_SAVE_PATH="$CIRQUE_CACHE_PATH"/"$IMAGE"_"$OT_BR_POSIX_CHECKOUT".tar

[[ ${*/--help//} != "${*}" ]] && {
    set +x
    echo "Usage: $me <OPTIONS>

  Build and (optionally tag as latest, push) a docker image from Dockerfile in CWD

  Options:
   --no-cache      passed as a docker build argument
   --help          get this message

"
    exit 0
}

die() {
    echo "$me: *** ERROR: $*"
    exit 1
}

set -ex

[[ -n $VERSION ]] || die "version cannot be empty"

# go find and build any CHIP images this image is "FROM"
awk -F/ '/^FROM connectedhomeip/ {print $2}' Dockerfile | while read -r dep; do
    dep=${dep%:*}
    (cd "../$dep" && ./build.sh "$@")
done

BUILD_ARGS=()
if [[ ${*/--no-cache//} != "${*}" ]]; then
    BUILD_ARGS+=(--no-cache)
else
    if docker load -i "$IMAGE_SAVE_PATH"; then
        echo "Loaded docker image from Github action cache."
        BUILD_ARGS+=(--cache-from "$ORG/$IMAGE")
    fi
fi

docker build -t "$ORG/$IMAGE" -f "$SOURCE_DIR/Dockerfile" "${BUILD_ARGS[@]}" --build-arg OT_BR_POSIX_CHECKOUT="$OT_BR_POSIX_CHECKOUT" "$SOURCE_DIR"

if [[ "$GITHUB_ACTION_RUN" = "1" ]]; then
    # Save cache
    mkdir -p "$CIRQUE_CACHE_PATH"
    docker save -o "$IMAGE_SAVE_PATH" "$ORG/$IMAGE"
    echo "Saved docker image for future run: "
    ls -lh "$IMAGE_SAVE_PATH"
    chmod 644 "$IMAGE_SAVE_PATH"
fi

exit 0
