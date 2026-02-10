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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/support/LinkedList.h>
namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class Instance
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature, OptionalAttributes aOptionalAttrs,
             OptionalCommands aOptionalCmds);

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;
    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;
    bool SupportsOptCmd(OptionalCommands aOptionalCmds) const;

    // Attribute accessors - pass through to cluster
    StateEnum GetState() const;
    CHIP_ERROR SetState(StateEnum newValue);

    SupplyStateEnum GetSupplyState() const;
    CHIP_ERROR SetSupplyState(SupplyStateEnum newValue);

    FaultStateEnum GetFaultState() const;
    CHIP_ERROR SetFaultState(FaultStateEnum newValue);

    DataModel::Nullable<uint32_t> GetChargingEnabledUntil() const;
    CHIP_ERROR SetChargingEnabledUntil(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<uint32_t> GetDischargingEnabledUntil() const;
    CHIP_ERROR SetDischargingEnabledUntil(DataModel::Nullable<uint32_t> newValue);

    int64_t GetCircuitCapacity() const;
    CHIP_ERROR SetCircuitCapacity(int64_t newValue);

    int64_t GetMinimumChargeCurrent() const;
    CHIP_ERROR SetMinimumChargeCurrent(int64_t newValue);

    int64_t GetMaximumChargeCurrent() const;
    CHIP_ERROR SetMaximumChargeCurrent(int64_t newValue);

    int64_t GetMaximumDischargeCurrent() const;
    CHIP_ERROR SetMaximumDischargeCurrent(int64_t newValue);

    int64_t GetUserMaximumChargeCurrent() const;
    CHIP_ERROR SetUserMaximumChargeCurrent(int64_t newValue);

    uint32_t GetRandomizationDelayWindow() const;
    CHIP_ERROR SetRandomizationDelayWindow(uint32_t newValue);

    DataModel::Nullable<uint32_t> GetNextChargeStartTime() const;
    CHIP_ERROR SetNextChargeStartTime(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<uint32_t> GetNextChargeTargetTime() const;
    CHIP_ERROR SetNextChargeTargetTime(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<int64_t> GetNextChargeRequiredEnergy() const;
    CHIP_ERROR SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> newValue);

    DataModel::Nullable<Percent> GetNextChargeTargetSoC() const;
    CHIP_ERROR SetNextChargeTargetSoC(DataModel::Nullable<Percent> newValue);

    DataModel::Nullable<uint16_t> GetApproximateEVEfficiency() const;
    CHIP_ERROR SetApproximateEVEfficiency(DataModel::Nullable<uint16_t> newValue);

    DataModel::Nullable<Percent> GetStateOfCharge() const;
    CHIP_ERROR SetStateOfCharge(DataModel::Nullable<Percent> newValue);

    DataModel::Nullable<int64_t> GetBatteryCapacity() const;
    CHIP_ERROR SetBatteryCapacity(DataModel::Nullable<int64_t> newValue);

    DataModel::Nullable<CharSpan> GetVehicleID() const;
    CHIP_ERROR SetVehicleID(DataModel::Nullable<CharSpan> newValue);

    DataModel::Nullable<uint32_t> GetSessionID() const;
    CHIP_ERROR SetSessionID(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<uint32_t> GetSessionDuration() const;
    CHIP_ERROR SetSessionDuration(DataModel::Nullable<uint32_t> newValue);

    DataModel::Nullable<int64_t> GetSessionEnergyCharged() const;
    CHIP_ERROR SetSessionEnergyCharged(DataModel::Nullable<int64_t> newValue);

    DataModel::Nullable<int64_t> GetSessionEnergyDischarged() const;
    CHIP_ERROR SetSessionEnergyDischarged(DataModel::Nullable<int64_t> newValue);

private:
    RegisteredServerCluster<EnergyEvseCluster> mCluster;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
