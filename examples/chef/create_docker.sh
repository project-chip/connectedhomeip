#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e # Exit on any script error.

OUT_DIRECTORY="$WORKING_DIR/out/"

# Clean up the out directory before extracting device files
rm -r "$OUT_DIRECTORY"*

# Extract device files
tar -xf "$TAR_PATH" -C "$OUT_DIRECTORY"

# Build the docker image
docker build -f "$WORKING_DIR"/dockerfile -t "$IMAGE_NAME":latest -t "$IMAGE_NAME:short-sha_$SHORT_SHA" \
    -t "$IMAGE_NAME:build-id_$BUILD_ID" -t "$IMAGE_NAME:commit-sha_$COMMIT_SHA" -t "$IMAGE_NAME:revision-id_$REVISION_ID" \
    --build-arg DEVICE_NAME="$DEVICE_NAME" .
