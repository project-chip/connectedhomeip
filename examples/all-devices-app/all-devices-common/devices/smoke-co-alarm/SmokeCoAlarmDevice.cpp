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

#include "SmokeCoAlarmDevice.h"
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;

namespace chip {
namespace app {

namespace {

// This is an example device, so it bakes in a representative, spec-valid configuration rather than taking one
// from the factory. The factory only needs to pass in context (the timer delegate).

// CO concentration: numeric + level indication, measured in air as ppm.
SmokeCoAlarmDevice::ConcentrationCluster::Config DefaultCoConfig()
{
    return SmokeCoAlarmDevice::ConcentrationCluster::Config{
        .clusterId = CarbonMonoxideConcentrationMeasurement::Id,
        .features  = BitFlags<ConcentrationMeasurement::Feature>(ConcentrationMeasurement::Feature::kNumericMeasurement,
                                                                ConcentrationMeasurement::Feature::kLevelIndication),
        .medium    = ConcentrationMeasurement::MeasurementMediumEnum::kAir,
        .unit      = ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
    };
}

// Combined smoke + CO alarm exposing every optional attribute, to showcase the cluster's full surface.
SmokeCoAlarmCluster::Config DefaultSmokeConfig()
{
    SmokeCoAlarmCluster::Config config;
    config.featureMap.Set(SmokeCoAlarm::Feature::kSmokeAlarm).Set(SmokeCoAlarm::Feature::kCoAlarm);
    config.optionalAttribs = SmokeCoAlarmCluster::OptionalAttributeSet(SmokeCoAlarmCluster::OptionalAttributeSet::All());
    return config;
}

} // namespace

SmokeCoAlarmDevice::SmokeCoAlarmDevice(TimerDelegate & timerDelegate, Clusters::SmokeCoAlarmDelegate & smokeCoAlarmDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kSmokeCoAlarm, 1)), mTimerDelegate(timerDelegate),
    mSmokeCoAlarmDelegate(smokeCoAlarmDelegate), mCoConfig(DefaultCoConfig()), mSmokeConfig(DefaultSmokeConfig())
{}

CHIP_ERROR SmokeCoAlarmDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mSmokeCoAlarmCluster.Create(endpoint, mSmokeConfig);
    mSmokeCoAlarmCluster.Cluster().SetDelegate(&mSmokeCoAlarmDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mSmokeCoAlarmCluster.Registration()));

    mCoMeasurementCluster.Create(endpoint, mCoConfig);
    ReturnErrorOnFailure(provider.AddCluster(mCoMeasurementCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void SmokeCoAlarmDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
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

SmokeCoAlarmDevice::ConcentrationCluster & SmokeCoAlarmDevice::GetCoConcentrationCluster()
{
    VerifyOrDie(mCoMeasurementCluster.IsConstructed());
    return mCoMeasurementCluster.Cluster();
}
Clusters::SmokeCoAlarmCluster & SmokeCoAlarmDevice::GetSmokeCoAlarmCluster()
{
    VerifyOrDie(mSmokeCoAlarmCluster.IsConstructed());
    return mSmokeCoAlarmCluster.Cluster();
}
Clusters::IdentifyCluster & SmokeCoAlarmDevice::GetIdentifyCluster()
{
    VerifyOrDie(mIdentifyCluster.IsConstructed());
    return mIdentifyCluster.Cluster();
}

} // namespace app
} // namespace chip
