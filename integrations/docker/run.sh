#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

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

ORG=${DOCKER_RUN_ORG:-project-chip}

GHCR_ORG="ghcr.io"

# directory name is
IMAGE=${DOCKER_RUN_IMAGE:-$(basename "$here")}

# version
VERSION=${DOCKER_RUN_VERSION:-$(sed 's/ .*//' "$here/version")} ||
    die "please run me from an image directory or set environment variables:
          DOCKER_RUN_ORG
          DOCKER_RUN_IMAGE
          DOCKER_RUN_VERSION"

# full image name
FULL_IMAGE_NAME="$GHCR_ORG/$ORG/$IMAGE${VERSION:+:${VERSION}}"

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

    Add /tmp as an additional volume and run make
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

docker pull "$FULL_IMAGE_NAME" || "$here"/build.sh

docker run "${runargs[@]}" --rm --mount "source=/var/run/docker.sock,target=/var/run/docker.sock,type=bind" -w "$RUN_DIR" -v "$RUN_DIR:$RUN_DIR" "$FULL_IMAGE_NAME" "$@"
