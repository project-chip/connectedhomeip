#!/usr/bin/env bash

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

# Use git and restyle-path to help reformat anything that differs from
#  some base edit point.  Use before pushing a PR to make sure everything
#  you've written is kosher to CI
#
# Usage:
#  restyle-diff.sh [-d] [ref]
#
# if unspecified, ref defaults to upstream/master (or master)
# -d sets container's log level to DEBUG, if unspecified the default log level will remain (info level)
#

here=${0%/*}

set -e

CHIP_ROOT=$(cd "$here/../.." && pwd)
cd "$CHIP_ROOT"

docker_run() {
    if [ -t 0 ]; then
        exec docker run --tty "$@"

    else
        exec docker run "$@"

    fi
}

restyle-paths() {

    image=restyled/restyler:edge

    for path in "$@"; do
        (
            docker_run --tty --interactive --rm \
                --env LOG_LEVEL \
                --env LOG_DESTINATION \
                --env LOG_FORMAT \
                --env LOG_COLOR \
                --env HOST_DIRECTORY="$PWD" \
                --env UNRESTRICTED=1 \
                --volume "$PWD":/code \
                --volume /tmp:/tmp \
                --volume /var/run/docker.sock:/var/run/docker.sock \
                --entrypoint restyle-path \
                "$image" "$path"
        )
    done
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -d)
            export LOG_LEVEL="DEBUG"
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

mapfile -t paths < <(git diff --ignore-submodules --name-only --merge-base "$ref")
restyle-paths "${paths[@]}"
