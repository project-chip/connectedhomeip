#
# SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

ARG VERSION=1
FROM connectedhomeip/chip-cirque-device-base:${VERSION}

COPY out/debug/chip-resource-monitoring-app /usr/bin/
COPY entrypoint.sh /

ENTRYPOINT ["/entrypoint.sh", "server"]
