#!/usr/bin/env bash

set -x
env

make -C build/default pretty-check
