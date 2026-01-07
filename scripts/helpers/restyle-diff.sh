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
#  (which uses Docker and changes ownership of restyled files to root). Run this script as a regular user;
#  it will prompt for sudo only when needed to restore file ownership.

here=${0%/*}

set -e

MAX_ARGS=256

CHIP_ROOT=$(cd "$here/../.." && pwd)
cd "$CHIP_ROOT"

restyle-paths() {

    local uid="${SUDO_UID:-$(id -u)}"
    local gid="${SUDO_GID:-$(id -g)}"

    echo "[restyle-diff.sh] Please wait, Restyling files (and Pulling restyler Docker images if needed)"
    restyle --config-file=.restyled.yaml "$@"

    echo
    echo "[restyle-diff.sh] Restoring file ownership to current user (sudo required)"
    sudo chown -h "$uid:$gid" -- "$@"
}

ensure_restyle_installed() {
    if command -v restyle >/dev/null 2>&1; then
        return 0
    fi

    echo "[restyle-diff.sh] Restyle CLI not found, downloading it from GitHub restyled-io/restyler releases..."

    # It seems that restyler releases only linux x86_64 and darwin arm64 binaries at this time
    case "$(uname -s)-$(uname -m)" in
        Linux-x86_64) asset="restyler-linux-x86_64" ;;
        Darwin-arm64) asset="restyler-darwin-arm64" ;;
        *)
            echo "[restyle-diff.sh] Unsupported platform: $(uname -s)-$(uname -m)"
            echo "[restyle-diff.sh] Check available binaries at: https://github.com/restyled-io/restyler/releases"
            exit 1
            ;;
    esac

    tmpdir=$(mktemp -d)
    trap 'rm -rf "$tmpdir"' EXIT

    if ! curl -sSfL "https://github.com/restyled-io/restyler/releases/latest/download/$asset.tar.gz" | tar xz -C "$tmpdir"; then
        echo "[restyle-diff.sh] Failed to download restyle for $(uname -s)-$(uname -m)"
        echo "[restyle-diff.sh] Check available binaries at: https://github.com/restyled-io/restyler/releases"
        exit 1
    fi

    echo "[restyle-diff.sh] Installing restyle to $HOME/.local/bin"
    mkdir -p "$HOME/.local/bin"
    install "$tmpdir/$asset/restyle" "$HOME/.local/bin"
    export PATH="$HOME/.local/bin:$PATH"

}

# This was added to be able to use xargs to call the function restyle-paths
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

paths=$(git diff --ignore-submodules --name-only --merge-base "$ref")

ensure_restyle_installed

echo "$paths" | xargs -n "$MAX_ARGS" "$BASH" -c 'restyle-paths "$@"' -
