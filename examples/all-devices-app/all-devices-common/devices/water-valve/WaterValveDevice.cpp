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
#include <devices/Types.h>
#include <devices/water-valve/WaterValveDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

WaterValveDevice::WaterValveDevice(TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterValve, 1)), mTimerDelegate(timerDelegate)
{}

CHIP_ERROR WaterValveDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                      EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    ValveConfigurationAndControlCluster::StartupConfiguration config{ DataModel::NullNullable,
                                                                      ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                                                      ValveConfigurationAndControlCluster::kDefaultLevelStep };

    ValveConfigurationAndControlCluster::ValveContext valveContext = {
        .features             = BitFlags<ValveConfigurationAndControl::Feature>(ValveConfigurationAndControl::Feature::kLevel),
        .optionalAttributeSet = {},
        .config               = config,
        .tsTracker            = nullptr,
        .delegate             = this,
    };

    mValveCluster.Create(endpoint, valveContext);
    ReturnErrorOnFailure(provider.AddCluster(mValveCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void WaterValveDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mValveCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mValveCluster.Cluster()));
        mValveCluster.Destroy();
    }

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

Clusters::ValveConfigurationAndControlCluster & WaterValveDevice::ValveConfigurationAndControlCluster()
{
    VerifyOrDie(mValveCluster.IsConstructed());
    return mValveCluster.Cluster();
}

DataModel::Nullable<Percent> WaterValveDevice::HandleOpenValve(DataModel::Nullable<Percent> level)
{
    Percent targetLevel = level.ValueOr(100);
    ChipLogProgress(AppServer, "WaterValveDevice: Opening valve to level %u", targetLevel);
    return DataModel::MakeNullable(targetLevel);
}

CHIP_ERROR WaterValveDevice::HandleCloseValve()
{
    ChipLogProgress(AppServer, "WaterValveDevice: Closing valve");
    if (mValveCluster.IsConstructed())
    {
        mValveCluster.Cluster().UpdateCurrentLevel(0);
    }
    return CHIP_NO_ERROR;
}

void WaterValveDevice::HandleRemainingDurationTick(uint32_t duration)
{
    ChipLogProgress(AppServer, "WaterValveDevice: Remaining duration tick: %lu", static_cast<unsigned long>(duration));
}

} // namespace chip::app
