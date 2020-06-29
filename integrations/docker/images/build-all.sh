#!/usr/bin/env bash

#
# temporary wrapper build script until we can build something better
#  https://github.com/project-chip/connectedhomeip/issues/710
#

find . -name Dockerfile | while read -r dockerfile; do
    dir=${dockerfile%/*}
    (cd "$dir" && ./build.sh) || exit $?
done
