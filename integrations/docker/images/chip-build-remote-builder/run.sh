#!/bin/bash -xe

#
#    Copyright (c) 2022 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# run.sh   - utility for running a Docker image
#
# This script expects to live in a directory named after the image
#  with a version file next to it.  So: use symlinks
#
USERNAME=${USERNAME:-ubuntu}
PROJECT=$(gcloud info --format='value(config.project)')
INSTANCE_NAME=${INSTANCE_NAME:-matter-remote-builder}
ZONE=${ZONE:-us-central1-c}
GCLOUD=${GCLOUD:-gcloud}
RETRIES=${RETRIES:-3}

# Run command on the instance via ssh
function ssh() {
    "$GCLOUD" compute ssh --project="$PROJECT" --zone="$ZONE" "$USERNAME@$INSTANCE_NAME" -- "$1"
}

# Always delete workspace after attempting build
function cleanup() {
    ssh "$CLEANUP_CMD"
}

"$GCLOUD" config set project "$PROJECT_ID"
"$GCLOUD" config set compute/zone "$ZONE"

trap cleanup EXIT

RETRY_COUNT=1
while [ "$(ssh 'printf pass')" != "pass" ]; do
    echo "[Try $RETRY_COUNT of $RETRIES] Waiting for instance to start accepting SSH connections..."
    if [ "$RETRY_COUNT" == "$RETRIES" ]; then
        echo "Retry limit reached, giving up!"
        exit 1
    fi
    sleep 10
    RETRY_COUNT=$(($RETRY_COUNT + 1))
done

# Cleanup workspace if there is leftover
ssh "$CLEANUP_CMD"

# Setup workspace with connectedhomeip
ssh "$GIT_CLONE_CMD"

# Run coverage tests
ssh "$RUN_COVERAGE_CMD"

"$GCLOUD" compute scp --project="$PROJECT" --zone="$ZONE" \
    "$USERNAME@$INSTANCE_NAME:/home/ubuntu/connectedhomeip/out/coverage/coverage/coverage_html.tar.gz" "$PWD"

# Always delete workspace after build
ssh "$CLEANUP_CMD"
