#!/bin/bash
#
#   Copyright (c) 2020 Project CHIP Authors
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
#
if [ -z ${EFR32_SDK_ROOT+x} ]; then echo "EFR32_SDK_ROOT must be set";
else echo "Using EFR32 SDK Located in '$EFR32_SDK_ROOT'";
fi
export CHIP_ROOT=$(cd ../../../ && pwd)
export EFR32_TOOLS_ROOT=$(cd $EFR32_SDK_ROOT/../../../.. && pwd)
export FREERTOS_ROOT=$(cd $EFR32_SDK_ROOT/util/third_party/freertos && pwd)