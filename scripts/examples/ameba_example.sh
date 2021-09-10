#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
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

set -x
env

app="$1"
root=examples/$app/Ameba/

if [ -z "$app" ]; then
    echo "No mandatory app argument supplied!"
    exit 1
fi

source ../../scripts/activate.sh

cd ${AMEBA_PATH}/project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp
make clean
make all

cd ${AMEBA_PATH}/project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp
make clean
make -C asdk lib_all
make all

cp "$AMEBA_PATH"/project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp/asdk/image/km0_boot_all.bin /root/chip/examples/all-clusters-app/Ameba/build/chip/gen/km0_boot_all.bin
cp "$AMEBA_PATH"/project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/asdk/image/km4_boot_all.bin /root/chip/examples/all-clusters-app/Ameba/build/chip/gen/km4_boot_all.bin
cp "$AMEBA_PATH"/project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/asdk/image/km0_km4_image2.bin /root/chip/examples/all-clusters-app/Ameba/build/chip/gen/km0_km4_image2.bin
