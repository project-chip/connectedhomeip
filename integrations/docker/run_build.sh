#!/usr/bin/env bash

./bootstrap && mkdir -p build/default && (cd build/default && ../../configure --enable-coverage --enable-coverage-info) && make -C build/default distcheck
