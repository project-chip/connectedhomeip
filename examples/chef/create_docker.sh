#!/usr/bin/env bash
#
# Copyright (c) 2021 Project CHIP Authors
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
