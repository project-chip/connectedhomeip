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

function trim_whitespace() {
    local OLD_SHOPT_EXTGLOB="$(shopt -p extglob)"
    shopt -s extglob

    while IFS= read -r line; do                        # For every line in stdin
        local trimmed_line="${line##+([[:space:]])}"   # Remove leading whitespace
        trimmed_line="${trimmed_line%%+([[:space:]])}" # Remove trailing whitespace
        echo "$trimmed_line"                           # Output to stdout
    done

    eval "$OLD_SHOPT_EXTGLOB"
}

# This function parses the input from `ninja -t query` and extracts the input targets.
function parse_input_targets() {
    while IFS= read -r line; do
        if [[ "$line" == "input: "* ]]; then
            rule="${line#input: }"
            if [[ "$rule" == "phony" ]]; then
                in_input_block=true
            elif [[ -n "${POSSIBLE_RULES["$rule"]}" ]]; then
                RULES["$rule"]=1
            fi
        elif [[ "$in_input_block" == false ]]; then
            continue
        elif [[ "$line" == "outputs:" ]]; then
            in_input_block=false
        elif [[ "$in_input_block" == true && ! -n "${NEW_TARGETS["$line"]}" ]]; then
            NEW_TARGETS["$line"]=1
        fi
    done
}

# This function executes a command with lastpipe enabled, allowing the last command in a pipeline to run in the current shell.
function dowithlastpipe() {
    # Save the current lastpipe and monitor settings
    local original_lastpipe_setting="$(shopt -p lastpipe)"
    local original_monitor_setting
    if [[ "$-" == *m* ]]; then
        original_monitor_setting="set -m"
    else
        original_monitor_setting="set +m"
    fi

    # Make sure to restore the original settings on exit
    trap "
        eval \"$original_lastpipe_setting\";
        eval \"$original_monitor_setting\";
        trap - EXIT
    " EXIT

    # Enable lastpipe and monitor
    set +m
    shopt -s lastpipe

    # Execute the command with lastpipe enabled
    eval "$@"

    # Return the exit status of the last command
    return "$?"
}

# This function returns the rules that should be cleaned based on the targets provided.
# Based on this, ninja -t clean -r <rules> will clean the exact targets that need to be reexecuted.
function get_rules_to_clean() {
    declare -A RULES

    # Read the rules that execute (not build) targets from the toolchain.ninja file
    readarray -t POSSIBLE_RULES_ARR < <(ninja -C out/coverage -f toolchain.ninja -t rules | grep "__rule")

    declare -A POSSIBLE_RULES
    for rule in "${POSSIBLE_RULES_ARR[@]}"; do
        POSSIBLE_RULES["$rule"]=1
    done

    # This will hold the targets that are already "ninja -t query"-ed
    declare -A QUERIED_TARGETS

    # This will hold the targets that are to be queried
    declare -A TARGETS
    for target in "$@"; do
        TARGETS["$target"]=1
    done

    while [[ "${#TARGETS[@]}" -gt 0 ]]; do

        # If at any point we have 'all' in the targets, we can just return all rules
        if [[ -n ${TARGETS["all"]} ]]; then
            for rule in "${!POSSIBLE_RULES[@]}"; do
                echo "$rule"
            done
            exit 0
        fi

        # This will hold the results of the last query
        unset NEW_TARGETS
        declare -A NEW_TARGETS

        # Query the targets and parse the input targets, if a rule is found, it will be added to RULES
        dowithlastpipe 'ninja -C out/coverage -t query "${!TARGETS[@]}" | grep "^  " | grep -v "|" | trim_whitespace | parse_input_targets'

        # Mark the queried targets
        for target in "${TARGETS[@]}"; do
            QUERIED_TARGETS["$target"]=1
        done

        # Clear the TARGETS array for the next iteration
        unset TARGETS
        declare -A TARGETS

        # Add the new targets from the last query to TARGETS
        for target in "${!NEW_TARGETS[@]}"; do
            TARGETS["$target"]="${NEW_TARGETS["$target"]}"
        done

    done

    # After all targets are processed, return the rules that were found
    for rule in "${!RULES[@]}"; do
        echo "$rule"
    done
}

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

# Get absolute path from a relative and normalize (e.g "foo/bar/../baz" -> "/path/to/foo/baz")
_abspath() {
    python3 -c "import os.path; print(os.path.abspath('$@'))"
}

CHIP_ROOT=$(_abspath "$(dirname "$0")/..")
OUTPUT_ROOT="$CHIP_ROOT/out/coverage"
COVERAGE_ROOT="$OUTPUT_ROOT/coverage"
SUPPORTED_CODE=(core clusters all)
CODE="core"
QUIET_FLAG=()
ACCUMULATE=false

skip_gn=false
TEST_TARGET=(check)

# By default, do not run YAML or Python tests
ENABLE_YAML=false
ENABLE_PYTHON=false

help() {
    echo "Usage: $file_name [--output_root=<output_root>] [--code=<core|clusters|all>] [Test options"
    echo
    echo "Misc:"
    echo "    -h, --help              Print this help, then exit."
    echo "    -q, --quiet             Decrease verbosity level."
    echo "    -a, --accumulate        Accumulate coverage data from previous runs."
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
    echo "    --target=TARGET         Specify one or more test targets to run for unit tests (e.g. 'TestEmberAttributeBuffer.run' or 'TestBleLayer.run TestBtpEngine.run')."
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
            IFS=' ' read -ra TEST_TARGET <<<"$TEST_TARGET"
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
        -q | --quiet)
            QUIET_FLAG=("--quiet")
            shift
            ;;
        -a | --accumulate)
            ACCUMULATE=true
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

    # Set coverage data to zero if not accumulating
    if [[ -d "$OUTPUT_ROOT/obj/src" && "$ACCUMULATE" == false ]]; then
        lcov --zerocounters --directory "$OUTPUT_ROOT/obj/src" \
            "${QUIET_FLAG[@]}"
    fi

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

    # Clean the targets to reexecute them

    IFS=' ' read -ra RULES_TO_CLEAR <<<"$(get_rules_to_clean "${TEST_TARGET[@]}")"
    if [[ ${#RULES_TO_CLEAR[@]} -gt 0 ]]; then
        ninja -C "$OUTPUT_ROOT" -f toolchain.ninja -t clean -r "${RULES_TO_CLEAR[@]}"
    fi

    #
    # 1) Always run unit tests
    #
    ninja -C "$OUTPUT_ROOT" "${TEST_TARGET[@]}"

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
fi

# ------------------------------------------------------------------------------
# Coverage Generation
#
# Exclude files we do NOT want included in coverage
# Exclude unit test files from coverage
# ----------------------------------------------------------------------------
LCOV_IGNORE_ERRORS="format,unsupported,inconsistent,inconsistent,unused,unused" # incosistent and unused twice is needed to suppress the warnings.
LCOV_EXCLUDE_INCLUDE_OPTIONS=()

if [[ "$skip_gn" == false ]]; then
    LCOV_EXCLUDE_INCLUDE_OPTIONS=(
        --exclude "$CHIP_ROOT/zzz_generated/**"
        --exclude "$CHIP_ROOT/third_party/**"
        --exclude "/usr/include/**"
        --exclude "$CHIP_ROOT/src/app/app-platform/**"
        --exclude "$CHIP_ROOT/src/app/common/**"
        --exclude "$CHIP_ROOT/src/app/util/mock/**"
        --exclude "$CHIP_ROOT/src/controller/python/**"
        --exclude "$CHIP_ROOT/src/lib/dnssd/platform/**"
        --exclude "$CHIP_ROOT/src/lib/shell/**"
        --exclude "$CHIP_ROOT/src/lwip/**"
        --exclude "$CHIP_ROOT/src/platform/**"
        --exclude "$CHIP_ROOT/src/tools/**"
        --exclude "**/tests/**"
    )

    # Restrict coverage to 'core' or 'clusters' if specified
    if [[ "$CODE" == "core" ]]; then
        LCOV_EXCLUDE_INCLUDE_OPTIONS+=(
            --exclude "$CHIP_ROOT/src/app/clusters/**"
        )
    elif [[ "$CODE" == "clusters" ]]; then
        LCOV_EXCLUDE_INCLUDE_OPTIONS+=(
            --include "$CHIP_ROOT/src/app/clusters/**"
        )
    fi
fi

mkdir -p "$COVERAGE_ROOT"

lcov --capture --initial --directory "$OUTPUT_ROOT/obj/src" \
    --ignore-errors "$LCOV_IGNORE_ERRORS" \
    --output-file "$COVERAGE_ROOT/lcov_base.info" \
    "${LCOV_EXCLUDE_INCLUDE_OPTIONS[@]}" \
    "${QUIET_FLAG[@]}"

lcov --capture --directory "$OUTPUT_ROOT/obj/src" \
    --ignore-errors "$LCOV_IGNORE_ERRORS" \
    --output-file "$COVERAGE_ROOT/lcov_test.info" \
    "${LCOV_EXCLUDE_INCLUDE_OPTIONS[@]}" \
    "${QUIET_FLAG[@]}"

lcov --ignore-errors "$LCOV_IGNORE_ERRORS" \
    --add-tracefile "$COVERAGE_ROOT/lcov_base.info" \
    --add-tracefile "$COVERAGE_ROOT/lcov_test.info" \
    --output-file "$COVERAGE_ROOT/lcov_final.info" \
    "${QUIET_FLAG[@]}"


genhtml "$COVERAGE_ROOT/lcov_final.info" \
    --ignore-errors inconsistent,category,count \
    --rc max_message_count=1000 \
    --output-directory "$COVERAGE_ROOT/html" \
    --title "SHA:$(git rev-parse HEAD)" \
    --header-title "Matter SDK Coverage Report" \
    "${QUIET_FLAG[@]}"

cp "$CHIP_ROOT/integrations/appengine/webapp_config.yaml" \
    "$COVERAGE_ROOT/webapp_config.yaml"

HTML_INDEX=$(_abspath "$COVERAGE_ROOT/html/index.html")
if [ -f "$HTML_INDEX" ]; then
    echo
    echo "============================================================"
    echo "Coverage report successfully generated:"
    echo "    file://$HTML_INDEX"
    echo "============================================================"
else
    echo "WARNING: Coverage HTML index was not found at expected path:"
    echo "    $HTML_INDEX"
fi
