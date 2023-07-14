#!/usr/bin/env bash
# TODO: Add a simple way to roll docker image numbers
#image: xxxxxxxxxx:0.7.14
CURRENT_VERSION=$(sed 's/ .*//' ./integrations/docker/images/base/chip-build/version)

echo "Current version: $CURRENT_VERSION"

NEXT_VERSION=$((CURRENT_VERSION + 1))

echo "Next version: $NEXT_VERSION"

sed -r -i "s|image: ghcr\.io/project-chip/(.*):[0-9]*(\..*)?|image: ghcr.io/project-chip/\1:$NEXT_VERSION|" .github/workflows/*.yaml

echo "$NEXT_VERSION" >./integrations/docker/images/base/chip-build/version
