#!/usr/bin/env bash

#
# clang-format does not return a non-zero exit code.  This wrapper
# exits with a non-zero exit code if clang-format outputs any
# replacements.
#

set -x

die() {
    echo " *** ERROR: " $*
    exit 1
}

# from `man diff`:
# Exit status is 0 if inputs are the same, 1 if different, 2 if trouble.

$(dirname "$0")/clang-format.sh -style=file $@  | diff -u $@ - || die

for arg; do true; done
file=$arg
[ -n "$(tail -c1 $file)" ] && {
    echo " *** ERROR: Missing EOF newline: " $file
    exit 1
}

exit 0
