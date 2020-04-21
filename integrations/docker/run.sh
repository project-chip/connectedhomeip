#!/usr/bin/env bash

# run.sh   - utility for running a Docker image
#
# This script expects to live in a directory named after the image
#  with a version file next to it.  So: use symlinks
#
here=$(cd "$(dirname "$0")" && pwd)
me=$(basename "$0")

die() {
  echo "$me: *** ERROR: $*"
  exit 1
}

ORG=${DOCKER_RUN_ORG:-connectedhomeip}

# directory name is
IMAGE=${DOCKER_RUN_IMAGE:-$(basename "$here")}

# version
VERSION=${DOCKER_RUN_VERSION:-$(cat "$here/version")} ||
  die "please run me from an image directory or set environment variables:
          DOCKER_RUN_ORG
          DOCKER_RUN_IMAGE
          DOCKER_RUN_VERSION"

# where
RUN_DIR=${DOCKER_RUN_DIR:-$(pwd)}

help() {
  set +x
  echo "Usage: $me [RUN_OPTIONS -- ] command

  Run a command in a docker image described by $here

  Options:
   --help        get this message

  Any number of 'docker run' options can be passed
     through to the invocation.  Terminate this list of
     options with '--' to begin command and arguments.

  Examples:
    To run bash interactively:
      $ $me -i -- bash
     note the terminating '--' for run options

    To just tell me about the image
      $ $me uname -a

    Add /tmp as an additional volumeand run make
      $ $me --volume /tmp:/tmp -- make -C src

"

}

runargs=()

# extract run options
for arg in "$@"; do
  case "$arg" in
    --help)
      help
      exit
      ;;

    --)
      shift
      break
      ;;

    -*)
      runargs+=("$arg")
      shift
      ;;

    *)
      ((!${#runargs[*]})) && break
      runargs+=("$arg")
      shift
      ;;

  esac
done

docker run "${runargs[@]}" --rm -w "$RUN_DIR" -v "$RUN_DIR:$RUN_DIR" "$ORG/$IMAGE${VERSION:+:${VERSION}}" "$@"
