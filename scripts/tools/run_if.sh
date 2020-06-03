#!/usr/bin/env bash

set -x
env

if [[ $1 == *"$2"* ]]; then
    shift
    shift
    "$@"
fi
