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
#include "IncreasingTemperatureSensor.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

constexpr System::Clock::Seconds16 kIncreaseTemperatureIntervalSec = System::Clock::Seconds16(10);

} // namespace

IncreasingTemperatureSensor::IncreasingTemperatureSensor(TimerDelegate & timerDelegate,
                                                         TemperatureMeasurementCluster::StartupConfiguration tempConfig,
                                                         TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributes) :
    TemperatureSensor(timerDelegate, tempConfig, optionalAttributes)
{}

IncreasingTemperatureSensor::~IncreasingTemperatureSensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR IncreasingTemperatureSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                 EndpointComposition composition)
{
    ReturnErrorOnFailure(TemperatureSensor::Register(endpoint, provider, composition));
    // Kick off the timer loop to increase temperature every few seconds
    return mTimerDelegate.StartTimer(this, kIncreaseTemperatureIntervalSec);
}

void IncreasingTemperatureSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    TemperatureSensor::Unregister(provider);
}

CHIP_ERROR IncreasingTemperatureSensor::RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    ThermostatUserInterfaceConfigurationCluster::Config config;
    config.optionalAttributes.Set<ThermostatUserInterfaceConfiguration::Attributes::ScheduleProgrammingVisibility::Id>();
    mUserInterfaceCluster.Create(endpoint, config);
    mUserInterfaceCluster.Cluster().SetDelegate(this);
    return provider.AddCluster(mUserInterfaceCluster.Registration());
}

void IncreasingTemperatureSensor::UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mUserInterfaceCluster.IsConstructed())
    {
        mUserInterfaceCluster.Cluster().SetDelegate(nullptr);
        LogErrorOnFailure(provider.RemoveCluster(&mUserInterfaceCluster.Cluster()));
        mUserInterfaceCluster.Destroy();
    }
}

void IncreasingTemperatureSensor::TimerFired()
{
    const int16_t minTemp =
        mTempConfig.minMeasuredValue.ValueOr(!mTempConfig.maxMeasuredValue.IsNull() && mTempConfig.maxMeasuredValue.Value() < -1000
                                                 ? mTempConfig.maxMeasuredValue.Value()
                                                 : -1000);
    const int16_t maxTemp = mTempConfig.maxMeasuredValue.ValueOr(INT16_MAX);

    // Simulate increasing temperature
    if (mTemperatureMeasuredValue.IsNull() || mTemperatureMeasuredValue.Value() >= maxTemp)
    {
        mTemperatureMeasuredValue.SetNonNull(minTemp);
    }
    else
    {
        mTemperatureMeasuredValue.SetNonNull(static_cast<int16_t>(mTemperatureMeasuredValue.Value() + 1));
    }

    ChipLogProgress(AppServer, "IncreasingTemperatureValue: Increasing to %d", mTemperatureMeasuredValue.Value());
    LogErrorOnFailure(mTemperatureMeasurementCluster.Cluster().SetMeasuredValue(mTemperatureMeasuredValue));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kIncreaseTemperatureIntervalSec));
}

void IncreasingTemperatureSensor::OnTemperatureDisplayModeChanged(
    ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value)
{
    ChipLogProgress(AppServer, "TemperatureSensor: TemperatureDisplayMode changed to %u", static_cast<unsigned>(value));
}

void IncreasingTemperatureSensor::OnKeypadLockoutChanged(ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value)
{
    ChipLogProgress(AppServer, "TemperatureSensor: KeypadLockout changed to %u", static_cast<unsigned>(value));
}

} // namespace app
} // namespace chip
