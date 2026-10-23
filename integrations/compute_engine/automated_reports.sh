#!/usr/bin/env bash

#
# Copyright (c) 2026 Project CHIP Authors
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

# Default values
RUN_COVERAGE=false
RUN_CONFORMANCE=false
RUN_ALCHEMY=false
DEPLOY=false
SERVE=false
PAT_SECRET=""
OUT_DIR="out/coverage/coverage/html"
TEMPLATE="integrations/compute_engine/report_not_generated.html.template"
INDEX_TEMPLATE="integrations/compute_engine/appengine_index.html.template"

help() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  --coverage        Generate coverage report"
    echo "  --conformance     Generate conformance report"
    echo "  --alchemy         Generate Alchemy diff report"
    echo "  --all             Generate all reports"
    echo "  --deploy          Deploy to App Engine"
    echo "  --serve, --test   Start local server to view reports"
    echo "  --pat-secret NAME Secret name for GitHub PAT (used by Alchemy)"
    echo "  -h, --help        Print this help"
    echo
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --coverage)
            RUN_COVERAGE=true
            shift
            ;;
        --conformance)
            RUN_CONFORMANCE=true
            shift
            ;;
        --alchemy)
            RUN_ALCHEMY=true
            shift
            ;;
        --all)
            RUN_COVERAGE=true
            RUN_CONFORMANCE=true
            RUN_ALCHEMY=true
            shift
            ;;
        --deploy)
            DEPLOY=true
            shift
            ;;
        --serve|--test)
            SERVE=true
            shift
            ;;
        --pat-secret)
            PAT_SECRET="$2"
            shift 2
            ;;
        -h|--help)
            help
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            help
            exit 1
            ;;
    esac
done

# Ensure output directory exists
mkdir -p "$OUT_DIR"

write_dummy() {
    local file=$1
    local name=$2
    echo "Generating dummy page for $name..."
    # Ensure directory exists if file is in a subdirectory
    mkdir -p "$(dirname "$OUT_DIR/$file")"
    sed "s/__REPORT_NAME__/$name/g" "$TEMPLATE" > "$OUT_DIR/$file"
}

# --- Coverage Report ---
if [ "$RUN_COVERAGE" = true ]; then
    echo "Generating Coverage Report..."
    ./scripts/build_coverage.sh

    # Move coverage files to a subdirectory
    echo "Reorganizing coverage files..."
    mkdir -p out/coverage_tmp
    # Move everything from html to tmp
    mv "$OUT_DIR"/* out/coverage_tmp/ 2>/dev/null || true
    # Recreate html dir and coverage subdir
    mkdir -p "$OUT_DIR/coverage"
    # Move files back to coverage subdir
    mv out/coverage_tmp/* "$OUT_DIR/coverage/" 2>/dev/null || true
    rm -rf out/coverage_tmp
else
    write_dummy "coverage/index.html" "Coverage Report"
fi

# --- Conformance Report ---
if [ "$RUN_CONFORMANCE" = true ]; then
    echo "Generating Conformance Report..."
    source scripts/activate.sh
    ./scripts/build_python.sh -w false -i out/python_env
    python3 -u scripts/examples/conformance_report.py
    cp /tmp/conformance_report/conformance_report.html "$OUT_DIR/"
else
    write_dummy "conformance_report.html" "Conformance Report"
fi

# --- Alchemy Diff Report ---
if [ "$RUN_ALCHEMY" = true ]; then
    echo "Generating Alchemy Diff Report..."
    ARGS=""
    if [ -n "$PAT_SECRET" ]; then
        ARGS="--use-pat-secret $PAT_SECRET"
    fi
    ./integrations/compute_engine/run_alchemy_diff.sh $ARGS

    # Copy results if they exist
    if [ -f "out/generated_xml/mismatches.html" ]; then
        cp out/generated_xml/mismatches.html "$OUT_DIR/sdk_spec_zapdiff.html"
        cp out/generated_xml/mismatches.csv "$OUT_DIR/sdk_spec_zapdiff.csv" 2>/dev/null || true
        echo "Alchemy diff reports copied to $OUT_DIR as sdk_spec_zapdiff"
    else
        echo "Warning: Alchemy diff files not found."
        write_dummy "sdk_spec_zapdiff.html" "Alchemy Diff Report"
    fi
else
    write_dummy "sdk_spec_zapdiff.html" "Alchemy Diff Report"
fi

# --- Generate Index Page ---
echo "Generating Index Page..."
COV_CLASS="enabled"
CONF_CLASS="enabled"
ALCH_CLASS="enabled"
COV_STATUS="Active"
CONF_STATUS="Active"
ALCH_STATUS="Active"

if [ "$RUN_COVERAGE" = false ]; then COV_CLASS="disabled"; COV_STATUS="Skipped"; fi
if [ "$RUN_CONFORMANCE" = false ]; then CONF_CLASS="disabled"; CONF_STATUS="Skipped"; fi
if [ "$RUN_ALCHEMY" = false ]; then ALCH_CLASS="disabled"; ALCH_STATUS="Skipped"; fi

sed -e "s/__COVERAGE_CLASS__/$COV_CLASS/g" \
    -e "s/__CONFORMANCE_CLASS__/$CONF_CLASS/g" \
    -e "s/__ALCHEMY_CLASS__/$ALCH_CLASS/g" \
    -e "s/__COVERAGE_STATUS__/$COV_STATUS/g" \
    -e "s/__CONFORMANCE_STATUS__/$CONF_STATUS/g" \
    -e "s/__ALCHEMY_STATUS__/$ALCH_STATUS/g" \
    "$INDEX_TEMPLATE" > "$OUT_DIR/index.html"

# --- Deploy ---
if [ "$DEPLOY" = true ]; then
    echo "Deploying to App Engine..."
    (
        cd out/coverage/coverage
        gcloud app deploy webapp_config.yaml --quiet

        # Cleanup old versions, keep last 5
        versions=$(gcloud app versions list \
            --service default \
            --sort-by '~VERSION.ID' \
            --format 'value(VERSION.ID)' | sed 1,5d)

        for version in $versions; do
            gcloud app versions delete "$version" \
                --service default \
                --quiet
        done
    )
fi

# --- Serve ---
if [ "$SERVE" = true ]; then
    echo "Starting local server at http://localhost:8000"
    (
        cd "$OUT_DIR"
        python3 -m http.server 8000
    )
fi
