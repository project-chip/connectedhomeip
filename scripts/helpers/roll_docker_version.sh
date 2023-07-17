#!/usr/bin/env bash
CURRENT_VERSION=$(sed 's/ .*//' ./integrations/docker/images/base/chip-build/version)

echo "Current version: $CURRENT_VERSION"

NEXT_VERSION=$((CURRENT_VERSION + 1))

echo "Next version: $NEXT_VERSION"

echo "Rolling workflows"
sed -r -i "s|image: ghcr\.io/project-chip/(.*):[0-9]*(\..*)?|image: ghcr.io/project-chip/\1:$NEXT_VERSION|" .github/workflows/*.yaml

echo "Rolling VSCode container"
sed -r -i "s|matter-dev-environment:local --version [0-9]*|matter-dev-environment:local --version $NEXT_VERSION|" .devcontainer/devcontainer.json

echo "Rolling docker images"
sed -r -i "s|ARG VERSION=[0-9]*|ARG VERSION=$NEXT_VERSION|" integrations/docker/images/*/*/Dockerfile

echo "Rolling main docker version"
echo "$NEXT_VERSION" >./integrations/docker/images/base/chip-build/version

echo "Done"
