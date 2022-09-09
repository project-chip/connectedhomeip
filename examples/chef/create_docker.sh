#!/usr/bin/env bash

set -e # Exit on any script error.

IMAGE_NAME='us.gcr.io/nest-eng/chef/dimmable-light'

# Extract dimmable light files
mkdir /workspace/examples/chef/tmp/
tar -xf /workspace/artifacts/linux_x86-rootnode_dimmablelight_bCwGYSDpoe.tar.gz -C /workspace/examples/chef/tmp

# Build the docker image
docker build -f /workspace/examples/chef/dockerfile -t $IMAGE_NAME:latest -t $IMAGE_NAME:short-sha_$SHORT_SHA \
    -t $IMAGE_NAME:build-id_$BUILD_ID -t $IMAGE_NAME:commit-sha_$COMMIT_SHA -t $IMAGE_NAME:revision-id_$REVISION_ID .

# Cleanup the extracted files
rm -r /workspace/examples/chef/tmp
