#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -x
env

mkdir -p "$1"

find . -name test-suite.log | while read -r name; do
    destination=${name#./}
    destination=$1/${destination//\//_}
    echo "Saving $name to $destination"
    cp "$name" "$destination"
done
