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

#include <device/types/energy-evse/impl/simulation/AllDevicesEvseChargingSchedule.h>

#include <EvseChargingSchedule.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

namespace {

using namespace Clusters;
using namespace Clusters::EnergyEvse;
using Protocols::InteractionModel::Status;

class AllDevicesEvseChargingScheduleContext : public EvseChargingScheduleContext
{
public:
    explicit AllDevicesEvseChargingScheduleContext(EvseSimulationDelegate & delegate) : mDelegate(delegate) {}

    const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & GetChargingTargets() const override
    {
        return mDelegate.GetEvseTargetsDelegate()->GetTargets();
    }

    bool IsEvsePluggedIn() const override { return mDelegate.IsEvsePluggedIn(); }

    SupplyStateEnum GetSupplyState() const override { return mDelegate.GetSupplyState(); }

    bool HasSoCReportingFeature() const override
    {
        Clusters::EnergyEvse::EnergyEvseCluster * cluster = mDelegate.GetCluster();
        VerifyOrReturnError(cluster != nullptr, false);
        return cluster->Features().Has(Feature::kSoCReporting);
    }

    DataModel::Nullable<Percent> GetStateOfCharge() const override { return mDelegate.GetStateOfCharge(); }

    DataModel::Nullable<int64_t> GetBatteryCapacity() const override { return mDelegate.GetBatteryCapacity(); }

    int64_t HwGetNominalMainsVoltage() const override { return mDelegate.HwGetNominalMainsVoltage(); }

    int64_t GetMaximumChargeCurrent() const override { return mDelegate.GetMaximumChargeCurrent(); }

    CHIP_ERROR SetNextChargeStartTime(DataModel::Nullable<uint32_t> value) override
    {
        Clusters::EnergyEvse::EnergyEvseCluster * cluster = mDelegate.GetCluster();
        VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return cluster->SetNextChargeStartTime(value);
    }

    CHIP_ERROR SetNextChargeTargetTime(DataModel::Nullable<uint32_t> value) override
    {
        Clusters::EnergyEvse::EnergyEvseCluster * cluster = mDelegate.GetCluster();
        VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return cluster->SetNextChargeTargetTime(value);
    }

    CHIP_ERROR SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> value) override
    {
        Clusters::EnergyEvse::EnergyEvseCluster * cluster = mDelegate.GetCluster();
        VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return cluster->SetNextChargeRequiredEnergy(value);
    }

    CHIP_ERROR SetNextChargeTargetSoC(DataModel::Nullable<Percent> value) override
    {
        Clusters::EnergyEvse::EnergyEvseCluster * cluster = mDelegate.GetCluster();
        VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return cluster->SetNextChargeTargetSoC(value);
    }

private:
    EvseSimulationDelegate & mDelegate;
};

CHIP_ERROR ApplyDefaultEvseHardwareLimits(EvseSimulationDelegate & delegate)
{
    VerifyOrReturnError(delegate.HwSetNominalMainsVoltage(230000) == Status::Success, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(delegate.HwSetMaxHardwareChargeCurrentLimit(32000) == Status::Success, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(delegate.HwSetMaxHardwareDischargeCurrentLimit(32000) == Status::Success, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(delegate.HwSetCircuitCapacity(20000) == Status::Success, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

} // namespace

void AllDevicesEvseChargingScheduleCallbackHandler(const EVSECbInfo * cb, intptr_t arg)
{
    auto * delegate = reinterpret_cast<EvseSimulationDelegate *>(arg);
    VerifyOrReturn(delegate != nullptr);

    switch (cb->type)
    {
    case EVSECallbackType::StateChanged:
        ChipLogProgress(AppServer, "EVSE callback - state changed");
        {
            AllDevicesEvseChargingScheduleContext context(*delegate);
            LogErrorOnFailure(ComputeEvseChargingSchedule(context));
        }
        break;
    case EVSECallbackType::ChargeCurrentChanged:
        ChipLogProgress(AppServer, "EVSE callback - maxChargeCurrent changed to %ld",
                        static_cast<long>(cb->ChargingCurrent.maximumChargeCurrent));
        {
            AllDevicesEvseChargingScheduleContext context(*delegate);
            LogErrorOnFailure(ComputeEvseChargingSchedule(context));
        }
        break;
    case EVSECallbackType::ChargingPreferencesChanged:
        ChipLogProgress(AppServer, "EVSE callback - ChargingPreferencesChanged");
        {
            AllDevicesEvseChargingScheduleContext context(*delegate);
            LogErrorOnFailure(ComputeEvseChargingSchedule(context));
        }
        break;
    default:
        break;
    }
}

CHIP_ERROR RegisterAllDevicesEvseChargingSchedule(EvseSimulationDelegate & delegate)
{
    ReturnErrorOnFailure(ApplyDefaultEvseHardwareLimits(delegate));
    VerifyOrReturnError(delegate.HwRegisterEvseCallbackHandler(AllDevicesEvseChargingScheduleCallbackHandler,
                                                               reinterpret_cast<intptr_t>(&delegate)) == Status::Success,
                        CHIP_ERROR_INTERNAL);
    AllDevicesEvseChargingScheduleContext context(delegate);
    return ComputeEvseChargingSchedule(context);
}

void UnregisterAllDevicesEvseChargingSchedule(EvseSimulationDelegate & delegate)
{
    delegate.ClearEvseCallbackHandler();
}

} // namespace app
} // namespace chip
