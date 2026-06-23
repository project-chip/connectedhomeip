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
#pragma once

#include <devices/proximity-ranger/ProximityRangerDevice.h>
#include <devices/proximity-ranger/impl/LoggingRangingAdapter.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/**
 * Default implementation of ProximityRangerDevice for platforms without real
 * ranging hardware. Owns a LoggingRangingAdapter for each of the three
 * technologies the cluster supports (BLE Beacon RSSI, Wi-Fi USD, Bluetooth
 * Channel Sounding) and injects them into the base ProximityRangerDevice.
 */
class LoggingProximityRangerDevice : public ProximityRangerDevice
{
public:
    LoggingProximityRangerDevice(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage);
    ~LoggingProximityRangerDevice() override = default;

    // Non-copyable / non-movable: would invalidate the adapter pointers that the base
    // ProximityRangerDevice holds into our LoggingRangingAdapter members.
    LoggingProximityRangerDevice(const LoggingProximityRangerDevice &)             = delete;
    LoggingProximityRangerDevice & operator=(const LoggingProximityRangerDevice &) = delete;
    LoggingProximityRangerDevice(LoggingProximityRangerDevice &&)                  = delete;
    LoggingProximityRangerDevice & operator=(LoggingProximityRangerDevice &&)      = delete;

private:
    Clusters::ProximityRanging::LoggingRangingAdapter mBleRangingAdapter;
    Clusters::ProximityRanging::LoggingRangingAdapter mWiFiRangingAdapter;
    Clusters::ProximityRanging::LoggingRangingAdapter mBltcsRangingAdapter;
};

} // namespace app
} // namespace chip
