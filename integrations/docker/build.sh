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
else
    DOCKER_VOLUME_PATH=/var/lib/docker/
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

if [[ -z $DOCKER_BUILD_PLATFORM ]]; then
    case "$(uname -m)" in
        arm64 | aarch64) DOCKER_BUILD_PLATFORM="linux/arm64" ;;
        x86_64 | amd64) DOCKER_BUILD_PLATFORM="linux/amd64" ;;
        *)
            echo "$me: *** ERROR: unsupported host architecture: $(uname -m)"
            exit 1
            ;;
    esac
fi
TARGET_PLATFORM_TYPE="$DOCKER_BUILD_PLATFORM"

die() {
    echo "$me: *** ERROR: $*"
    exit 1
}

set -ex

[[ -n $VERSION ]] || die "version cannot be empty"

if [ -f "$DOCKER_VOLUME_PATH" ]; then
    mb_space_before=$(df -m "$DOCKER_VOLUME_PATH" | awk 'FNR==2{print $3}')
fi

# Save original arguments for recursive calls before parsing
ORIG_ARGS=("$@")

BUILD_ARGS=()
LATEST=false
PUSH=false
SKIP_BUILD=false
SQUASH=false
CLEAR=false
NO_CACHE=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-cache)
            BUILD_ARGS+=("$1")
            NO_CACHE=true
            shift
            ;;
        --latest)
            LATEST=true
            shift
            ;;
        --push)
            PUSH=true
            shift
            ;;
        --skip-build)
            SKIP_BUILD=true
            shift
            ;;
        --squash)
            SQUASH=true
            shift
            ;;
        --clear)
            CLEAR=true
            shift
            ;;
        --build-arg)
            BUILD_ARGS+=("$1" "$2")
            shift 2
            ;;
        --build-arg=*)
            BUILD_ARGS+=("$1")
            shift
            ;;
        *)
            # Forward any other arguments to docker build
            BUILD_ARGS+=("$1")
            shift
            ;;
    esac
done

# go find and build any CHIP images this image is "FROM".
# Images are referenced as ghcr.io/project-chip/<name>:<tag>, and the parent may
# live in any stage directory, so resolve it by name under images/ rather than
# assuming it is a sibling.
# The script is symlinked into each image directory and has already cd'd there,
# so images/ is two levels up. Derived from that rather than from git, so the
# walk works on an exported tree with no repository.
IMAGES_ROOT="$(cd ../.. && pwd)"
# Take the image token ($2) so a trailing "AS <alias>" is excluded by
# construction rather than by the tag strip happening to swallow it, and cut at
# ":" or "@" so both tagged and digest references reduce to the image name.
awk 'toupper($1) == "FROM" && $2 ~ /project-chip\// {
        ref = $2
        sub(/.*project-chip\//, "", ref)
        sub(/[:@].*/, "", ref)
        print ref
     }' Dockerfile |
    sort -u | while read -r dep; do
    dep_dir=$(find "$IMAGES_ROOT" -maxdepth 2 -type d -name "$dep" | head -1)
    [[ -n $dep_dir ]] || die "cannot locate a directory for parent image '$dep' under $IMAGES_ROOT"
    # Prefer a parent that already exists over building one. Locally that avoids
    # rebuilding the base image once per dependent image during a build-all run;
    # in CI, where each image builds on its own runner, it avoids rebuilding the
    # base for every image in the matrix.
    #
    # Building is the fallback rather than the default, since the tag is missing
    # exactly when a change has not been published yet, which is the case on a
    # pull request that bumps a version file.
    #
    # $VERSION is what the parent would be tagged with too: every image reads the
    # same DOCKER_BUILD_VERSION when set, and the per-image version files match.
    if docker image inspect "$GHCR_ORG/$ORG/$dep:$VERSION" >/dev/null 2>&1; then
        echo "$me: parent $GHCR_ORG/$ORG/$dep:$VERSION already present"
    elif docker pull "$GHCR_ORG/$ORG/$dep:$VERSION"; then
        echo "$me: pulled parent $GHCR_ORG/$ORG/$dep:$VERSION"
    else
        echo "$me: parent $GHCR_ORG/$ORG/$dep:$VERSION not published, building it"
        (cd "$dep_dir" && ./build.sh "${ORIG_ARGS[@]}")
    fi
done

BUILT=false
if [ "$SKIP_BUILD" = false ]; then
    # An image that already exists at this version does not need building. These
    # images fetch SDKs and toolchains from vendor servers while they build, so
    # a build that does not happen is also an unrelated vendor outage that
    # cannot fail the run. --no-cache means a rebuild was asked for explicitly.
    if [ "$NO_CACHE" = false ] && docker image inspect "$GHCR_ORG/$ORG/$IMAGE:$VERSION" >/dev/null 2>&1; then
        echo "$me: $GHCR_ORG/$ORG/$IMAGE:$VERSION already present, not rebuilding"
    elif [ "$NO_CACHE" = false ] && docker pull "$GHCR_ORG/$ORG/$IMAGE:$VERSION"; then
        echo "$me: $GHCR_ORG/$ORG/$IMAGE:$VERSION already published, not rebuilding"
    else
        # Seed the layer cache from the previous version. Most of what changes
        # between versions is one step, so the layers before it are often still
        # valid. Only meaningful for numeric versions, and only once the
        # previous image carries inline cache metadata, which is why the build
        # below records it for next time.
        CACHE_ARGS=()
        # 10# forces base ten: bash reads a leading zero as octal, so "08" and
        # "09" are errors and "010" would quietly mean 8. There is also no
        # predecessor to version zero.
        if [[ $VERSION =~ ^[0-9]+$ ]] && ((10#$VERSION > 0)); then
            PREVIOUS="$GHCR_ORG/$ORG/$IMAGE:$((10#$VERSION - 1))"
            if [ "$NO_CACHE" = false ] && docker pull "$PREVIOUS"; then
                CACHE_ARGS=(--cache-from "$PREVIOUS")
            fi
        fi
        docker build "${BUILD_ARGS[@]}" "${CACHE_ARGS[@]}" --build-arg BUILDKIT_INLINE_CACHE=1 --platform="$TARGET_PLATFORM_TYPE" --build-arg TARGETPLATFORM="$TARGET_PLATFORM_TYPE" --build-arg VERSION="$VERSION" -t "$GHCR_ORG/$ORG/$IMAGE:$VERSION" .
        BUILT=true
    fi
    docker image prune --force
fi

if [ "$LATEST" = true ]; then
    docker tag "$GHCR_ORG"/"$ORG"/"$IMAGE":"$VERSION" "$GHCR_ORG"/"$ORG"/"$IMAGE":latest
fi

if [ "$SQUASH" = true ]; then
    command -v docker-squash >/dev/null &&
        docker-squash "$GHCR_ORG"/"$ORG"/"$IMAGE":"$VERSION" -t "$GHCR_ORG"/"$ORG"/"$IMAGE":latest
fi

if [ "$PUSH" = true ]; then
    docker push "$GHCR_ORG"/"$ORG"/"$IMAGE":"$VERSION"
    if [ "$LATEST" = true ]; then
        docker push "$GHCR_ORG"/"$ORG"/"$IMAGE":latest
    fi
fi

if [ "$CLEAR" = true ]; then
    docker rmi -f "$GHCR_ORG"/"$ORG"/"$IMAGE":"$VERSION"
    if [ "$LATEST" = true ]; then
        docker rmi -f "$GHCR_ORG"/"$ORG"/"$IMAGE":latest
    fi
fi

docker images --filter=reference="$GHCR_ORG/$ORG/*"

if [ -f "$DOCKER_VOLUME_PATH" ]; then
    df -h "$DOCKER_VOLUME_PATH"
    mb_space_after=$(df -m "$DOCKER_VOLUME_PATH" | awk 'FNR==2{print $3}')
    printf "%'.f MB total used\n" "$((mb_space_before - mb_space_after))"
fi

exit 0
