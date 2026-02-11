#!/usr/bin/env bash
#
# Local build that replicates build.sh behavior. Must be run from repo root
# (connectedhomeip_silabs) because the Dockerfile COPYs scripts/setup/silabs/*
#
# Usage (from connectedhomeip_silabs repo root):
#   ./integrations/docker/images/stage-2/chip-build-efr32/build-local.sh [--no-cache]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../../../.." && pwd)"
GHCR_ORG="ghcr.io"
ORG="${DOCKER_BUILD_ORG:-project-chip}"
IMAGE="chip-build-efr32"
VERSION="${DOCKER_BUILD_VERSION:-1}"

if [[ "$OSTYPE" == darwin* ]]; then
    TARGET_PLATFORM_TYPE="linux/arm64"
    # Use amd64 so base image ghcr.io/project-chip/chip-build:1 (often amd64) matches; avoids platform warning
    PLATFORM_FLAG="--platform=linux/amd64"
else
    TARGET_PLATFORM_TYPE="linux/amd64"
    PLATFORM_FLAG=""
fi

BUILD_ARGS=(
    $PLATFORM_FLAG
    --build-arg "TARGETPLATFORM=$TARGET_PLATFORM_TYPE"
    --build-arg "VERSION=$VERSION"
    -f "$SCRIPT_DIR/Dockerfile"
    -t "$GHCR_ORG/$ORG/$IMAGE:$VERSION"
)

[[ " $* " == *" --no-cache "* ]] && BUILD_ARGS=(--no-cache "${BUILD_ARGS[@]}")

cd "$REPO_ROOT"
docker build "${BUILD_ARGS[@]}" .
docker image prune --force

echo "Built $GHCR_ORG/$ORG/$IMAGE:$VERSION"
echo "Run: docker run --rm -it $GHCR_ORG/$ORG/$IMAGE:$VERSION"
