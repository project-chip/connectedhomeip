#!/usr/bin/env bash

env

make -f Makefile-bootstrap clean-repos && git clean -Xdf
