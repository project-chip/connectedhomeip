/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *    Provides a class that serves as a wrapper for the event system of CYW30739
 *    platform's underlying RTOS. An event instance is comprised of 32 flags.
 *    Each flag can be utilized for thread synchronization purposes.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <wiced_rtos.h>

namespace chip {
namespace DeviceLayer {

/**
 * A class represents an event group with 32 flags.
 */
class EventFlags
{
public:
    CHIP_ERROR Init(void);
    CHIP_ERROR Set(uint32_t flags);
    CHIP_ERROR WaitAnyForever(uint32_t & flags);

private:
    wiced_event_flags_t * mFlags;
};

} // namespace DeviceLayer
} // namespace chip
