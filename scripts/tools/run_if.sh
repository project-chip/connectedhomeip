#!/bin/bash

if [[ $1 == *"$2"* ]]; then
    shift
    shift
    "$@"
fi
