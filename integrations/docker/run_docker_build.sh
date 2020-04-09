#!/usr/bin/env bash

set -e
here=$(cd "$(dirname "$0")" && pwd)
# update me if I move out of integrations/docker
#         |
#         +---------------+
#                         V
SOURCE_PATH=$(cd "${here}/../.." && pwd)

ORGANIZATION="connectedhomeip"
IMAGE="chip-build"
VERSION=$(cat "${here}/images/${IMAGE}/version")

docker run --rm -w "${SOURCE_PATH}" -v "${SOURCE_PATH}:${SOURCE_PATH}" "${ORGANIZATION}/${IMAGE}:${VERSION}" integrations/docker/run_build.sh "${@:-distcheck}"
