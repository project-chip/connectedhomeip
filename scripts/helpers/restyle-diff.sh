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
#  restyle-diff.sh [-d] [-p] [ref]
#
# if unspecified, ref defaults to upstream/master (or master)
# -d enables debug logging for Restyle CLI
# -p pull restyler Docker images before running (default: skip pull, reuse cached images)
#

here=${0%/*}

set -e

MAX_ARGS=256

CHIP_ROOT=$(cd "$here/../.." && pwd)
cd "$CHIP_ROOT"

restyle-paths() {
    [[ $# -eq 0 ]] && return 0

    if [[ "$PULL" == "True" ]]; then
        echo "[restyle-diff.sh] Please wait, Restyling files (and Pulling restyler Docker images if needed)"
    else
        echo "[restyle-diff.sh] Please wait, Restyling files (using cached images; pass -p to pull updates)"
    fi
    restyle --config-file=.restyled.yaml "$@"

    # warn if restyle left any files owned by root (which means older restyle-CLI is being used)
    root_owned=$(find "$@" -maxdepth 0 -user 0 2>/dev/null || true)
    if [[ -n "$root_owned" ]]; then
        echo
        echo "[restyle-diff.sh] WARNING: The following restyled files are owned by root:"
        echo "$root_owned"
        echo
        echo "[restyle-diff.sh] This typically means your restyle CLI is older than v0.80 which fixed this bug."
        echo "[restyle-diff.sh] Please UPGRADE to a newer restyle CLI by running"
        echo "[restyle-diff.sh] 1. rm -f \"\$(command -v restyle)\""
        echo "[restyle-diff.sh] 2. re-run this script without using sudo (it will automatically download latest restyle-CLI version)."
    fi

}

version_lt() {
    local v1="$1" v2="$2"
    local num1 num2 i=0
    while :; do
        num1="${v1%%.*}" v1="${v1#"$num1"}" v1="${v1#.}"
        num2="${v2%%.*}" v2="${v2#"$num2"}" v2="${v2#.}"
        num1=${num1:-0} num2=${num2:-0}
        if [ "$num1" -lt "$num2" ] 2>/dev/null; then return 0; fi
        if [ "$num1" -gt "$num2" ] 2>/dev/null; then return 1; fi
        if [ -z "$v1" ] && [ -z "$v2" ]; then return 1; fi
    done
}

ensure_restyle_installed() {
    if command -v restyle >/dev/null 2>&1; then
        local version
        version=$(restyle --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+' || echo "0.0.0.0")
        if version_lt "$version" "0.8.1.0"; then
            echo "[restyle-diff.sh] WARNING: restyle version $version is older than the version 0.8.1.0 which is faster and most stable"
            echo "[restyle-diff.sh] Please UPGRADE to a newer restyle CLI by running"
            echo "[restyle-diff.sh] 1. rm -f \"\$(command -v restyle)\""
            echo "[restyle-diff.sh] 2. re-run this script without using sudo (it will automatically download latest restyle-CLI version)."
            exit 1
        fi
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

# Default to skipping image pulls; image tags in .restyled.yaml are pinned, so cached images stay correct until the .restyled.yaml
# config bumps a tag. When that happens, Restyle CLI's `docker run --pull never` fails with a "No such image" error;
# if you see that error, re-run this script with -p to fetch the new image.
export PULL=False

while [[ $# -gt 0 ]]; do
    case "$1" in
        -d)
            export DEBUG=True
            shift
            ;;
        -p)
            export PULL=True
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
