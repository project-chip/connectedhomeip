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

set -x

cd /tmp
rm -rf connectedhomeip
git clone --recurse-submodules https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Generate Coverage Report
./scripts/build_coverage.sh 2>&1 | tee /tmp/matter_build.log

# Generate Conformance Report
source scripts/activate.sh
./scripts/build_python.sh -i out/python_env
python3 -u scripts/examples/conformance_report.py
cp /tmp/conformance_report/conformance_report.html out/coverage/coverage/html

# Upload
cd out/coverage/coverage
gcloud app deploy webapp_config.yaml 2>&1 | tee /tmp/matter_publish.log
versions=$(gcloud app versions list \
    --service default \
    --sort-by '~VERSION.ID' \
    --format 'value(VERSION.ID)' | sed 1,5d)
for version in "$versions"; do
    gcloud app versions delete "$version" \
        --service default \
        --quiet
done
