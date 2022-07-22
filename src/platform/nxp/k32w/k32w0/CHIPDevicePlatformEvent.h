/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on K32W061 platforms using the NXP SDK.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates K32W061 platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates K32W061 platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    /* None currently defined */
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for NXP K32W061 platforms.
 */

struct ChipDevicePlatformEvent final
{
};

} // namespace DeviceLayer
} // namespace chip
