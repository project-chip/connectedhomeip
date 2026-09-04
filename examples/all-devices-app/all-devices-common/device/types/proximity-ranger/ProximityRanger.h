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
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

#include <vector>

namespace chip {
namespace app {

class ProximityRanger : public SingleEndpoint
{
public:
    /**
     * Generic Proximity Ranger device. The caller injects the set of
     * RangingAdapter instances the cluster should expose; the device does not
     * own the adapters but takes a copy of the pointer list so callers do not
     * need to keep their own backing array alive.
     *
     * Each adapter pointer MUST be non-null and the underlying adapter MUST
     * outlive this device. Construction VerifyOrDies if any pointer is null.
     *
     * The cluster's feature map is derived from the technologies of the
     * supplied adapters, so the caller does not pass a feature mask in.
     */
    ProximityRanger(TimerDelegate & timerDelegate, std::vector<Clusters::ProximityRanging::RangingAdapter *> adapters);
    ~ProximityRanger() override = default;

    // Non-copyable / non-movable: copying or moving this device would invalidate the
    // adapter pointers that subclasses (e.g. LoggingProximityRanger) take to their
    // own member adapters and pass into the base.
    ProximityRanger(const ProximityRanger &)             = delete;
    ProximityRanger & operator=(const ProximityRanger &) = delete;
    ProximityRanger(ProximityRanger &&)                  = delete;
    ProximityRanger & operator=(ProximityRanger &&)      = delete;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::ProximityRanging::ProximityRangingCluster & ProximityRangingCluster() { return mProximityRangingCluster.Cluster(); }

private:
    /// Derive the cluster's feature map from the supplied adapters.
    ///
    /// Cannot be cached as a member computed in the constructor: a derived
    /// class (e.g. LoggingProximityRanger) typically owns the adapter
    /// objects as its own members, and base-class member init runs before
    /// those subclass members are constructed. Calling adapter->GetTechnology()
    /// at that point is a virtual call on a not-yet-constructed object —
    /// undefined behavior. Register() runs after construction completes, so
    /// it's safe to derive there.
    BitMask<Clusters::ProximityRanging::Feature> DeriveFeatures() const;

    TimerDelegate & mTimerDelegate;
    const std::vector<Clusters::ProximityRanging::RangingAdapter *> mAdapters;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::ProximityRanging::ProximityRangingCluster> mProximityRangingCluster;

    bool mRegistered = false;
};

} // namespace app
} // namespace chip
