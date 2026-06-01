/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "LinuxBleRssiRangingAdapter.h"
#include <DeviceFactoryPlatformOverride.h>
#include <PosixChimeDevice.h>
#include <app_config/enabled_devices.h>
#include <devices/device-factory/DeviceFactory.h>
#include <devices/proximity-ranger/ProximityRangerDevice.h>

namespace chip {
namespace app {

void RegisterDeviceFactoryOverrides(TimerDelegate & timerDelegate, PersistentStorageDelegate * storageDelegate)
{
    if constexpr (ALL_DEVICES_ENABLE_CHIME)
    {
        DeviceFactory::GetInstance().RegisterCreator("chime", [&timerDelegate]() {
            static const ChimeDevice::Sound kDefaultSounds[] = {
                { 0, "Ding Dong"_span },
                { 1, "Ring Ring"_span },
            };
            return std::make_unique<PosixChimeDevice>(timerDelegate, Span<const ChimeDevice::Sound>(kDefaultSounds));
        });
    }

    if constexpr (ALL_DEVICES_ENABLE_PROXIMITY_RANGER)
    {
        static LinuxBleRssiRangingAdapter sBleAdapter;
        LogErrorOnFailure(sBleAdapter.Init(storageDelegate));
        DeviceFactory::GetInstance().RegisterCreator("proximity-ranger", [&timerDelegate]() {
            static Clusters::ProximityRanging::RangingAdapter * adapters[] = { &sBleAdapter };
            return std::make_unique<ProximityRangerDevice>(timerDelegate,
                                                           Span<Clusters::ProximityRanging::RangingAdapter * const>(adapters));
        });
    }
}

} // namespace app
} // namespace chip
