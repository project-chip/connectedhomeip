#!/usr/bin/env bash

set -x
env

make -f Makefile-bootstrap clean-repos && git clean -Xdf
