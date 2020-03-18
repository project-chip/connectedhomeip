/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Defines platform-specific event types and data for the Weave
 *          Device Layer on EFR32 platforms using the Silicon Labs SDK.
 */

#ifndef WEAVE_DEVICE_PLATFORM_EVENT_H
#define WEAVE_DEVICE_PLATFORM_EVENT_H

#include <Weave/DeviceLayer/WeaveDeviceEvent.h>

namespace nl {
namespace Weave {
namespace System {
class PacketBuffer;
}
} // namespace Weave
} // namespace nl

namespace nl {
namespace Weave {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates EFR32 platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates EFR32 platform-specific event types that are internal to the Weave Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    /* None currently defined */
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Silicon Labs EFR32 platforms.
 */

struct WeaveDevicePlatformEvent final
{
    union
    {
        /* None currently defined */
    };
};

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // WEAVE_DEVICE_PLATFORM_EVENT_H
