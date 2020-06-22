#!/usr/bin/env bash

set -x
env

make V=1 -C build/default/src/crypto/ && make V=1 -C build/default/src/crypto/tests/ TestCryptoPAL
