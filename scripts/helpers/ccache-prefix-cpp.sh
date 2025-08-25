#!/bin/bash

# Add -P flag to get rid of #line directives which break caching
args=()
for arg in "$@"; do
    args+=("$arg")
    [[ "$arg" == "-E" ]] && args+=("-P")
done

exec "${args[@]}"
