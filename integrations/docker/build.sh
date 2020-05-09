#!/usr/bin/env bash

# build.sh   - utility for building (and optionally) tagging and pushing
#               the a Docker image
#
# This script expects to find a Dockerfile next to $0, so symlink
#  in an image name directory is the expected use case.

me=$(basename "$0")
cd "$(dirname "$0")"

ORG=${DOCKER_BUILD_ORG:-connectedhomeip}

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

BUILD_ARGS=""
if [[ ${*/--no-cache//} != "${*}" ]]; then
  BUILD_ARGS+=" --no-cache "
fi

docker build "$BUILD_ARGS" -t "$ORG/$IMAGE:$VERSION" .

[[ ${*/--latest//} != "${*}" ]] && {
  docker tag "$ORG"/"$IMAGE":"$VERSION" "$ORG"/"$IMAGE":latest
}

[[ ${*/--push//} != "${*}" ]] && {
  docker push "$ORG"/"$IMAGE":"$VERSION"
  [[ ${*/--latest//} != "${*}" ]] && {
    docker push "$ORG"/"$IMAGE":latest
  }
}

for filename in ./variants/*; do
  echo "Variant Filename: $filename"
  VARIANT=${filename##*.}
  docker build "$BUILD_ARGS" -f "$filename" --build-arg VERSION="$VERSION" -t "$ORG/$IMAGE-$VARIANT:$VERSION" ./variants/

  [[ ${*/--latest//} != "${*}" ]] && {
    docker tag "$ORG"/"$IMAGE-$VARIANT":"$VERSION" "$ORG"/"$IMAGE-$VARIANT":latest
  }

  [[ ${*/--push//} != "${*}" ]] && {
    docker push "$ORG"/"$IMAGE-$VARIANT":"$VERSION"
    [[ ${*/--latest//} != "${*}" ]] && {
      docker push "$ORG"/"$IMAGE-$VARIANT":latest
    }
  }

done

exit 0
