#
# SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

ARG VERSION=1
FROM ghcr.io/project-chip/chip-cirque-device-base:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

COPY out/debug/chip-lighting-app /usr/bin/
COPY entrypoint.sh /

ENTRYPOINT ["/entrypoint.sh", "server"]
