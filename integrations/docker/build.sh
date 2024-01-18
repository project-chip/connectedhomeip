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

me=$(basename "$0")
cd "$(dirname "$0")"

GHCR_ORG="ghcr.io"
ORG=${DOCKER_BUILD_ORG:-project-chip}

# directory name is
IMAGE=${DOCKER_BUILD_IMAGE:-$(basename "$(pwd)")}

# version
VERSION=${DOCKER_BUILD_VERSION:-$(sed 's/ .*//' version)}

if [[ $OSTYPE == 'darwin'* ]]; then
    DOCKER_VOLUME_PATH=~/Library/Containers/com.docker.docker/Data/vms/0/
    TARGET_PLATFORM_TYPE="linux/arm64"
else
    DOCKER_VOLUME_PATH=/var/lib/docker/
    TARGET_PLATFORM_TYPE="linux/amd64"
fi

[[ ${*/--help//} != "${*}" ]] && {
    set +x
    echo "Usage: $me <OPTIONS>

  Build and (optionally tag as latest, push) a docker image from Dockerfile in CWD

  Options:
   --no-cache   passed as a docker build argument
   --latest     update latest to the current built version (\"$VERSION\")
   --push       push image(s) to docker.io (requires docker login for \"$ORG\")
   --skip-build skip the build/prune step
   --help       get this message
   --squash     squash docker layers before push them to docker.io (requires docker-squash python module)

"
    exit 0
}

die() {
    echo "$me: *** ERROR: $*"
    exit 1
}

set -ex

[[ -n $VERSION ]] || die "version cannot be empty"

if [ -f "$DOCKER_VOLUME_PATH" ]; then
    mb_space_before=$(df -m "$DOCKER_VOLUME_PATH" | awk 'FNR==2{print $3}')
fi

# go find and build any CHIP images this image is "FROM"
awk -F/ '/^FROM project-chip/ {print $2}' Dockerfile | while read -r dep; do
    dep=${dep%:*}
    (cd "../$dep" && ./build.sh "$@")
done

BUILD_ARGS=()
if [[ ${*/--no-cache//} != "${*}" ]]; then
    BUILD_ARGS+=(--no-cache)
fi

[[ ${*/--skip-build//} != "${*}" ]] || {
    docker build "${BUILD_ARGS[@]}" --build-arg TARGETPLATFORM="$TARGET_PLATFORM_TYPE" --build-arg VERSION="$VERSION" -t "$GHCR_ORG/$ORG/$IMAGE:$VERSION" .
    docker image prune --force
}

[[ ${*/--latest//} != "${*}" ]] && {
    docker tag "$GHCR_ORG"/"$ORG"/"$IMAGE":"$VERSION" "$GHCR_ORG"/"$ORG"/"$IMAGE":latest
}

[[ ${*/--squash//} != "${*}" ]] && {
    command -v docker-squash >/dev/null &&
        docker-squash "$GHCR_ORG"/"$ORG"/"$IMAGE":"$VERSION" -t "$GHCR_ORG"/"$ORG"/"$IMAGE":latest
}

[[ ${*/--push//} != "${*}" ]] && {
    docker push "$GHCR_ORG"/"$ORG"/"$IMAGE":"$VERSION"
    [[ ${*/--latest//} != "${*}" ]] && {
        docker push "$GHCR_ORG"/"$ORG"/"$IMAGE":latest
    }
}

[[ ${*/--clear//} != "${*}" ]] && {
    docker rmi -f "$GHCR_ORG"/"$ORG"/"$IMAGE":"$VERSION"
    [[ ${*/--latest//} != "${*}" ]] && {
        docker rmi -f "$GHCR_ORG"/"$ORG"/"$IMAGE":latest
    }
}

docker images --filter=reference="$GHCR_ORG/$ORG/*"

if [ -f "$DOCKER_VOLUME_PATH" ]; then
    df -h "$DOCKER_VOLUME_PATH"
    mb_space_after=$(df -m "$DOCKER_VOLUME_PATH" | awk 'FNR==2{print $3}')
    printf "%'.f MB total used\n" "$((mb_space_before - mb_space_after))"
fi

exit 0
