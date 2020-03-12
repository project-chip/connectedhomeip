#!/usr/bin/env bash
set -e

here="$(dirname "$0")"

MAKE_TARGET="${1:-all}"

docker run --rm -v "${PWD}/$here/..":/tmp/chip rsmmr/clang make -C /tmp/chip "${MAKE_TARGET}"
