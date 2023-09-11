/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the Chip
 *          Device Layer on the platform.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {

/**
 * Represents platform-specific event information for the platforms.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
        /* None currently defined */
    };
};

} // namespace DeviceLayer
} // namespace chip
