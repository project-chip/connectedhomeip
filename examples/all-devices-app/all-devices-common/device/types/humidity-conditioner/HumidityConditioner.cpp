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

#include <device/types/humidity-conditioner/HumidityConditioner.h>

#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

HumidityConditioner::HumidityConditioner(TimerDelegate & timerDelegate, BitFlags<Humidistat::Feature> features,
                                         HumidistatCluster::OptionalAttributeSet optionalAttributes,
                                         HumidistatCluster::StartupConfiguration humidistatConfig,
                                         RelativeHumidityMeasurementCluster::Config humidityConfig) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kHumidityConditioner, 1)),
    mTimerDelegate(timerDelegate), mFeatures(features), mOptionalAttributes(optionalAttributes),
    mHumidistatConfig(humidistatConfig), mHumidityConfig(humidityConfig)
{}

CHIP_ERROR HumidityConditioner::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                         EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mHumidistatCluster.Create(endpoint, mFeatures, mOptionalAttributes, mHumidistatConfig);
    ReturnErrorOnFailure(provider.AddCluster(mHumidistatCluster.Registration()));

    mRelativeHumidityMeasurementCluster.Create(endpoint, mHumidityConfig);
    ReturnErrorOnFailure(provider.AddCluster(mRelativeHumidityMeasurementCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void HumidityConditioner::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mRelativeHumidityMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mRelativeHumidityMeasurementCluster.Cluster()));
        mRelativeHumidityMeasurementCluster.Destroy();
    }
    if (mHumidistatCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mHumidistatCluster.Cluster()));
        mHumidistatCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip