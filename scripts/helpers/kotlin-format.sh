#!/usr/bin/env bash

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

here=${0%/*}

set -e

CHIP_ROOT=$(cd "$here/../.." && pwd)
cd "$CHIP_ROOT"

if ! command -v java &>/dev/null; then
    echo "Java is not installed. Please install Java and try again."
    exit 1
fi

if [ ! -f "/tmp/ktfmt-0.44-jar-with-dependencies.jar" ]; then
    wget "https://repo1.maven.org/maven2/com/facebook/ktfmt/0.44/ktfmt-0.44-jar-with-dependencies.jar" -P /tmp
fi

find src examples -name '*.kt' | xargs java -jar /tmp/ktfmt-0.44-jar-with-dependencies.jar --google-style
