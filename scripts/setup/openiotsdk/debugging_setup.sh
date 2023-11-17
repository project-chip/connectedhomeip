#!/bin/bash

#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Installing FVP Fast Model extension in /opt directory

FAST_MODEL_EXT_DIR=/opt/FastModels_ThirdParty_IP_11-16_b16_Linux64

if [ ! -d "$FAST_MODEL_EXT_DIR" ]; then
    echo "Fast Model Extension package directory: $FAST_MODEL_EXT_DIR does not exists. Ensure it is mounted." >&2
    exit 1
fi

echo "Install Fast Model extension in /opt" >&2
cd "$FAST_MODEL_EXT_DIR"
sudo ./setup.bin --i-accept-the-license-agreement --basepath /opt
cd -
