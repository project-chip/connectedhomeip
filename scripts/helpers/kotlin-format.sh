#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
