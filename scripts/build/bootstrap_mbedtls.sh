#!/bin/bash

git clean -xdf
mkdir -p build/default (cd build/default && ../../bootstrap-configure --enable-debug --enable-coverage --with-crypto=mbedtls)
