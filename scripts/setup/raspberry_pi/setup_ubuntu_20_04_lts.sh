#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# Installs required packages and configuration for Raspberry Pi to be able
# to build CHIP and run example/integration applications
#
# See docs/guides/BUILDING.md for more details

set -ex

sudo apt-get install -fy \
    g++ \
    gcc \
    git \
    libavahi-client-dev \
    libcairo2-dev \
    libdbus-1-dev \
    libgirepository1.0-dev \
    libglib2.0-dev \
    libssl-dev \
    ninja-build \
    pi-bluetooth \
    pkg-config \
    protobuf-compiler \
    python \
    python3-dev \
    python3-venv \
    unzip &&
    true

echo "You must REBOOT after pi-bluetooth has been installed"
