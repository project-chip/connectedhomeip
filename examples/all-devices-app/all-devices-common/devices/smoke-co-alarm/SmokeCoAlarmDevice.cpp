/*
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

#include <devices/Types.h>
#include "SmokeCoAlarmDevice.h"
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

SmokeCoAlarmDevice::SmokeCoAlarmDevice(TimerDelegate & timerDelegate, const ConcentrationCluster::Config & smokeConfig) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kSmokeCoAlarm, 1)),
    mTimerDelegate(timerDelegate), mSmokeConfig(smokeConfig)
{}

CHIP_ERROR SmokeCoAlarmDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mSmokeCoAlarmCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mSmokeCoAlarmCluster.Registration()));

    mSmokeMeasurementCluster.Create(endpoint, mSmokeConfig);
    ReturnErrorOnFailure(provider.AddCluster(mSmokeMeasurementCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void SmokeCoAlarmDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mSmokeMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mSmokeMeasurementCluster.Cluster()));
        mSmokeMeasurementCluster.Destroy();
    }
    if (mSmokeCoAlarmCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mSmokeCoAlarmCluster.Cluster()));
        mSmokeCoAlarmCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

SmokeCoAlarmDevice::ConcentrationCluster & SmokeCoAlarmDevice::GetSmokeConcentCluster()
{
    VerifyOrDie(mSmokeMeasurementCluster.IsConstructed());
    return mSmokeMeasurementCluster.Cluster();
}
Clusters::SmokeCoAlarmCluster & SmokeCoAlarmDevice::GetSmokeCoAlarmCluster()
{
    VerifyOrDie(mSmokeCoAlarmCluster.IsConstructed());
    return mSmokeCoAlarmCluster.Cluster();
}
    Clusters::IdentifyCluster &  SmokeCoAlarmDevice::GetIdentifyCluster()
    {
        VerifyOrDie(mIdentifyCluster.IsConstructed());
        return mIdentifyCluster.Cluster();
    }

} // namespace app
} // namespace chip
