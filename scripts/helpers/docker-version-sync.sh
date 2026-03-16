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

VERSION=$(sed 's/ .*//' ./integrations/docker/images/base/chip-build/version)
echo "Current version: $VERSION"

echo "Syncing GitHub Action Workflows"
sed -r -i "s|image: ghcr\.io/project-chip/(.*):[0-9]+|image: ghcr.io/project-chip/\1:$VERSION|" .github/workflows/*.{yaml,yml}

echo "Syncing README.md Files"
find . -iname README.md -not -path './third_party/*' -exec \
    sed -r -i "s|ghcr\.io/project-chip/(.*):[0-9]+|ghcr.io/project-chip/\1:$VERSION|" {} +
