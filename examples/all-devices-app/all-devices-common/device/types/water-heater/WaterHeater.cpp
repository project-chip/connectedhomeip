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
#include <device/types/water-heater/WaterHeater.h>
#include <clusters/WaterHeaterManagement/AttributeIds.h>
#include <clusters/WaterHeaterManagement/ClusterId.h>
#include <clusters/Thermostat/ClusterId.h>
#include <clusters/Thermostat/AttributeIds.h>
#include <app/clusters/thermostat-server/ThermostatClusterBase.h>
#include <devices/Types.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;
using namespace chip::app::Clusters::WaterHeaterManagement::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip::app {

WaterHeater::WaterHeater(TimerDelegate & timerDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterHeater, 1)), mTimerDelegate(timerDelegate) {}

WaterHeater::~WaterHeater()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR WaterHeater::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(SingleEndpoint::mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    mProvider = &provider;
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mWaterHeaterManagementCluster.Create(endpoint, *this, static_cast<Feature>(0));
    ReturnErrorOnFailure(provider.AddCluster(mWaterHeaterManagementCluster.Registration()));

    mThermostatDelegate = std::make_unique<Thermostat::ThermostatDelegate>(endpoint);
    mThermostatSetpointsDelegate = std::make_unique<Thermostat::ThermostatSetpointsDelegate>(endpoint);
    mThermostatCluster.Create(endpoint, static_cast<Thermostat::Feature>(0), 
        Thermostat::ThermostatClusterBase::Config(Thermostat::OptionalAttributes(), mTimerDelegate),
        *mThermostatDelegate, *mThermostatSetpointsDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mThermostatCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void WaterHeater::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    mProvider = nullptr;
    UnregisterDescriptor(provider);
    if (mWaterHeaterManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWaterHeaterManagementCluster.Cluster()));
        mWaterHeaterManagementCluster.Destroy();
    }
}

Clusters::WaterHeaterManagement::WaterHeaterManagementCluster & WaterHeater::WaterHeaterManagementCluster()
{
    VerifyOrDie(mWaterHeaterManagementCluster.IsConstructed());
    return mWaterHeaterManagementCluster.Cluster();
}

void WaterHeater::TimerFired()
{
    ChipLogProgress(AppServer, "WaterHeater: Boost duration elapsed");
    EndBoost();
}

Status WaterHeater::HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                Optional<Percent> targetReheat)
{
    ChipLogProgress(AppServer, "WaterHeater: Boost duration=%" PRIu32 "s", duration);

    mTimerDelegate.CancelTimer(this);

    mBoostState = BoostStateEnum::kActive;
    mHeatDemand = mHeaterTypes;

    CHIP_ERROR err =
        GenerateBoostStartedEvent(duration, oneShot, emergencyBoost, temporarySetpoint, targetPercentage, targetReheat);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "WaterHeater: Failed to generate BoostStarted event: %" CHIP_ERROR_FORMAT, err.Format());
    }

    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Seconds32(duration)));
    NotifyHeatDemandAndBoostStateChanged();
    return Status::Success;
}

Status WaterHeater::HandleCancelBoost()
{
    if (mBoostState == BoostStateEnum::kInactive)
    {
        return Status::Success;
    }

    ChipLogProgress(AppServer, "WaterHeater: CancelBoost");
    EndBoost();
    return Status::Success;
}

BitMask<WaterHeaterHeatSourceBitmap> WaterHeater::GetHeaterTypes()
{
    return mHeaterTypes;
}

BitMask<WaterHeaterHeatSourceBitmap> WaterHeater::GetHeatDemand()
{
    return mHeatDemand;
}

uint16_t WaterHeater::GetTankVolume()
{
    return 0;
}

Energy_mWh WaterHeater::GetEstimatedHeatRequired()
{
    return 0;
}

Percent WaterHeater::GetTankPercentage()
{
    return 0;
}

BoostStateEnum WaterHeater::GetBoostState()
{
    return mBoostState;
}

void WaterHeater::EndBoost()
{
    mTimerDelegate.CancelTimer(this);
    mBoostState = BoostStateEnum::kInactive;
    mHeatDemand.ClearAll();

    CHIP_ERROR err = GenerateBoostEndedEvent();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "WaterHeater: Failed to generate BoostEnded event: %" CHIP_ERROR_FORMAT, err.Format());
    }

    NotifyHeatDemandAndBoostStateChanged();
}

void WaterHeater::NotifyHeatDemandAndBoostStateChanged()
{
    VerifyOrReturn(mProvider != nullptr);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), Id, HeatDemand::Id },
                                      DataModel::AttributeChangeType::kReportable);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), Id, BoostState::Id },
                                      DataModel::AttributeChangeType::kReportable);
}

} // namespace chip::app
