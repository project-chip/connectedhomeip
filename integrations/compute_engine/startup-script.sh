#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

cd /tmp
rm -rf connectedhomeip
git clone --recurse-submodules https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip
./scripts/build_coverage.sh 2>&1 | tee /tmp/matter_build.log
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
