#
# SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

FROM connectedhomeip/chip-cirque-device-base:latest

COPY out/debug/chip-bridge-app /usr/bin/
COPY entrypoint.sh /

ENTRYPOINT ["/entrypoint.sh", "server"]
