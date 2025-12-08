#!/usr/bin/env bash

#
# Copyright (c) 2020-2024 Project CHIP Authors
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

# Use git and restyle-path to help reformat anything that differs from
#  some base edit point.  Use before pushing a PR to make sure everything
#  you've written is kosher to CI
#
# Usage:
#  restyle-diff.sh [-d] [ref]
#
# if unspecified, ref defaults to upstream/master (or master)
# -d enables debug logging for Restyle CLI
#
# Note: This script requires sudo to restore file ownership after restyle
#  (which uses Docker and changes ownership of restyled files to root); You can either run the
#  script directly and be prompted for sudo, or run it with sudo.

here=${0%/*}

set -e

MAX_ARGS=256

CHIP_ROOT=$(cd "$here/../.." && pwd)
cd "$CHIP_ROOT"

restyle-paths() {

    local uid="${SUDO_UID:-$(id -u)}"
    local gid="${SUDO_GID:-$(id -g)}"

    restyle --config-file=.restyled.yaml "$@"

    echo
    echo "[restyle-diff.sh] Restoring file ownership to current user (sudo required)"
    sudo chown "$uid:$gid" "$@"
}

if ! command -v restyle >/dev/null 2>&1; then
    echo
    echo "Restyle CLI is not installed or is not in PATH."
    echo
    echo "Install it by following the installation instructions in the restyled-io/restyler GitHub README and then run again:"
    echo "  https://github.com/restyled-io/restyler#installation"
    echo
    exit 1
fi

#This was added to be able to use xargs to call the function restyle-paths
export -f restyle-paths

while [[ $# -gt 0 ]]; do
    case "$1" in
        -d)
            export DEBUG=True
            shift
            ;;
        *)
            ref="$1"
            shift
            ;;
    esac
done

if [[ -z "$ref" ]]; then
    ref="master"
    git remote | grep -qxF upstream && ref="upstream/master"
fi

if [[ $pull_image -eq 1 ]]; then
    docker pull restyled/restyler:edge
fi

paths=$(git diff --ignore-submodules --name-only --merge-base "$ref")

echo "$paths" | xargs -n "$MAX_ARGS" "$BASH" -c 'restyle-paths "$@"' -
