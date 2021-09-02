/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides implementations of the CHIP System Layer platform
 *          event functions that are suitable for use on all platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace System {

using namespace ::chip::DeviceLayer;

CHIP_ERROR PlatformEventing::PostEvent(System::Layer & aLayer, System::Object & aTarget, System::EventType aType,
                                       uintptr_t aArgument)
{
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kChipSystemLayerEvent;
    event.ChipSystemLayerEvent.Type     = aType;
    event.ChipSystemLayerEvent.Target   = &aTarget;
    event.ChipSystemLayerEvent.Argument = aArgument;

    PlatformMgr().PostEvent(&event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformEventing::DispatchEvents(System::Layer & aLayer)
{
    PlatformMgr().RunEventLoop();

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformEventing::DispatchEvent(System::Layer & aLayer, const ChipDeviceEvent * aEvent)
{
    PlatformMgr().DispatchEvent(aEvent);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformEventing::StartTimer(System::Layer & aLayer, uint32_t aMilliseconds)
{
    return PlatformMgr().StartChipTimer(aMilliseconds);
}

} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
