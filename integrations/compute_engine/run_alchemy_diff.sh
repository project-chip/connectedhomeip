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
USE_SECRET=""
BASELINE_XML="src/app/zap-templates/zcl/data-model/chip"
GENERATED_XML="out/generated_xml"
SDK_ROOT="."
SPEC_ROOT="out/spec"
SKIP_UPDATE=false
TEMP_BASELINE="out/baseline_xml"
TEMP_GENERATED="out/generated_xml_files"
BACKUP_FILES="out/backup_files"

# Default versions/branches
ALCHEMY_VERSION="1.6.14"
SPEC_BRANCH="master"

# Default attributes for generation (from spec repo workflow)
GEN_ATTRIBUTES="ambient-context-sensor,ambientsensing,cameras,cameras-image-rotation,groupcast,hrap-pdc,hrap-tbrd,hvac-thermostat-events,hvac-thermostat-suggestions,improved-capabilities-tcr,partitioned-crl,security-sensor-events-tcr,tcr-smokeco-unmounted-state,temperature-alarm,thread-commissioning"

# Specific files modified by Alchemy that need backup and restore
# Running alchemy zap regen will modify these, so restore them back
# after the alchemy run.
FILES_TO_RESTORE=(
    "src/app/zap-templates/zcl/zcl-with-test-extensions.json"
    "src/app/zap-templates/zcl/zcl.json"
    "src/app/zap_cluster_list.json"
    ".github/workflows/tests.yaml"
)

help() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  --use-pat-secret NAME  Name of the gcloud secret containing the GitHub PAT"
    echo "  --baseline-xml PATH    Path to directory containing baseline ZAP XMLs (default: $BASELINE_XML)"
    echo "  --generated-xml PATH   Path to directory where reports will be saved (default: $GENERATED_XML)"
    echo "  --spec-root PATH       Path to directory containing the spec repository (default: $SPEC_ROOT)"
    echo "  --skip-update          Skip updating existing checkouts (default: false)"
    echo "  -h, --help             Print this help"
    echo
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --use-pat-secret)
            USE_SECRET="$2"
            shift 2
            ;;
        --baseline-xml)
            BASELINE_XML="$2"
            shift 2
            ;;
        --generated-xml)
            GENERATED_XML="$2"
            shift 2
            ;;
        --spec-root)
            SPEC_ROOT="$2"
            shift 2
            ;;
        --skip-update)
            SKIP_UPDATE=true
            shift
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

# Handle PAT
GITHUB_PAT=""
if [ -n "$USE_SECRET" ]; then
    echo "Fetching PAT from secret: $USE_SECRET"
    GITHUB_PAT=$(gcloud secrets versions access latest --secret="$USE_SECRET")
fi

# Ensure output directory exists
mkdir -p out

update_or_clone() {
    local dir=$1
    local url=$2
    local name=$3
    local branch=$4

    if [ "$SKIP_UPDATE" = true ] && [ -d "$dir" ]; then
        echo "Using existing $name directory as requested."
        return
    fi

    # Setup GIT_ASKPASS if we have a token
    local cleanup_askpass=false
    if [ -n "$GITHUB_PAT" ]; then
        local askpass_path="out/askpass.sh"
        echo '#!/bin/sh' > "$askpass_path"
        echo 'echo "$GITHUB_PAT"' >> "$askpass_path"
        chmod +x "$askpass_path"
        export GIT_ASKPASS="$(pwd)/$askpass_path"
        cleanup_askpass=true
        
        # Modify URL to include x-access-token username if it's an HTTPS URL
        if [[ "$url" == https://* ]]; then
            url="https://x-access-token@${url#https://}"
        fi
    fi

    if [ -d "$dir" ]; then
        echo "Updating $name to $branch..."
        (
            cd "$dir"
            git fetch origin "$branch"
            git checkout FETCH_HEAD
        )
    else
        echo "Cloning $name ($branch)..."
        git clone --branch "$branch" "$url" "$dir"
    fi

    # Cleanup
    if [ "$cleanup_askpass" = true ]; then
        rm -f "out/askpass.sh"
        unset GIT_ASKPASS
    fi
}

# Clone/Update Spec (Private)
if [ -n "$GITHUB_PAT" ]; then
    update_or_clone "$SPEC_ROOT" "https://${GITHUB_PAT}@github.com/CHIP-Specifications/connectedhomeip-spec.git" "Spec" "$SPEC_BRANCH"
else
    update_or_clone "$SPEC_ROOT" "git@github.com:CHIP-Specifications/connectedhomeip-spec.git" "Spec" "$SPEC_BRANCH"
fi

# Download Alchemy Release
ALCHEMY_ASSET="alchemy-${ALCHEMY_VERSION}-linux-amd64.tar.gz"
ALCHEMY_URL="https://github.com/project-chip/alchemy/releases/download/v${ALCHEMY_VERSION}/${ALCHEMY_ASSET}"

if [ "$SKIP_UPDATE" = true ] && [ -f "out/alchemy" ]; then
    echo "Using existing Alchemy binary."
else
    echo "Downloading Alchemy v${ALCHEMY_VERSION}..."
    curl -L "$ALCHEMY_URL" -o "out/${ALCHEMY_ASSET}"

    echo "Extracting Alchemy..."
    tar -xzf "out/${ALCHEMY_ASSET}" -C out/
    chmod +x out/alchemy
fi

# --- Step 1: Save Baseline and Files ---
echo "Saving baseline XMLs and files..."
rm -rf "$TEMP_BASELINE"
mkdir -p "$TEMP_BASELINE"
cp -r "$BASELINE_XML"/* "$TEMP_BASELINE/"

# Backup specific files
mkdir -p "$BACKUP_FILES"
for f in "${FILES_TO_RESTORE[@]}"; do
    if [ -f "$f" ]; then
        mkdir -p "$BACKUP_FILES/$(dirname "$f")"
        cp "$f" "$BACKUP_FILES/$f"
    fi
done

# --- Step 2: Generate New XMLs from Spec ---
echo "Generating ZAP XMLs from spec..."
# Using --force to proceed despite spec parsing errors
./out/alchemy zap --spec-root "$SPEC_ROOT" --sdk-root "$SDK_ROOT" --force -a "$GEN_ATTRIBUTES"

# --- Step 3: Save Generated and Restore Baseline ---
echo "Checking for differences before restore..."
# This will show if alchemy actually modified anything in the SDK
diff -rq "$TEMP_BASELINE" "$BASELINE_XML" || true

echo "Saving generated XMLs and restoring baseline..."
rm -rf "$TEMP_GENERATED"
mkdir -p "$TEMP_GENERATED"
cp -r "$BASELINE_XML"/* "$TEMP_GENERATED/"

# Restore SDK XMLs to clean state
echo "Restoring SDK XMLs to clean state..."
rm -rf "$BASELINE_XML"/*
cp -r "$TEMP_BASELINE"/* "$BASELINE_XML/"

# Restore specific files
echo "Restoring specific files..."
for f in "${FILES_TO_RESTORE[@]}"; do
    if [ -f "$BACKUP_FILES/$f" ]; then
        cp "$BACKUP_FILES/$f" "$f"
    fi
done

# --- Step 4: Run Diff ---
echo "Running Alchemy Diff..."
mkdir -p "$GENERATED_XML"
./out/alchemy zap-diff \
    --xml-root-1 "$TEMP_BASELINE" \
    --xml-root-2 "$TEMP_GENERATED" \
    --out "$GENERATED_XML" \
    --format "both"

echo "Done. Check $GENERATED_XML for results."
