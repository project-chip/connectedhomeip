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

#include "SmokeCoAlarm.h"
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;

namespace chip {
namespace app {

SmokeCoAlarm::SmokeCoAlarm(TimerDelegate & timerDelegate, Clusters::SmokeCoAlarmDelegate & smokeCoAlarmDelegate,
                           const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kSmokeCoAlarm, 1)),
    mTimerDelegate(timerDelegate), mSmokeCoAlarmDelegate(smokeCoAlarmDelegate), mConfig(config)
{}

CHIP_ERROR SmokeCoAlarm::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                  EndpointComposition composition)
{
    if (mConfig.coConcentrationConfig.has_value())
    {
        VerifyOrReturnError(mConfig.coConcentrationConfig->clusterId == CarbonMonoxideConcentrationMeasurement::Id,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (mConfig.smokeConcentrationConfig.has_value())
    {
        VerifyOrReturnError(mConfig.smokeConcentrationConfig->clusterId == SmokeConcentrationMeasurement::Id,
                            CHIP_ERROR_INVALID_ARGUMENT);
    }

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mSmokeCoAlarmCluster.Create(endpoint, mConfig.alarmConfig);
    mSmokeCoAlarmCluster.Cluster().SetDelegate(&mSmokeCoAlarmDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mSmokeCoAlarmCluster.Registration()));

    if (mConfig.coConcentrationConfig.has_value())
    {
        mCoMeasurementCluster.Create(endpoint, *mConfig.coConcentrationConfig);
        ReturnErrorOnFailure(provider.AddCluster(mCoMeasurementCluster.Registration()));
    }

    if (mConfig.smokeConcentrationConfig.has_value())
    {
        mSmokeConcentrationCluster.Create(endpoint, *mConfig.smokeConcentrationConfig);
        ReturnErrorOnFailure(provider.AddCluster(mSmokeConcentrationCluster.Registration()));
    }

    return provider.AddEndpoint(mEndpointRegistration);
}

void SmokeCoAlarm::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mSmokeConcentrationCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mSmokeConcentrationCluster.Cluster()));
        mSmokeConcentrationCluster.Destroy();
    }
    if (mCoMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mCoMeasurementCluster.Cluster()));
        mCoMeasurementCluster.Destroy();
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

SmokeCoAlarm::ConcentrationCluster & SmokeCoAlarm::GetCoConcentrationCluster()
{
    VerifyOrDie(mCoMeasurementCluster.IsConstructed());
    return mCoMeasurementCluster.Cluster();
}

SmokeCoAlarm::ConcentrationCluster & SmokeCoAlarm::GetSmokeConcentrationCluster()
{
    VerifyOrDie(mSmokeConcentrationCluster.IsConstructed());
    return mSmokeConcentrationCluster.Cluster();
}
Clusters::SmokeCoAlarmCluster & SmokeCoAlarm::GetSmokeCoAlarmCluster()
{
    VerifyOrDie(mSmokeCoAlarmCluster.IsConstructed());
    return mSmokeCoAlarmCluster.Cluster();
}
Clusters::IdentifyCluster & SmokeCoAlarm::GetIdentifyCluster()
{
    VerifyOrDie(mIdentifyCluster.IsConstructed());
    return mIdentifyCluster.Cluster();
}

} // namespace app
} // namespace chip
