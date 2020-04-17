#!/usr/bin/env bash
# This script assumes it's being run from the ToT

./bootstrap && mkdir -p build/default && (cd build/default && ../../configure --enable-coverage) && make -C build/default "${@:-distcheck}" && git clean -Xdf && (cd apps/lock/nrf5 && make)
