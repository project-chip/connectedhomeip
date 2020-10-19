#!/usr/bin/env bash

# This function is a workaround for the fact that ninja sends all
# output, including compiler errors, to stdout, not stderr.  We want
# to suppress the various output from ninja itself, but not compiler
# errors.  Do that by prefixing the ninja output with a string that is
# unlikely to appear in compiler errors, then filtering it out.
function run_ninja() {
    prefix="|ninja_filter_prefix|"
    NINJA_STATUS="$prefix[%f/%t] " ninja "$@" | grep -v "^$prefix"
}
