#!/bin/bash


ls -R src/app > foo
cat foo

make V=1 -C build/default
