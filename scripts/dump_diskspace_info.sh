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

# This script is meant to run in github CI to dump information related to
# disk space usage. Currently there is a heisenbug related to running out
# of disk space. Dumping information below might help us get a better
# understanding of how disk space is used in github CI.

echo "Disk Space Usage:"
df -h

listOfDirectories=("third_party/" ".environment/" "out/")

pipCacheDir=$(python -m pip cache dir)
exitcode=$?

if [ "$exitcode" -eq 0 ]; then
    listOfDirectories+=("$pipCacheDir")
fi

echo
echo "Storage Space Used By Key Directories:"
for directory in "${listOfDirectories[@]}"; do
    if [ -d "$directory" ]; then
        du -d1 -h "$directory" | sort -h
        echo
    fi
done
