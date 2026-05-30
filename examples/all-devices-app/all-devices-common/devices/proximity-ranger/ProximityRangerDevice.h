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
#include <app/clusters/proximity-ranging-server/RangingAdapter.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/BitMask.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class ProximityRangerDevice : public SingleEndpointDevice
{
public:
    /**
     * Returns the process-wide ProximityRangingDriver shared by every
     * ProximityRangerDevice instance. The driver is lazily initialized
     * on the first call with the supplied adapter set; subsequent calls
     * return the same instance regardless of `adapters`.
     *
     * Sharing a single driver across devices keeps adapter Callback
     * registrations stable when devices are constructed and destroyed
     * in sequence: each ProximityRangingDriver constructor registers
     * itself as the adapter callback, so per-device drivers would
     * alias and overwrite each other.
     *
     * The backing array referenced by `adapters` MUST outlive the
     * process, since the singleton driver lives until program exit.
     *
     * The constructor of ProximityRangerDevice triggers initialization;
     * external callers that just need a reference to the existing
     * driver may omit `adapters`.
     */
    static Clusters::ProximityRanging::ProximityRangingDriver &
    GetRangingDriver(Span<Clusters::ProximityRanging::RangingAdapter * const> adapters = {});

    ProximityRangerDevice(TimerDelegate & timerDelegate, Span<Clusters::ProximityRanging::RangingAdapter * const> adapters,
                          BitMask<Clusters::ProximityRanging::Feature> features);
    ~ProximityRangerDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
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
