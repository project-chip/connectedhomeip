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

namespace {

// This is an example device, so it bakes in a representative, spec-valid configuration rather than taking one
// from the factory. The factory only needs to pass in context (the timer delegate).

// CO concentration: numeric + level indication, measured in air as ppm.
SmokeCoAlarm::ConcentrationCluster::Config DefaultCoConfig()
{
    return SmokeCoAlarm::ConcentrationCluster::Config{
        .clusterId = CarbonMonoxideConcentrationMeasurement::Id,
        .features  = BitFlags<ConcentrationMeasurement::Feature>(ConcentrationMeasurement::Feature::kNumericMeasurement,
                                                                ConcentrationMeasurement::Feature::kLevelIndication),
        .medium    = ConcentrationMeasurement::MeasurementMediumEnum::kAir,
        .unit      = ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
    };
}

// Smoke concentration: numeric + level indication, measured in air as percent obscuration per foot.
SmokeCoAlarm::ConcentrationCluster::Config DefaultSmokeConcentrationConfig()
{
    return SmokeCoAlarm::ConcentrationCluster::Config{
        .clusterId = SmokeConcentrationMeasurement::Id,
        .features  = BitFlags<ConcentrationMeasurement::Feature>(ConcentrationMeasurement::Feature::kNumericMeasurement,
                                                                ConcentrationMeasurement::Feature::kLevelIndication),
        .medium    = ConcentrationMeasurement::MeasurementMediumEnum::kAir,
        .unit      = ConcentrationMeasurement::MeasurementUnitEnum::kPcft,
    };
}

// Combined smoke + CO alarm exposing every optional attribute, to showcase the cluster's full surface.
SmokeCoAlarmCluster::Config DefaultSmokeConfig()
{
    SmokeCoAlarmCluster::Config config;
    config.featureMap.Set(Clusters::SmokeCoAlarm::Feature::kSmokeAlarm).Set(Clusters::SmokeCoAlarm::Feature::kCoAlarm);
    config.optionalAttribs = SmokeCoAlarmCluster::OptionalAttributeSet(SmokeCoAlarmCluster::OptionalAttributeSet::All());
    return config;
}

} // namespace

SmokeCoAlarm::SmokeCoAlarm(TimerDelegate & timerDelegate, Clusters::SmokeCoAlarmDelegate & smokeCoAlarmDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kSmokeCoAlarm, 1)), mTimerDelegate(timerDelegate),
    mSmokeCoAlarmDelegate(smokeCoAlarmDelegate), mCoConfig(DefaultCoConfig()),
    mSmokeConcentrationConfig(DefaultSmokeConcentrationConfig()), mSmokeConfig(DefaultSmokeConfig())
{}

CHIP_ERROR SmokeCoAlarm::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                  EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mSmokeCoAlarmCluster.Create(endpoint, mSmokeConfig);
    mSmokeCoAlarmCluster.Cluster().SetDelegate(&mSmokeCoAlarmDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mSmokeCoAlarmCluster.Registration()));

    mCoMeasurementCluster.Create(endpoint, mCoConfig);
    ReturnErrorOnFailure(provider.AddCluster(mCoMeasurementCluster.Registration()));

    mSmokeConcentrationCluster.Create(endpoint, mSmokeConcentrationConfig);
    ReturnErrorOnFailure(provider.AddCluster(mSmokeConcentrationCluster.Registration()));

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
