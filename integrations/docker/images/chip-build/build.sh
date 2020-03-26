#!/usr/bin/env bash
set -ex

cd "$(dirname "$0")"

die() {
    echo " *** ERROR: $*"
    exit 1
}

# fix me, should be "projectchip" or "zigbee" or something,
#   since docker.io doesn't allow hyphens
ORG=rwalkerapple

# directory name is
IMAGE=$(basename "$(pwd)")

# version
VERSION=$(cat version)

[[ -n $VERSION ]] || die "version cannot be empty"

docker build -t "$ORG/$IMAGE:$VERSION" .

[[ ${*/--latest//} != "${*}" ]] && {
  docker tag "$ORG"/"$IMAGE":latest "$ORG"/"$IMAGE":"$VERSION"
}

[[ ${*/--push//} != "${*}" ]] && {
  docker push "$ORG"/"$IMAGE":"$VERSION"
  [[ ${*/--latest//} != "${*}" ]] && {
    docker push "$ORG"/"$IMAGE":latest
  }
}
