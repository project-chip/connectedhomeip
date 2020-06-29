#!/usr/bin/env bash

#
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

CLANG_FORMAT_VERSION="clang-format version 9.0"

die() {
    echo " *** ERROR: $*"
    exit 1
}

if command -v clang-format-9 >/dev/null; then
    alias clang-format=clang-format-9
elif command -v clang-format >/dev/null; then
    case "$(clang-format --version)" in
        "$CLANG_FORMAT_VERSION"*) ;;

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
for arg; do
    case $arg in
        -i)
            REPLACE=yes
            ;;
    esac
done

file=$arg

[[ $REPLACE != yes ]] || {
    [[ -z $(tail -c1 "$file") ]] || echo >>"$file"
}

exit 0
