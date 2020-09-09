#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Build script for GN examples GitHub workflow.

CHIP_ROOT="$(dirname "$0")/../.."

source "$CHIP_ROOT/scripts/activate.sh"

GN_ARGS=()

EXAMPLE_DIR=$1
shift
OUTPUT_DIR=$1
shift

NINJA_VERBOSE=
for arg; do
    case $arg in
        -v)
            NINJA_VERBOSE=-v
            ;;
        *=*)
            GN_ARGS+=("$arg")
            ;;
        *)
            echo >&2 "invalid argument: $arg"
            exit 2
            ;;
    esac
done

set -e
set -x
env

gn gen --root="$EXAMPLE_DIR" "$OUTPUT_DIR" --args="${GN_ARGS[*]}"

ninja "$NINJA_VERBOSE" -C "$OUTPUT_DIR"
