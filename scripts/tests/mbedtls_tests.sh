#!/usr/bin/env bash

set -x
env

make V=1 -C build/default/third_party/mbedtls
