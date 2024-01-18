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
SUPPORTED_CODE=(core clusters all)
SUPPORTED_TESTS=(unit yaml all)
CODE="core"
TESTS="unit"
skip_gn=false

help() {

    echo "Usage: $file_name [--output_root=<output_root>] [--code=<core|clusters|all>] [--tests=<unit|yaml|all>]"
    echo
    echo "Misc:
  -h, --help                Print this help, then exit."
    echo
    echo "Options:
  -o, --output_root         Set the build output directory.  When set manually, performs only lcov stage
                            on provided build output.  Assumes output_root has been built with 'use_coverage=true'
                            and that 'ninja check' was run.
  -c, --code                Specify which scope to collect coverage data.
                            'core': collect coverage data from core stack in Matter SDK. --default
                            'clusters': collect coverage data from clusters implementation in Matter SDK.
                            'all': collect coverage data from Matter SDK.
  -t, --tests               Specify which tools to run the coverage check.
                            'unit': Run unit test to drive the coverage check. --default
                            'yaml': Run yaml test to drive the coverage check.
                            'all': Run unit & yaml test to drive the coverage check.
  "
}

file_name=${0##*/}

for i in "$@"; do
    case $i in
        -h | --help)
            help
            exit 1
            ;;
        -c=* | --code=*)
            CODE="${i#*=}"
            shift
            ;;
        -t=* | --tests=*)
            TESTS="${i#*=}"
            shift
            ;;
        -o=* | --output_root=*)
            OUTPUT_ROOT="${i#*=}"
            COVERAGE_ROOT="$OUTPUT_ROOT/coverage"
            skip_gn=true
            shift
            ;;
        *)
            echo "Unknown Option \"$1\""
            echo
            help
            exit 1
            ;;
    esac
done

if [[ ! " ${SUPPORTED_CODE[@]} " =~ " ${CODE} " ]]; then
    echo "ERROR: Code $CODE not supported"
    exit 1
fi

if [[ ! " ${SUPPORTED_TESTS[@]} " =~ " ${TESTS} " ]]; then
    echo "ERROR: Tests $TESTS not supported"
    exit 1
fi

if [ "$skip_gn" == false ]; then
    # Ensure we have a compilation environment
    source "$CHIP_ROOT/scripts/activate.sh"

    # Generates ninja files
    EXTRA_GN_ARGS=""
    if [[ "$TESTS" == "yaml" || "$TESTS" == "all" ]]; then
      EXTRA_GN_ARGS="$EXTRA_GN_ARGS chip_build_all_clusters_app=true"
    else
        EXTRA_GN_ARGS="$EXTRA_GN_ARGS chip_build_tools=false"
    fi
    gn --root="$CHIP_ROOT" gen "$OUTPUT_ROOT" --args="use_coverage=true$EXTRA_GN_ARGS"
    ninja -C "$OUTPUT_ROOT"

    # Run unit tests
    if [[ "$TESTS" == "unit" || "$TESTS" == "all" ]]; then
        ninja -C "$OUTPUT_ROOT" check
    fi

    # Run yaml tests
    if [[ "$TESTS" == "yaml" || "$TESTS" == "all" ]]; then
        scripts/run_in_build_env.sh \
            "./scripts/tests/run_test_suite.py \
             --chip-tool ""$OUTPUT_ROOT/chip-tool \
             run \
             --iterations 1 \
             --test-timeout-seconds 120 \
             --all-clusters-app ""$OUTPUT_ROOT/chip-all-clusters-app
          "
    fi

    # Remove misc support components from coverage statistics
    rm -rf "$OUTPUT_ROOT/obj/src/app/app-platform"
    rm -rf "$OUTPUT_ROOT/obj/src/app/common"
    rm -rf "$OUTPUT_ROOT/obj/src/app/util/mock"
    rm -rf "$OUTPUT_ROOT/obj/src/controller/python"
    rm -rf "$OUTPUT_ROOT/obj/src/lib/dnssd/platform"
    rm -rf "$OUTPUT_ROOT/obj/src/lib/shell"
    rm -rf "$OUTPUT_ROOT/obj/src/lwip"
    rm -rf "$OUTPUT_ROOT/obj/src/platform"
    rm -rf "$OUTPUT_ROOT/obj/src/tools"

    # Remove unit test itself from coverage statistics
    find "$OUTPUT_ROOT/obj/src/" -depth -name 'tests' -exec rm -rf {} \;

    if [ "$CODE" == "core" ]; then
        rm -rf "$OUTPUT_ROOT/obj/src/app/clusters"
    elif [ "$CODE" == "clusters" ]; then
        mv "$OUTPUT_ROOT/obj/src/app/clusters" "$OUTPUT_ROOT/obj/clusters"
        rm -rf "$OUTPUT_ROOT/obj/src"
        mkdir "$OUTPUT_ROOT/obj/src"
        mv "$OUTPUT_ROOT/obj/clusters" "$OUTPUT_ROOT/obj/src/clusters"
    fi
fi

mkdir -p "$COVERAGE_ROOT"
lcov --initial --capture --directory "$OUTPUT_ROOT/obj/src" --exclude="$PWD"/zzz_generated/* --exclude="$PWD"/third_party/* --exclude=/usr/include/* --output-file "$COVERAGE_ROOT/lcov_base.info"
lcov --capture --directory "$OUTPUT_ROOT/obj/src" --exclude="$PWD"/zzz_generated/* --exclude="$PWD"/third_party/* --exclude=/usr/include/* --output-file "$COVERAGE_ROOT/lcov_test.info"
lcov --add-tracefile "$COVERAGE_ROOT/lcov_base.info" --add-tracefile "$COVERAGE_ROOT/lcov_test.info" --output-file "$COVERAGE_ROOT/lcov_final.info"
genhtml "$COVERAGE_ROOT/lcov_final.info" --output-directory "$COVERAGE_ROOT/html"

# Copy webapp's YAML file to the coverage output directory
cp "$CHIP_ROOT/integrations/appengine/webapp_config.yaml" "$COVERAGE_ROOT/webapp_config.yaml"
