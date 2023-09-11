#!/usr/bin/env bash
#
#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

#
#    Description:
#      Run this from root of SDK to regenerate only the ZAP needed by chip-tool,
#      rather than all of zap like `./scripts/tools/zap_regen_all.py
#

./scripts/tools/zap_regen_all.py --type tests
