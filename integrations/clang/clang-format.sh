#!/usr/bin/env bash

CLANG_FORMAT_VERSION="clang-format version 9.0"

die() {
    echo " *** ERROR: $*"
    exit 1
}

if command -v clang-format-9 > /dev/null; then
    alias clang-format=clang-format-9
elif command -v clang-format > /dev/null; then
    case "$(clang-format --version)" in
        "$CLANG_FORMAT_VERSION"*)
            ;;
        *)
            die "$(clang-format --version); \"$CLANG_FORMAT_VERSION\" required"
            ;;
    esac
else
    die "$CLANG_FORMAT_VERSION required"
fi

clang-format "$@" || die "format failed"

# ensure EOF newline
REPLACE=no
for arg
do
    case $arg in
        -i)
            REPLACE=yes
            ;;
    esac
done

file=$arg

[[ $REPLACE != yes ]] || {
    [[ -z $(tail -c1 "$file") ]] || echo >> "$file"
}

exit 0
