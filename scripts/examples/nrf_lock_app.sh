#!/usr/bin/env bash

set -x
env

make VERBOSE=1 -C examples/lock-app/nrf5
