#!/usr/bin/env bash

set -x
env

mkdir -p "$1"

for name in $(find . -name test-suite.log); do
    destination="$1"/$(echo "$name" | sed -E s-\^.\/-- | sed s-/-_-g)
    echo "Saving $name to $destination"
    cp "$name" "$destination"
done
