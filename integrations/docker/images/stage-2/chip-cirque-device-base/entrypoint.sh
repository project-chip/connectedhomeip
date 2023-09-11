#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

service dbus start
service avahi-daemon start
/bin/CHIPCirqueDaemon.py --server otbr-agent
