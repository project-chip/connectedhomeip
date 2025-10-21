#!/bin/sh
# Add -P flag to get rid of #line directives which break caching
exec "$@" -P
