#!/bin/bash

# scripts/build/bootstrap.sh --with-device-layer=linux

git clean -xdf
mkdir -p build/default
(cd build/default && ../../bootstrap-configure --enable-debug --with-device-layer=linux)
