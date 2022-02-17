#!/usr/bin/env bash
#
#
#    Copyright (c) 2022 Project CHIP Authors
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

#
#    Description:
#      Run this from root of SDK to regenerate only the ZAP needed by chip-tool,
#      rather than all of zap like `./scripts/tools/zap_regen_all.py
#

./scripts/tools/zap/generate.py src/controller/data_model/controller-clusters.zap \
    -o zzz_generated/chip-tool/zap-generated -t examples/chip-tool/templates/templates.json
