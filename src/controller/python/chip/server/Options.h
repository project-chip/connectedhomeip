/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Support functions for parsing command-line arguments.
 *
 */

#pragma once

#include <cstdint>

#include <lib/core/CHIPError.h>

struct LinuxDeviceOptions
{
    uint32_t mBleDevice = 0;
    bool mWiFi          = true;
    bool mThread        = false;

    static LinuxDeviceOptions & GetInstance();
};

CHIP_ERROR ParseArguments(int argc, char * argv[]);
