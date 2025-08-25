#!/bin/bash

cmd="$@"

# Add -P flag to get rid of #line directives which break caching
cmd="${cmd/" -E "/" -E -P "}"

eval "$cmd"
