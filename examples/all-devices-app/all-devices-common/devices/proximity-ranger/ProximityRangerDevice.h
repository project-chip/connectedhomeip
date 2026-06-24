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
#include <devices/interface/SingleEndpointDevice.h>
#include <devices/proximity-ranger/DefaultProximityRangingDriver.h>
#include <devices/proximity-ranger/RangingAdapter.h>
#include <lib/support/BitMask.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class ProximityRangerDevice : public SingleEndpointDevice
{
public:
    ProximityRangerDevice(TimerDelegate & timerDelegate, Span<Clusters::ProximityRanging::RangingAdapter * const> adapters);
    ~ProximityRangerDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    Clusters::ProximityRanging::DefaultProximityRangingDriver mRangingDriver;
    TimerDelegate & mTimerDelegate;
    Span<Clusters::ProximityRanging::RangingAdapter * const> mAdapters; // Backing array must outlive this device

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::ProximityRanging::ProximityRangingCluster> mProximityRangingCluster;

    bool mRegistered = false;

    // Ref-counts ProximityRangerDevice instances that have completed Register so the
    // shared adapters remain registered with the singleton RangingTechnologyController
    // as long as at least one device is registered. Adapters are unregistered only
    // when the last ProximityRangerDevice is unregistered.
    //
    // ASSUMES every ProximityRangerDevice instance is constructed with the same adapter
    // set. A heterogeneous configuration (e.g. one device with BLE+UWB, another with
    // BLE only) could leak adapters from the larger span if the last device of that
    // flavor unregisters while a smaller-span device is still active.
    static size_t sActiveCount;
};

} // namespace app
} // namespace chip
