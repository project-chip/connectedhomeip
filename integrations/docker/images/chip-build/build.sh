#!/usr/bin/env bash
set -ex

cd "$(dirname "$0")"

die() {
    echo " *** ERROR: $*"
    exit 1
}

# FIX ME, should be "projectchip" or "zigbee" or something,
#   (docker.io doesn't allow hyphens)
ORG=${DOCKER_BUILD_ORG:-rwalkerapple}

# directory name is
IMAGE=${DOCKER_BUILD_IMAGE:-$(basename "$(pwd)")}

# version
VERSION=${DOCKER_BUILD_VERSION:-$(cat version)}

[[ -n $VERSION ]] || die "version cannot be empty"

docker build -t "$ORG/$IMAGE:$VERSION" .

[[ ${*/--latest//} != "${*}" ]] && {
  docker tag "$ORG"/"$IMAGE":"$VERSION" "$ORG"/"$IMAGE":latest
}

[[ ${*/--push//} != "${*}" ]] && {
  docker push "$ORG"/"$IMAGE":"$VERSION"
  [[ ${*/--latest//} != "${*}" ]] && {
    docker push "$ORG"/"$IMAGE":latest
  }
}

exit 0
