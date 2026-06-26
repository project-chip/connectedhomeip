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
#include <devices/proximity-ranger/impl/LoggingProximityRangerDevice.h>

namespace chip {
namespace app {

// Note on member initialization order: the base ProximityRangerDevice is
// initialized first and is handed pointers to the three LoggingRangingAdapter
// members. The members' storage is laid out at object-construction time, so
// taking their addresses here is well-defined. The base only stores the
// pointer list — it does not dereference until Register() runs, by which
// point all members are fully constructed.
LoggingProximityRangerDevice::LoggingProximityRangerDevice(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage) :
    ProximityRangerDevice(timerDelegate, { &mBleRangingAdapter, &mWiFiRangingAdapter, &mBltcsRangingAdapter }),
    mBleRangingAdapter(Clusters::ProximityRanging::RangingTechEnum::kBLEBeaconRSSIRanging, timerDelegate, &storage,
                       /*periodicRangingSupport=*/true),
    mWiFiRangingAdapter(Clusters::ProximityRanging::RangingTechEnum::kWiFiRoundTripTimeRanging, timerDelegate),
    mBltcsRangingAdapter(Clusters::ProximityRanging::RangingTechEnum::kBluetoothChannelSounding, timerDelegate)
{}

} // namespace app
} // namespace chip
