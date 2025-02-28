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
CODE="core"

skip_gn=false
TEST_TARGET=check

# By default, do not run YAML or Python tests
ENABLE_YAML=false
ENABLE_PYTHON=false

help() {
    echo "Usage: $file_name [--output_root=<output_root>] [--code=<core|clusters|all>] [Test options"
    echo
    echo "Misc:"
    echo "    -h, --help              Print this help, then exit."
    echo
    echo "Build/Output options:"
    echo "    -o, --output_root=DIR   Set the build output directory."
    echo "                            When set manually, script only runs lcov on the provided build output."
    echo "                            This directory must be built with 'use_coverage=true' and 'ninja check' must have run."
    echo
    echo "    -c, --code=TYPE         Specify which scope to collect coverage data. One of:"
    echo "        core      - (default) coverage from core stack in Matter SDK."
    echo "        clusters  - coverage from cluster implementations in Matter SDK."
    echo "        all       - coverage from entire Matter SDK."
    echo
    echo "Test options:"
    echo "    --yaml                  In addition to unit tests, run YAML-based tests."
    echo "    --python                In addition to unit tests, run Python-based tests."
    echo "                            Both can be combined if needed."
    echo
    echo "    --target=TARGET         Specific test target to run for unit tests (e.g. 'TestEmberAttributeBuffer.run')."
    echo
}

file_name=${0##*/}

# ------------------------------------------------------------------------------
# Parse arguments
# ------------------------------------------------------------------------------
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
        --target=*)
            TEST_TARGET="${i#*=}"
            shift
            ;;
        -o=* | --output_root=*)
            OUTPUT_ROOT="${i#*=}"
            COVERAGE_ROOT="$OUTPUT_ROOT/coverage"
            skip_gn=true
            shift
            ;;
        --yaml)
            ENABLE_YAML=true
            shift
            ;;
        --python)
            ENABLE_PYTHON=true
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

# Validate code argument
if [[ ! " ${SUPPORTED_CODE[@]} " =~ " ${CODE} " ]]; then
    echo "ERROR: Code $CODE not supported"
    exit 1
fi

# ------------------------------------------------------------------------------
# Build & Test
# ------------------------------------------------------------------------------
if [ "$skip_gn" == false ]; then
    # Ensure environment is set
    source "$CHIP_ROOT/scripts/activate.sh"

    # Generate ninja files
    EXTRA_GN_ARGS=""

    # We only need 'chip_build_all_clusters_app' if we run YAML tests
    if [ "$ENABLE_YAML" == true ]; then
        EXTRA_GN_ARGS="$EXTRA_GN_ARGS chip_build_all_clusters_app=true"
    else
        # Otherwise skip building tools
        EXTRA_GN_ARGS="$EXTRA_GN_ARGS chip_build_tools=false"
    fi

    gn --root="$CHIP_ROOT" gen "$OUTPUT_ROOT" --args="use_coverage=true $EXTRA_GN_ARGS"

    #
    # 1) Always run unit tests
    #
    ninja -C "$OUTPUT_ROOT" "$TEST_TARGET"

    #
    # 2) Run YAML tests if requested
    #
    if [ "$ENABLE_YAML" == true ]; then
        ninja -C "$OUTPUT_ROOT"

        scripts/run_in_build_env.sh \
            "./scripts/tests/run_test_suite.py \
             --runner chip_tool_python \
             --exclude-tags MANUAL \
             --exclude-tags FLAKY \
             --exclude-tags IN_DEVELOPMENT \
             --exclude-tags EXTRA_SLOW \
             --exclude-tags SLOW \
             --exclude-tags PURPOSEFUL_FAILURE \
             --chip-tool \"$OUTPUT_ROOT/chip-tool\" \
             --target TestUserLabelCluster \
             run \
             --iterations 1 \
             --test-timeout-seconds 120 \
             --all-clusters-app \"$OUTPUT_ROOT/chip-all-clusters-app\" \
            "
    fi

    #
    # 3) Run Python tests if requested
    #
    if [ "$ENABLE_PYTHON" == true ]; then
        echo "Running Python tests ..."
        # TODO: run python tests.
    fi

    # ----------------------------------------------------------------------------
    # Remove objects we do NOT want included in coverage
    # ----------------------------------------------------------------------------
    rm -rf "$OUTPUT_ROOT/obj/src/app/app-platform"
    rm -rf "$OUTPUT_ROOT/obj/src/app/common"
    rm -rf "$OUTPUT_ROOT/obj/src/app/util/mock"
    rm -rf "$OUTPUT_ROOT/obj/src/controller/python"
    rm -rf "$OUTPUT_ROOT/obj/src/lib/dnssd/platform"
    rm -rf "$OUTPUT_ROOT/obj/src/lib/shell"
    rm -rf "$OUTPUT_ROOT/obj/src/lwip"
    rm -rf "$OUTPUT_ROOT/obj/src/platform"
    rm -rf "$OUTPUT_ROOT/obj/src/tools"

    # Remove unit test objects from coverage
    find "$OUTPUT_ROOT/obj/src/" -depth -name 'tests' -exec rm -rf {} \;

    # Restrict coverage to 'core' or 'clusters' if specified
    if [ "$CODE" == "core" ]; then
        rm -rf "$OUTPUT_ROOT/obj/src/app/clusters"
    elif [ "$CODE" == "clusters" ]; then
        mv "$OUTPUT_ROOT/obj/src/app/clusters" "$OUTPUT_ROOT/obj/clusters"
        rm -rf "$OUTPUT_ROOT/obj/src"
        mkdir -p "$OUTPUT_ROOT/obj/src"
        mv "$OUTPUT_ROOT/obj/clusters" "$OUTPUT_ROOT/obj/src/clusters"
    fi
fi

# ------------------------------------------------------------------------------
# Coverage Generation
# ------------------------------------------------------------------------------
mkdir -p "$COVERAGE_ROOT"

lcov --initial --capture --directory "$OUTPUT_ROOT/obj/src" \
    --ignore-errors inconsistent \
    --exclude="$PWD"/zzz_generated/* \
    --exclude="$PWD"/third_party/* \
    --exclude=/usr/include/* \
    --output-file "$COVERAGE_ROOT/lcov_base.info"

lcov --capture --directory "$OUTPUT_ROOT/obj/src" \
    --ignore-errors inconsistent \
    --exclude="$PWD"/zzz_generated/* \
    --exclude="$PWD"/third_party/* \
    --exclude=/usr/include/* \
    --output-file "$COVERAGE_ROOT/lcov_test.info"

lcov --add-tracefile "$COVERAGE_ROOT/lcov_base.info" \
    --add-tracefile "$COVERAGE_ROOT/lcov_test.info" \
    --ignore-errors inconsistent \
    --output-file "$COVERAGE_ROOT/lcov_final.info"

genhtml "$COVERAGE_ROOT/lcov_final.info" \
    --ignore-errors inconsistent \
    --output-directory "$COVERAGE_ROOT/html" \
    --title "SHA:$(git rev-parse HEAD)" \
    --header-title "Matter SDK Coverage Report"

cp "$CHIP_ROOT/integrations/appengine/webapp_config.yaml" \
    "$COVERAGE_ROOT/webapp_config.yaml"
