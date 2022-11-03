#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

#
# temporary wrapper build script until we can build something better
#  https://github.com/project-chip/connectedhomeip/issues/710
#
set -e
find "$(git rev-parse --show-toplevel)"/integrations/docker/images/ -name Dockerfile ! -path "*chip-cert-bins/*" | while read -r dockerfile; do
    pushd "$(dirname "$dockerfile")" >/dev/null
    ./build.sh "$@"
    popd >/dev/null
done
