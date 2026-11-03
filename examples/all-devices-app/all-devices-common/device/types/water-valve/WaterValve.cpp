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
#include <device/types/water-valve/WaterValve.h>

#include <app/data-model/Nullable.h>
#include <devices/Types.h>

using namespace chip::app::Clusters;

namespace chip::app {

WaterValve::WaterValve(TimerDelegate & timerDelegate, Clusters::ValveConfigurationAndControl::Delegate & delegate,
                       const WaterValveConfig & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterValve, 1)),
    mTimerDelegate(timerDelegate), mValveDelegate(delegate), mConfig(config)
{}
CHIP_ERROR WaterValve::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mValveCluster.Create(endpoint,
                         ValveConfigurationAndControlCluster::ValveContext{ .features             = mConfig.features,
                                                                            .optionalAttributeSet = mConfig.optionalAttributes,
                                                                            .config               = mConfig.startup,
                                                                            .tsTracker            = mConfig.tsTracker,
                                                                            .delegate             = &mValveDelegate });

    ReturnErrorOnFailure(provider.AddCluster(mValveCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void WaterValve::Unregister(CodeDrivenDataModelProvider & provider)
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

Clusters::ValveConfigurationAndControlCluster & WaterValve::ValveConfigurationAndControlCluster()
{
    VerifyOrDie(mValveCluster.IsConstructed());
    return mValveCluster.Cluster();
}

} // namespace chip::app
