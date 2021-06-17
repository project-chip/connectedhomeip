#!/bin/bash

set -e

if [ $# -lt 1 ]; then
    echo "Illegal number of parameters. Please pass lava result"
    exit 1
fi

mapfile -t results <$1

if [ ${#results[@]} -lt 2 ]; then
    echo "Wrong lava results format"
    exit 1
fi

unset results[0] # removes the header of the results

for result in "${results[@]}"; do
    echo "$result"
    if [[ $result != *"[pass]"* ]]; then
        exit 1
    fi
done
