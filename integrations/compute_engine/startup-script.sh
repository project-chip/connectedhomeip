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

set -e

cd /tmp
rm -rf connectedhomeip
git clone --recurse-submodules https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Make sure the scripts are executable
chmod +x integrations/compute_engine/automated_reports.sh
chmod +x integrations/compute_engine/run_alchemy_diff.sh

# Run all reports and deploy
./integrations/compute_engine/automated_reports.sh --all --deploy --pat-secret github-read-only-pat
