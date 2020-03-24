#!/usr/bin/env bash

#
# clang-format does not return a non-zero exit code.  This wrapper
# exits with a non-zero exit code if clang-format outputs any
# replacements.
#

die() {
    echo " *** ERROR: $*"
    exit 1
}

# from `man diff`:
# Exit status is 0 if inputs are the same, 1 if different, 2 if trouble.

"$(dirname "$0")"/clang-format.sh -style=file "$@"  | diff -u "$@" - || die "diffs exist"

for arg; do true; done
file=$arg
[[ -z $(tail -c1 "$file") ]] || die "Missing EOF newline: $file"

exit 0
