#!/usr/bin/env bash

./bootstrap && mkdir -p build/default && (cd build/default && ../../configure --enable-coverage --enable-coverage-info) && ./bootstrap -w make && make -C build/default distcheck
