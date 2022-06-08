#!/bin/bash

#  Copyright (c) 2020 Silicon Labs

#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

USAGE="Export Matter Examples to a new Location : export_examples.sh <new location> <examples path> <matter_root>"

if [ -z "$1" ]; then
    echo "Missing export location"
fi

if [ -z "$2" ]; then
    echo "Missing examples location"
fi

if [ -z "${MATTER_ROOT}" ]; then
    if [ -z "$3" ]; then
        echo "Cannot Export without the MATTER_ROOT"
        exit 1
    else
        ROOT="$3"
    fi
else
    ROOT="${MATTER_ROOT}"
fi

#1. copy folder examples and platfor/generated stuff
CURRENT_DIR=$(pwd)
mkdir -p $1

rsync -a $2 $1 --exclude=third_party --exclude=build_overrides

#2. Symlink build scripts
if [ -f "$1/../build_silabs_examples.sh" ]; then
    echo "Build scripts already linked"
else
    ln -s "${ROOT}/scripts/examples/gn_efr32_example.sh" "$1/../build_silabs_examples.sh"
fi

#3. symlink the buildoverride folder
cd $1
ln -s "${ROOT}/examples/build_overrides" "./efr32/build_overrides"

#4 symlink the third_party to point to the actual location of the matter repo
mkdir -p ./efr32/third_party
cd ./efr32/third_party

ln -s ${ROOT} ./connectedhomeip
