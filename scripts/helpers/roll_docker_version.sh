#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

#
# Usage:
#  ./scripts/helpers/roll_docker_version.sh -v=X --reason="INSERT_REASON_HERE"
#

CURRENT_VERSION=$(sed 's/ .*//' ./integrations/docker/images/base/chip-build/version)
NEXT_VERSION=$((CURRENT_VERSION + 1))
ROLL_REASON=""
ALL_ARGS=""

for i in "$@"; do
    case $i in
        -v=* | --version=*)
            NEXT_VERSION="${i#*=}"
            ;;
        -r=* | --reason=*)
            ROLL_REASON="${i#*=}"
            ;;
        *)
            ALL_ARGS="$ALL_ARGS ${i#*=}"
            ;;
    esac
done

if [[ ! $ROLL_REASON ]]; then
    echo 'Roll reason required, please add --reason="Why I am rolling"'
    exit -1
fi

echo "Current version: $CURRENT_VERSION"
echo "Next version: $NEXT_VERSION"
echo "Reason: $ROLL_REASON"
echo ""

echo "Rolling workflows"
sed -r -i "s|image: ghcr\.io/project-chip/(.*):[0-9,a-z,A-Z,-]*(\..*)?|image: ghcr.io/project-chip/\1:$NEXT_VERSION|" .github/workflows/*.yaml
sed -r -i "s|image: ghcr\.io/project-chip/(.*):[0-9,a-z,A-Z,-]*(\..*)?|image: ghcr.io/project-chip/\1:$NEXT_VERSION|" .github/workflows/*.yml

echo "Rolling VSCode container"
sed -r -i "s|matter-dev-environment:local --version [0-9,a-z,A-Z,-]*|matter-dev-environment:local --version $NEXT_VERSION|" .devcontainer/devcontainer.json

echo "Rolling docker images"
find . -iname Dockerfile -not -path "./third_party/*" | xargs sed -r -i "s|ARG VERSION=[0-9,a-z,A-Z,-]*|ARG VERSION=$NEXT_VERSION|"

echo "Rolling main docker version"
echo "$NEXT_VERSION : $ROLL_REASON" >./integrations/docker/images/base/chip-build/version

echo "Rolling README.md files"
find . -iname readme.md -not -path "./third_party/*" | xargs sed -r -i "s|ghcr\.io/project-chip/(.*):[0-9,a-z,A-Z,-]*(\..*)?|ghcr.io/project-chip/\1:$NEXT_VERSION|"

echo ""
echo "Done"
