#!/usr/bin/env bash
#
#   Copyright (c) 2020 Project CHIP Authors
#   All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

# If this file is sourced, it exports a function called "idf" that initializes
# the espressif environment via the espressive export.sh script and runs
# a command presented as arguments
#
# This file can also be used as an executable

CURR_PATH=$(
    test -d "${0%/*}" && cd "${0%/*}"
    pwd
)
CHIP_PATH=$CURR_PATH/../../..
AMEBA_PATH=$CHIP_PATH/../ambd_sdk_with_chip_non_NDA
AMEBA_LP_PATH=$AMEBA_PATH/project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp
AMEBA_HP_PATH=$AMEBA_PATH/project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp
AMEBA_CHIP_MAIN=$AMEBA_HP_PATH/asdk/make/chip_main

cd "$CHIP_PATH"
source scripts/activate.sh

cd "$AMEBA_LP_PATH"
make clean
make all

cd "$AMEBA_HP_PATH"
make clean
make -C asdk lib_all
make all
