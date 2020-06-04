#!/usr/bin/env bash

set -x

if [[ $1 == *"$2"* ]]; then
    shift
    shift
    "$@"
fi
