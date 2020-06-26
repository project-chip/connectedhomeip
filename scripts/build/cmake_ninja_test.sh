#!/usr/bin/env bash

build_dir=build/default

(cd "$build_dir" && ninja test)
