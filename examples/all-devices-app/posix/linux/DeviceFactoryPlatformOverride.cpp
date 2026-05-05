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
#include <DeviceFactoryPlatformOverride.h>
#include <PosixChimeDevice.h>
#include <devices/device-factory/DeviceFactory.h>
#include <devices/proximity-ranger/DefaultProximityRangingDriver.h>
#include <devices/proximity-ranger/ProximityRangerDevice.h>

#include "LinuxBleRssiRangingAdapter.h"

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ProximityRanging;

LinuxBleRssiRangingAdapter sBleAdapter;
RangingTechnologyController sRangingController;
DefaultProximityRangingDriver sRangingDriver{ sRangingController, BitMask<Feature>(Feature::kBleBeaconRssi) };

} // namespace

namespace chip {
namespace app {

void RegisterDeviceFactoryOverrides(TimerDelegate & timerDelegate, PersistentStorageDelegate * storageDelegate)
{
    (void) sBleAdapter.Init(storageDelegate);
    (void) sRangingController.RegisterAdapter(sBleAdapter);

    DeviceFactory::GetInstance().RegisterCreator("chime", [&timerDelegate]() {
        static const ChimeDevice::Sound kDefaultSounds[] = {
            { 0, "Ding Dong"_span },
            { 1, "Ring Ring"_span },
        };
        return std::make_unique<PosixChimeDevice>(timerDelegate, Span<const ChimeDevice::Sound>(kDefaultSounds));
    });

    DeviceFactory::GetInstance().RegisterCreator("proximity-ranger", [&timerDelegate]() {
        return std::make_unique<ProximityRangerDevice>(ProximityRangerDevice::Context{
            .timerDelegate = timerDelegate,
            .driver        = sRangingDriver,
        });
    });
}

} // namespace app
} // namespace chip
