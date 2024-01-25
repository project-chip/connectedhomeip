#!/bin/bash

#
# Copyright (c) 2022 Project CHIP Authors
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

function _usage() {
    cat <<EOF
Invalid arguments passed.
Usage:
    zap_bootstrap.sh -> install and update required packages
    zap_bootstrap.sh -c -> run a clean bootstrap, install packages from scratch
EOF
    exit 1
}

set -e

if [ -z "$ZAP_DEVELOPMENT_PATH" ]; then
    echo 'Please set $ZAP_DEVELOPMENT_PATH to your zap checkout'
    exit 1
fi

(
    cd "$ZAP_DEVELOPMENT_PATH"

    if [ $# -eq 0 ]; then
        echo "Running ZAP bootstrap"
        if ! npm list installed-check &>/dev/null; then
            npm install installed-check
        fi

        if ! ./node_modules/.bin/installed-check -c &>/dev/null; then
            npm install
        fi
    elif [ $# -eq 1 ] && [ "$1" = "-c" ]; then
        echo "Running clean ZAP bootstrap"
        npm ci
        npm run version-stamp
        npm rebuild canvas --update-binary
        npm run build-spa
    else
        _usage
    fi
)

echo "ZAP bootstrap done!"
