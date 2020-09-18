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

set -x

git clean -Xdf

git submodule foreach git clean -Xdf

# TODO - Remove after fixing
# https://bugs.chromium.org/p/pigweed/issues/detail?id=265
if test -n "$PW_ENVIRONMENT_ROOT" -a -w "$PW_ENVIRONMENT_ROOT"; then
    rm -rf "$PW_ENVIRONMENT_ROOT"
fi
