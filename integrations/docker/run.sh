#!/usr/bin/env bash

# run.sh   - utility for running a Docker image
#
# This script expects to live in a directory named after the image
#  with a version file next to it.  So: use symlinks
#
here=$(cd "$(dirname "$0")" && pwd)
me=$(basename "$0")

ORG=${DOCKER_RUN_ORG:-connectedhomeip}

# directory name is
IMAGE=${DOCKER_RUN_IMAGE:-$(basename "$here")}

# version
VERSION=${DOCKER_RUN_VERSION:-$(cat "$here/version")}

# where
RUN_DIR=${DOCKER_RUN_DIR:-$(pwd)}

[[ ${*/--help//} != "${*}" ]] && {
  set +x
  echo "Usage: $me command

  Run a command in a docker image described by $here

  Options:
   --help     get this message

"
  exit 0
}

docker run --rm -w "$RUN_DIR" -v "$RUN_DIR:$RUN_DIR" "$ORG/$IMAGE:$VERSION" "$@"
