/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>
#include <app/clusters/energy-evse-server/EnergyEvseDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class MockEvseDelegate : public Delegate
{
public:
    MockEvseDelegate() : Delegate(), mCluster(nullptr) {}
    ~MockEvseDelegate() = default;

    void SetCluster(EnergyEvseCluster & aCluster) { mCluster = &aCluster; }

    // ------------------------------------------------------------------
    // Command handlers (implementations in MockEvseDelegate.cpp)
    Protocols::InteractionModel::Status Disable() override;
    Protocols::InteractionModel::Status EnableCharging(const DataModel::Nullable<uint32_t> & enableChargeTime,
                                                       const int64_t & minimumChargeCurrent,
                                                       const int64_t & maximumChargeCurrent) override;
    Protocols::InteractionModel::Status EnableDischarging(const DataModel::Nullable<uint32_t> & enableDischargeTime,
                                                          const int64_t & maximumDischargeCurrent) override;
    Protocols::InteractionModel::Status StartDiagnostics() override;
    Protocols::InteractionModel::Status SetTargets(
        const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules) override;
    Protocols::InteractionModel::Status LoadTargets() override;
    Protocols::InteractionModel::Status
    GetTargets(DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules) override;
    Protocols::InteractionModel::Status ClearTargets() override;

    // ------------------------------------------------------------------
    // Test helper getters (for verifying targets state)
    size_t GetTotalTargetsCount() const { return mTotalTargetsCount; }
    uint8_t GetDaysWithTargets() const { return mDaysWithTargets; }

    // Maximum targets per day and total (for mock purposes)
    static constexpr size_t kMaxTargetsPerDay = 10;
    static constexpr size_t kMaxTotalTargets  = 70; // 7 days * 10 targets

private:
    EnergyEvseCluster * mCluster;

    // Targets storage
    size_t mTotalTargetsCount = 0;
    uint8_t mDaysWithTargets  = 0;                                      // Bitmask of days that have targets
    Structs::ChargingTargetStruct::Type mTargets[7][kMaxTargetsPerDay]; // 7 days, up to kMaxTargetsPerDay each
    size_t mTargetsPerDay[7] = { 0 };
    Structs::ChargingTargetScheduleStruct::Type mSchedules[7]; // For GetTargets response
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
