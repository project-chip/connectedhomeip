#!/usr/bin/env bash

me=$(basename "$0")
cd "$(dirname "$0")"

# FIX ME, should be "projectchip" or "zigbee" or something,
#   (docker.io doesn't allow hyphens)
ORG=${DOCKER_BUILD_ORG:-rwalkerapple}

# directory name is
IMAGE=${DOCKER_BUILD_IMAGE:-$(basename "$(pwd)")}

# version
VERSION=${DOCKER_BUILD_VERSION:-$(cat version)}

[[ ${*/--help//} != "${*}" ]] && {
  set +x
  echo "Usage: $me <OPTIONS>

  Build and (optionally tag as latest, push) a docker image from Dockerfile in CWD

  Options:
   --latest   update latest to the current built version (\"$VERSION\")
   --push     push image(s) to docker.io (requires docker login for \"$ORG\")
   --help     get this message

"
  exit 0
}

die() {
    echo "$me: *** ERROR: $*"
    exit 1
}

set -ex

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
