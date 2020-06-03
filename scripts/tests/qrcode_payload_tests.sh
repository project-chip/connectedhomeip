#!/usr/bin/env bash

set -x
env

make V=1 -C build/default/src/setup_payload/tests/ TestQRCode
