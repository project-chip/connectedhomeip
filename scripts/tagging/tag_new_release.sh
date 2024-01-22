#
# Copyright (c) 2023 Project CHIP Authors
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

# Pulls the most recent release from gihub, matching the spec version on the current tree
CURRENT_RELEASE=$(gh release list --exclude-pre-releases | grep -Fi "$CURRENT_SPEC_VERSION" | awk '{print $1}' | head -n1)

if [ -z "$CURRENT_RELEASE" ]; then
    # If there are no releases, this is our first one for this Spec version
    SDK_RELEASE_REVISIONS=0
    echo "No revision found for current release"
else
    # Otherwise pull the SDK revision (4th item) from the release
    SDK_RELEASE_REVISIONS="$(echo "$CURRENT_RELEASE" | cut -d'.' -f4)"
fi

if [ ! -z "$CURRENT_RELEASE" ]; then
    # If there is current release, construct a string like 1.2.0.0 based o the current one
    CURRENT_RELEASE="v$CURRENT_SPEC_VERSION.$SDK_RELEASE_REVISIONS"
    # Then revise the SDK release to be +1
    SDK_RELEASE_REVISIONS=$(($SDK_RELEASE_REVISIONS + 1))
fi

# Construct a final tag, eg: 1.2.0.5 (MAJOR.MINOR.PATCH.SDK_REVISION)
NEW_RELEASE_TAG="v$CURRENT_SPEC_VERSION.$SDK_RELEASE_REVISIONS"

ADDITIONAL_ARGS=""

# Look for any prerelease information in the spec version (eg: 1.3.0-sve), and target the prerelease channel
case "$NEW_RELEASE_TAG" in
    *alpha* | *beta* | *prerelease* | *testevent* | *te* | *sve*)
        ADDITIONAL_ARGS="$ADDITIONAL_ARGS --prerelease"
        ;;
esac

echo "Current release: $CURRENT_RELEASE"
echo "SDK release revisions: $SDK_RELEASE_REVISIONS"
echo "New release: $NEW_RELEASE_TAG"
echo "Additional arguments: $ADDITIONAL_ARGS"

echo executing: gh release create "$NEW_RELEASE_TAG" --notes-start-tag "$CURRENT_RELEASE" "$@" "$ADDITIONAL_ARGS"
gh release create $NEW_RELEASE_TAG "--notes-start-tag" $CURRENT_RELEASE $@ $ADDITIONAL_ARGS
