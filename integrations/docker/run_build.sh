#!/usr/bin/env bash
# Note: this needs to be run from the root directory presently. 
# https://github.com/project-chip/connectedhomeip/issues/273

./bootstrap && mkdir -p build/default && (cd build/default && ../../configure --enable-coverage --enable-coverage-info) && make -C build/default distcheck
