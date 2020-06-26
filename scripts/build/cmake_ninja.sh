#!/usr/bin/env bash

build_dir=build/default

if [[ ! -f $build_dir/build.ninja ]]; then
    mkdir -p "$build_dir"
    (cd "$build_dir" &&
        cmake -GNinja ../.. "$@" &&
        ninja)
else
    (cd "$build_dir" &&
        ninja)
fi
