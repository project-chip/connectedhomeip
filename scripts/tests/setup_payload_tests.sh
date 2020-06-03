#!/usr/bin/env bash

env

make V=1 -C build/default/src/setup_payload/tests TestQRCode && build/default/src/setup_payload/tests/TestQRCode
make V=1 -C build/default/src/setup_payload check
