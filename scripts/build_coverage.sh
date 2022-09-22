#!/usr/bin/env bash

#
# Copyright (c) 2022 Project CHIP Authors
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

set -e

_install_lcov() {
    if ! lcov --version >/dev/null 2>&1; then
        echo "lcov not installed. Installing..."
        case "$(uname)" in
            "Darwin")
                brew install lcov
                ;;
            "Linux")
                sudo apt-get update
                sudo apt-get install -y lcov
                ;;
            *)
                die
                ;;
        esac
    fi
}

_install_lcov

_normpath() {
    python3 -c "import os.path; print(os.path.normpath('$@'))"
}

CHIP_ROOT=$(_normpath "$(dirname "$0")/..")
OUTPUT_ROOT="$CHIP_ROOT/out/coverage"
COVERAGE_ROOT="$OUTPUT_ROOT/coverage"
skip_gn=false

help() {

    echo "Usage: $file_name [ options ... ]"

    echo "General Options:
  -h, --help                Display this information.
Input Options:
  -o, --output_root         Set the build output directory.  When set manually, performs only lcov stage
                            on provided build output.  Assumes output_root has been built with 'use_coverage=true'
                            and that 'ninja check' was run.
  "
}

file_name=${0##*/}

while (($#)); do
    case $1 in
        --help | -h)
            help
            exit 1
            ;;
        --output_root | -o)
            OUTPUT_ROOT=$2
            COVERAGE_ROOT="$OUTPUT_ROOT/coverage"
            skip_gn=true
            shift
            ;;
        -*)
            help
            echo "Unknown Option \"$1\""
            exit 1
            ;;
    esac
    shift
done

# Ensure we have a compilation environment
source "$CHIP_ROOT/scripts/activate.sh"

# Generates ninja files
if [ "$skip_gn" == false ]; then
    gn --root="$CHIP_ROOT" gen "$OUTPUT_ROOT" --args='use_coverage=true'
    ninja -C "$OUTPUT_ROOT" check
fi

echo "PWD:$PWD"
echo "CHIP_ROOT:$CHIP_ROOT"
echo "OUTPUT_ROOT:$OUTPUT_ROOT"

rm -rf "$OUTPUT_ROOT/obj/src/access/tests"
rm -rf "$OUTPUT_ROOT/obj/src/app/tests"
rm -rf "$OUTPUT_ROOT/obj/src/ble/tests"
rm -rf "$OUTPUT_ROOT/obj/src/controller/tests"
rm -rf "$OUTPUT_ROOT/obj/src/credentials/tests"
rm -rf "$OUTPUT_ROOT/obj/src/crypto/tests"
rm -rf "$OUTPUT_ROOT/obj/src/inet/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/asn1/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/core/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/dnssd/minimal_mdns/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/dnssd/minimal_mdns/core/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/dnssd/minimal_mdns/records/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/dnssd/minimal_mdns/responders/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/dnssd/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/shell/tests"
rm -rf "$OUTPUT_ROOT/obj/src/lib/support/tests"
rm -rf "$OUTPUT_ROOT/obj/src/messaging/tests"
rm -rf "$OUTPUT_ROOT/obj/src/platform/tests"
rm -rf "$OUTPUT_ROOT/obj/src/protocols/bdx/tests"
rm -rf "$OUTPUT_ROOT/obj/src/protocols/secure_channel/tests"
rm -rf "$OUTPUT_ROOT/obj/src/protocols/user_directed_commissioning/tests"
rm -rf "$OUTPUT_ROOT/obj/src/setup_payload/tests"
rm -rf "$OUTPUT_ROOT/obj/src/system/tests"
rm -rf "$OUTPUT_ROOT/obj/src/transport/tests"
rm -rf "$OUTPUT_ROOT/obj/src/transport/raw/tests"
rm -rf "$OUTPUT_ROOT/obj/src/transport/retransmit/tests"

mkdir -p "$COVERAGE_ROOT"
lcov --initial --capture --directory "$OUTPUT_ROOT/obj/src" --exclude="$PWD"/zzz_generated/* --exclude="$PWD"/third_party/* --exclude=/usr/include/* --output-file "$COVERAGE_ROOT/lcov_base.info"
lcov --capture --directory "$OUTPUT_ROOT/obj/src" --exclude="$PWD"/zzz_generated/* --exclude="$PWD"/third_party/* --exclude=/usr/include/* --output-file "$COVERAGE_ROOT/lcov_test.info"
lcov --add-tracefile "$COVERAGE_ROOT/lcov_base.info" --add-tracefile "$COVERAGE_ROOT/lcov_test.info" --output-file "$COVERAGE_ROOT/lcov_final.info"
genhtml "$COVERAGE_ROOT/lcov_final.info" --output-directory "$COVERAGE_ROOT/html"
