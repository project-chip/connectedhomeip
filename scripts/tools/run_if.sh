#!/usr/bin/env bash

env

if [[ $1 == *"$2"* ]]; then
    shift
    shift
    "$@"
fi
