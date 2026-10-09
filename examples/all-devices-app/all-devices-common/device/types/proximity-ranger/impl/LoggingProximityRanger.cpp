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
#include <device/types/proximity-ranger/impl/LoggingProximityRanger.h>
#include <iterator>
#include <lib/support/CodeUtils.h>
#include <vector>

namespace chip {
namespace app {

// Note on member initialization order: the base ProximityRanger is
// initialized first and is handed pointers to the three LoggingRangingAdapter
// members. The members' storage is laid out at object-construction time, so
// taking their addresses here is well-defined. The base only stores the
// pointer list — it does not dereference until Register() runs, by which
// point all members are fully constructed.
LoggingProximityRanger::LoggingProximityRanger(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage) :
    ProximityRanger(timerDelegate, { &mBleRangingAdapter, &mWiFiRangingAdapter, &mBltcsRangingAdapter },
                    Clusters::ProximityRanging::ProximityRangingCluster::OptionalAttributeSet()
                        .Set<Clusters::ProximityRanging::Attributes::RangingConstraints::Id>()),
    mBleRangingAdapter(Clusters::ProximityRanging::RangingTechEnum::kBLEBeaconRSSIRanging, timerDelegate, &storage,
                       /*periodicRangingSupport=*/true),
    mWiFiRangingAdapter(Clusters::ProximityRanging::RangingTechEnum::kWiFiRoundTripTimeRanging, timerDelegate),
    mBltcsRangingAdapter(Clusters::ProximityRanging::RangingTechEnum::kBluetoothChannelSounding, timerDelegate)
{}

// Returns RangingAdapter based on technology enum
Clusters::ProximityRanging::LoggingRangingAdapter *
LoggingProximityRanger::AdapterFor(Clusters::ProximityRanging::RangingTechEnum technology)
{
    using Clusters::ProximityRanging::RangingTechEnum;
    switch (technology)
    {
    case RangingTechEnum::kBLEBeaconRSSIRanging:
        return &mBleRangingAdapter;
    case RangingTechEnum::kWiFiRoundTripTimeRanging:
        return &mWiFiRangingAdapter;
    case RangingTechEnum::kBluetoothChannelSounding:
        return &mBltcsRangingAdapter;
    default:
        return nullptr;
    }
}

// Set accessor updating the RangingConstraints published by this device:
// Each entry will be routed to the RangingAdapter matching its technology.
// adapters not named by any entry are cleared. Fails with CHIP_ERROR_INVALID_ARGUMENT,
// when an entry names a technology this device has no adapter for.
CHIP_ERROR LoggingProximityRanger::SetRangingConstraints(
    Span<const Clusters::ProximityRanging::Structs::RangingConstraintStruct::Type> constraints)
{
    using Clusters::ProximityRanging::LoggingRangingAdapter;
    using RangingConstraint = Clusters::ProximityRanging::Structs::RangingConstraintStruct::Type;

    LoggingRangingAdapter * const adapters[] = { &mBleRangingAdapter, &mWiFiRangingAdapter, &mBltcsRangingAdapter };

    std::vector<RangingConstraint> perAdapter[std::size(adapters)];
    for (const auto & entry : constraints)
    {
        LoggingRangingAdapter * adapter = AdapterFor(entry.technology);
        VerifyOrReturnError(adapter != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        for (size_t i = 0; i < std::size(adapters); i++)
            if (adapters[i] == adapter)
                perAdapter[i].push_back(entry);
    }
    for (size_t i = 0; i < std::size(adapters); i++)
        adapters[i]->SetConstraints(Span<const RangingConstraint>(perAdapter[i].data(), perAdapter[i].size()));

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
