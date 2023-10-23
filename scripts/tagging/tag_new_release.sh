#
# Copyright (c) 2020 Project CHIP Authors
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

#!/bin/bash

CURRENT_SPEC_VERSION=$(cat SPECIFICATION_VERSION)

CURRENT_RELEASE=$(gh release list --exclude-pre-releases | grep -Fi "$CURRENT_SPEC_VERSION" | awk '{print $1}' | head -n1)

if [ -z "$CURRENT_RELEASE" ]; then
    SDK_RELEASE_REVISIONS=0
    echo "No revision found for current release"
else
    SDK_RELEASE_REVISIONS="$(echo "$CURRENT_RELEASE" | cut -d'.' -f4)"
fi

if [ ! -z "$CURRENT_RELEASE" ]; then
    CURRENT_RELEASE="v$CURRENT_SPEC_VERSION.$SDK_RELEASE_REVISIONS"
    SDK_RELEASE_REVISIONS=$(($SDK_RELEASE_REVISIONS + 1))
fi

NEW_RELEASE_TAG="v$CURRENT_SPEC_VERSION.$SDK_RELEASE_REVISIONS"

echo "Current release: $CURRENT_RELEASE"
echo "SDK release revisions: $SDK_RELEASE_REVISIONS"
echo "New release: $NEW_RELEASE_TAG"

gh release create --notes-start-tag "$CURRENT_RELEASE" "$NEW_RELEASE_TAG" "$@"
