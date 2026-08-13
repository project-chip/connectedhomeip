/*
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

#include <app/clusters/energy-evse-server/CodegenIntegration.h>

#include <app/util/generic-callbacks.h>
#include <clusters/EnergyEvse/Metadata.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

Instance::Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature, OptionalAttributes aOptionalAttrs,
                   OptionalCommands aOptionalCmds) :
    mCluster(EnergyEvseCluster::Config(aEndpointId, aDelegate, aFeature, aOptionalAttrs, aOptionalCmds))
{}

CHIP_ERROR Instance::Init()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(EnergyEvse::Id), err.Format());
    }
    return err;
}

void Instance::Shutdown()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                     mCluster.Cluster().GetPaths()[0].mEndpointId, ChipLogValueMEI(EnergyEvse::Id), err.Format());
    }
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mCluster.Cluster().Features().Has(aFeature);
}

bool Instance::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    return mCluster.Cluster().OptionalAttrs().Has(aOptionalAttrs);
}

bool Instance::SupportsOptCmd(OptionalCommands aOptionalCmds) const
{
    return mCluster.Cluster().OptionalCmds().Has(aOptionalCmds);
}

StateEnum Instance::GetState() const
{
    return mCluster.Cluster().GetState();
}

CHIP_ERROR Instance::SetState(StateEnum newValue)
{
    return mCluster.Cluster().SetState(newValue);
}

SupplyStateEnum Instance::GetSupplyState() const
{
    return mCluster.Cluster().GetSupplyState();
}

CHIP_ERROR Instance::SetSupplyState(SupplyStateEnum newValue)
{
    return mCluster.Cluster().SetSupplyState(newValue);
}

FaultStateEnum Instance::GetFaultState() const
{
    return mCluster.Cluster().GetFaultState();
}

CHIP_ERROR Instance::SetFaultState(FaultStateEnum newValue)
{
    return mCluster.Cluster().SetFaultState(newValue);
}

DataModel::Nullable<uint32_t> Instance::GetChargingEnabledUntil() const
{
    return mCluster.Cluster().GetChargingEnabledUntil();
}

CHIP_ERROR Instance::SetChargingEnabledUntil(DataModel::Nullable<uint32_t> newValue)
{
    return mCluster.Cluster().SetChargingEnabledUntil(newValue);
}

DataModel::Nullable<uint32_t> Instance::GetDischargingEnabledUntil() const
{
    return mCluster.Cluster().GetDischargingEnabledUntil();
}

CHIP_ERROR Instance::SetDischargingEnabledUntil(DataModel::Nullable<uint32_t> newValue)
{
    return mCluster.Cluster().SetDischargingEnabledUntil(newValue);
}

int64_t Instance::GetCircuitCapacity() const
{
    return mCluster.Cluster().GetCircuitCapacity();
}

CHIP_ERROR Instance::SetCircuitCapacity(int64_t newValue)
{
    return mCluster.Cluster().SetCircuitCapacity(newValue);
}

int64_t Instance::GetMinimumChargeCurrent() const
{
    return mCluster.Cluster().GetMinimumChargeCurrent();
}

CHIP_ERROR Instance::SetMinimumChargeCurrent(int64_t newValue)
{
    return mCluster.Cluster().SetMinimumChargeCurrent(newValue);
}

int64_t Instance::GetMaximumChargeCurrent() const
{
    return mCluster.Cluster().GetMaximumChargeCurrent();
}

CHIP_ERROR Instance::SetMaximumChargeCurrent(int64_t newValue)
{
    return mCluster.Cluster().SetMaximumChargeCurrent(newValue);
}

int64_t Instance::GetMaximumDischargeCurrent() const
{
    return mCluster.Cluster().GetMaximumDischargeCurrent();
}

CHIP_ERROR Instance::SetMaximumDischargeCurrent(int64_t newValue)
{
    return mCluster.Cluster().SetMaximumDischargeCurrent(newValue);
}

int64_t Instance::GetUserMaximumChargeCurrent() const
{
    return mCluster.Cluster().GetUserMaximumChargeCurrent();
}

CHIP_ERROR Instance::SetUserMaximumChargeCurrent(int64_t newValue)
{
    return mCluster.Cluster().SetUserMaximumChargeCurrent(newValue);
}

uint32_t Instance::GetRandomizationDelayWindow() const
{
    return mCluster.Cluster().GetRandomizationDelayWindow();
}

CHIP_ERROR Instance::SetRandomizationDelayWindow(uint32_t newValue)
{
    return mCluster.Cluster().SetRandomizationDelayWindow(newValue);
}

DataModel::Nullable<uint32_t> Instance::GetNextChargeStartTime() const
{
    return mCluster.Cluster().GetNextChargeStartTime();
}

CHIP_ERROR Instance::SetNextChargeStartTime(DataModel::Nullable<uint32_t> newValue)
{
    return mCluster.Cluster().SetNextChargeStartTime(newValue);
}

DataModel::Nullable<uint32_t> Instance::GetNextChargeTargetTime() const
{
    return mCluster.Cluster().GetNextChargeTargetTime();
}

CHIP_ERROR Instance::SetNextChargeTargetTime(DataModel::Nullable<uint32_t> newValue)
{
    return mCluster.Cluster().SetNextChargeTargetTime(newValue);
}

DataModel::Nullable<int64_t> Instance::GetNextChargeRequiredEnergy() const
{
    return mCluster.Cluster().GetNextChargeRequiredEnergy();
}

CHIP_ERROR Instance::SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> newValue)
{
    return mCluster.Cluster().SetNextChargeRequiredEnergy(newValue);
}

DataModel::Nullable<Percent> Instance::GetNextChargeTargetSoC() const
{
    return mCluster.Cluster().GetNextChargeTargetSoC();
}

CHIP_ERROR Instance::SetNextChargeTargetSoC(DataModel::Nullable<Percent> newValue)
{
    return mCluster.Cluster().SetNextChargeTargetSoC(newValue);
}

DataModel::Nullable<uint16_t> Instance::GetApproximateEVEfficiency() const
{
    return mCluster.Cluster().GetApproximateEVEfficiency();
}

CHIP_ERROR Instance::SetApproximateEVEfficiency(DataModel::Nullable<uint16_t> newValue)
{
    return mCluster.Cluster().SetApproximateEVEfficiency(newValue);
}

DataModel::Nullable<Percent> Instance::GetStateOfCharge() const
{
    return mCluster.Cluster().GetStateOfCharge();
}

CHIP_ERROR Instance::SetStateOfCharge(DataModel::Nullable<Percent> newValue)
{
    return mCluster.Cluster().SetStateOfCharge(newValue);
}

DataModel::Nullable<int64_t> Instance::GetBatteryCapacity() const
{
    return mCluster.Cluster().GetBatteryCapacity();
}

CHIP_ERROR Instance::SetBatteryCapacity(DataModel::Nullable<int64_t> newValue)
{
    return mCluster.Cluster().SetBatteryCapacity(newValue);
}

DataModel::Nullable<CharSpan> Instance::GetVehicleID() const
{
    return mCluster.Cluster().GetVehicleID();
}

CHIP_ERROR Instance::SetVehicleID(DataModel::Nullable<CharSpan> newValue)
{
    return mCluster.Cluster().SetVehicleID(newValue);
}

DataModel::Nullable<uint32_t> Instance::GetSessionID() const
{
    return mCluster.Cluster().GetSessionID();
}

CHIP_ERROR Instance::SetSessionID(DataModel::Nullable<uint32_t> newValue)
{
    return mCluster.Cluster().SetSessionID(newValue);
}

DataModel::Nullable<uint32_t> Instance::GetSessionDuration() const
{
    return mCluster.Cluster().GetSessionDuration();
}

CHIP_ERROR Instance::SetSessionDuration(DataModel::Nullable<uint32_t> newValue)
{
    return mCluster.Cluster().SetSessionDuration(newValue);
}

DataModel::Nullable<int64_t> Instance::GetSessionEnergyCharged() const
{
    return mCluster.Cluster().GetSessionEnergyCharged();
}

CHIP_ERROR Instance::SetSessionEnergyCharged(DataModel::Nullable<int64_t> newValue)
{
    return mCluster.Cluster().SetSessionEnergyCharged(newValue);
}

DataModel::Nullable<int64_t> Instance::GetSessionEnergyDischarged() const
{
    return mCluster.Cluster().GetSessionEnergyDischarged();
}

CHIP_ERROR Instance::SetSessionEnergyDischarged(DataModel::Nullable<int64_t> newValue)
{
    return mCluster.Cluster().SetSessionEnergyDischarged(newValue);
}

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip

// The current implementation already manually instantiates and initializes the cluster, so no need for the codegen integration.
void MatterEnergyEvseClusterInitCallback(chip::EndpointId) {}
void MatterEnergyEvseClusterShutdownCallback(chip::EndpointId, MatterClusterShutdownType) {}

// Legacy callback stubs
void MatterEnergyEvsePluginServerInitCallback() {}
void MatterEnergyEvsePluginServerShutdownCallback() {}
