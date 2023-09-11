#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
set -x

branch_name=$(git branch | grep "^*" | awk '{print $2}')

git remote add upstream https://github.com/project-chip/connectedhomeip.git
git fetch upstream restyled/"$branch_name"
git merge --ff-only FETCH_HEAD
git push
