/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for the Texas Instruments CC32XX platform.
 *
 * NOTE: currently a bare-bones implementation to allow for building.
 */

#ifndef CHIP_DEVICE_PLATFORM_EVENT_H
#define CHIP_DEVICE_PLATFORM_EVENT_H

#include <platform/CHIPDeviceEvent.h>

// XXX: Seth, what device events do we need

namespace chip {
namespace DeviceLayer {

/**
 * Represents platform-specific event information
 */
struct ChipDevicePlatformEvent final
{
};

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_PLATFORM_EVENT_H
