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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/proximity-ranging-server/ProximityRangingCluster.h>
#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/BitMask.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class ProximityRangerDevice : public SingleEndpointDevice
{
public:
    /**
     * Returns the process-wide ProximityRangingDriver shared by every
     * ProximityRangerDevice instance. Construction of the first
     * ProximityRangerDevice initializes the driver with a fixed set of
     * LoggingRangingAdapters (BLE-RSSI, WiFi USD, BLT-CS); subsequent
     * constructions reuse the same instance.
     *
     * Sharing a single driver across devices keeps adapter Callback
     * registrations stable when devices are constructed and destroyed
     * in sequence: each ProximityRangingDriver constructor registers
     * itself as the adapter callback, so per-device drivers would
     * alias and overwrite each other.
     *
     * MUST NOT be called before the first ProximityRangerDevice is
     * constructed.
     */
    static Clusters::ProximityRanging::ProximityRangingDriver & GetRangingDriver();

    ProximityRangerDevice(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage,
                          BitMask<Clusters::ProximityRanging::Feature> features);
    ~ProximityRangerDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    TimerDelegate & mTimerDelegate;

    BitMask<Clusters::ProximityRanging::Feature> mFeatures;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::ProximityRanging::ProximityRangingCluster> mProximityRangingCluster;

    bool mRegistered = false;
};

} // namespace app
} // namespace chip
