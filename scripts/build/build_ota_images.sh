#!/usr/bin/env bash

#
# Copyright (c) 2025 Project CHIP Authors
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

set -euo pipefail
set -x

CHIP_ROOT="$(dirname "$0")/../.."

# Default values for optional arguments
VENDOR_ID="0xDEAD"
PRODUCT_ID="0xBEEF"
MAX_RANGE=6
OUT_PREFIX="out/su_ota_images_min"
BUILT_IMAGES_STACK=()

# Parse command line arguments
while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --vendor-id)
            VENDOR_ID="$2"
            shift 2
            ;;
        --product-id)
            PRODUCT_ID="$2"
            shift 2
            ;;
        --max-range)
            MAX_RANGE="$2"
            shift 2
            ;;
        --out-prefix)
            OUT_PREFIX="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "Values to use for build"
echo "Vendor ID: $VENDOR_ID"
echo "Product ID: $PRODUCT_ID"
echo "Max Range: $MAX_RANGE"
echo "Output Prefix: $OUT_PREFIX"

BASE_OUT_PREFIX="$CHIP_ROOT/$OUT_PREFIX"
echo "PREFIX $BASE_OUT_PREFIX"

echo "Starting building the ota images with different version number."

for ((i = 2; i <= "$MAX_RANGE"; i++)); do
    echo "Building for version $i"

    # Use a per-version GN output dir to avoid clobbering / to enable reuse
    OUT_DIR="$BASE_OUT_PREFIX-v$i-${GITHUB_RUN_ID:-local}"
    mkdir -p "$OUT_DIR"

    # Pass version via GN args (target_cflags). Quotes must be escaped so the GN arg
    # arrives intact. gn_build_example.sh forwards key=value args to gn gen.
    GN_VERSION_ARGS="target_cflags=[\"-DCHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION=$i\",\"-DCHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING=\\\"$i.0\\\"\"]"

    # Build the image without editing source files
    echo "Building the requestor app (out: $OUT_DIR)"
    ./"$CHIP_ROOT"/scripts/examples/gn_build_example.sh \
        "$CHIP_ROOT"/examples/ota-requestor-app/linux "$OUT_DIR" \
        chip_config_network_layer_ble=false is_debug=false "$GN_VERSION_ARGS"

    if [ ! -f "$OUT_DIR/chip-ota-requestor-app" ]; then
        echo "ERROR: Could not find chip-ota-requetor-app in $OUT_DIR"
        ls -l "$OUT_DIR"
        exit 1
    fi

    # Strip command (create .min binary in the same OUT_DIR)
    if [ "$(uname -s)" = "Darwin" ]; then
        strip "$OUT_DIR"/chip-ota-requestor-app -o "$OUT_DIR"/chip-ota-requestor-app.min >/dev/null 2>&1
    else
        strip --strip-all "$OUT_DIR"/chip-ota-requestor-app -o "$OUT_DIR"/chip-ota-requestor-app.min >/dev/null 2>&1
    fi

    if [ ! .s "$OUT_DIR/chip-ota-requestor-app.min" ]; then
        echo "ERROR: Could not find .min file in $OUT_DIR"
        ls -l "$OUT_DIR"
        exit 1
    fi

    # Create ota image (store images under the shared OUT_PREFIX directory)
    OTA_IMAGE_PATH="$OUT_DIR/chip-ota-requestor-app_v$i.min.ota"
    python3 "$CHIP_ROOT"/src/app/ota_image_tool.py create -v "$VENDOR_ID" -p "$PRODUCT_ID" -vn "$i" -vs "$i.0" -da sha256 "$OUT_DIR"/chip-ota-requestor-app.min "$OTA_IMAGE_PATH"

    BUILT_IMAGES_STACK+=("$OTA_IMAGE_PATH")
done

echo "Generated files"
for item in "${BUILT_IMAGES_STACK[@]}"; do
    echo "$item"
done
