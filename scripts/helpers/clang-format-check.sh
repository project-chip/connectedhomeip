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

"$(dirname "$0")"/clang-format.sh -style=file "$@" | diff -u "$@" - || die "diffs exist"

for arg; do true; done
file=$arg
[[ -z $(tail -c1 "$file") ]] || die "Missing EOF newline: $file"

exit 0
