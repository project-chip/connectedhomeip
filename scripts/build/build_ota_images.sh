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

CHIP_ROOT="$(dirname "$0")/../.."

# Default values for optional arguments
VENDOR_ID="0xDEAD"
PRODUCT_ID="0xBEEF"
MAX_RANGE=5
OUT_PREFIX="out/su_ota_images_min"
BUILT_IMAGES_STACK=()
TARGET_FILE="$CHIP_ROOT/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h"
BACKUP_CONFIG_FILE="cp $TARGET_FILE $CHIP_ROOT/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup"
STATUS_CODE=0

replace_version_config_str() {
    local version="$1"
    local dest_file="$2"

    cat >>"$dest_file" <<EOL

#ifdef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#undef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION $i
#endif
#ifdef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#undef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "$i.0"
#endif
EOL

}

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

# Create the backup file
echo "Starting building the ota images with different version number."
echo "Creating the backup file"

cp "$TARGET_FILE" "$CHIP_ROOT"/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup
STATUS_CODE=$?
if [ "$STATUS_CODE" -ne 0 ]; then
    echo "Command failed with status code: $STATUS_CODE"
    exit 1
fi

for ((i = 2; i <= "$MAX_RANGE"; i++)); do
    echo "Running for version $i"

    replace_version_config_str "$i" "$TARGET_FILE"

    # Build the image
    echo "Building the requestor app"
    ./"$CHIP_ROOT"/scripts/examples/gn_build_example.sh "$CHIP_ROOT"/examples/ota-requestor-app/linux "$BASE_OUT_PREFIX" chip_config_network_layer_ble=false is_debug=false >/dev/null
    STATUS_CODE=$?
    if [ "$STATUS_CODE" -ne 0 ]; then
        echo "Failed to build the app $TARGET_FILE"
        break
    fi

    # Strip command
    if [ "$(uname -s)" = "Darwin" ]; then
        strip "$BASE_OUT_PREFIX"/chip-ota-requestor-app -o "$BASE_OUT_PREFIX"/chip-ota-requestor-app.min >/dev/null 2>&1
    else
        strip --strip-all "$BASE_OUT_PREFIX"/chip-ota-requestor-app -o "$BASE_OUT_PREFIX"/chip-ota-requestor-app.min >/dev/null 2>&1
    fi
    STATUS_CODE=$?
    if [ "$STATUS_CODE" -ne 0 ]; then
        echo "Failed to strip the code from min app"
        break
    fi

    # Create ota image
    OTA_IMAGE_PATH=$CHIP_ROOT/$OUT_PREFIX/chip-ota-requestor-app_v$i.min.ota
    python3 "$CHIP_ROOT"/src/app/ota_image_tool.py create -v "$VENDOR_ID" -p "$PRODUCT_ID" -vn "$i" -vs "$i.0" -da sha256 "$BASE_OUT_PREFIX"/chip-ota-requestor-app.min "$OTA_IMAGE_PATH"
    STATUS_CODE=$?
    if [ "$STATUS_CODE" -ne 0 ]; then
        echo "Failed to create the OTA Image $TARGET_FILE"
        break
    fi
    BUILT_IMAGES_STACK+=("$OTA_IMAGE_PATH")
    echo "Restoring the config file"
    cp "$CHIP_ROOT"/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup "$TARGET_FILE"
done

if [ "$STATUS_CODE" -eq 0 ]; then
    echo "Generated files"
    for item in "${BUILT_IMAGES_STACK[@]}"; do
        echo "$item"
    done
fi

echo "Restoring backup file"
cp "$CHIP_ROOT"/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup "$TARGET_FILE"
echo "Removing old backup file"
rm "$CHIP_ROOT"/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup

if [ "$STATUS_CODE" -ne 0 ]; then
    exit 1
fi
