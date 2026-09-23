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

#include <clusters/OnOff/Enums.h>
#include <devices/Types.h>

using namespace chip::app::Clusters;

namespace chip::app {

HumidityConditioner::HumidityConditioner(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kHumidityConditioner, 1)),
    mTimerDelegate(config.timerDelegate), mIdentifyDelegate(config.identifyDelegate), mOnOffDelegate(config.onOffDelegate),
    mHumidistatDelegate(config.humidistatDelegate)
{}

CHIP_ERROR HumidityConditioner::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                         EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Identify
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate).WithDelegate(&mIdentifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // On/Off: Dead Front Behavior is mandatory, Lighting is disallowed for this device type.
    mOnOffCluster.Create(endpoint,
                         OnOffCluster::Context{
                             .timerDelegate = mTimerDelegate,
                             .featureMap    = BitMask<OnOff::Feature>(OnOff::Feature::kDeadFrontBehavior),
                         });
    mOnOffCluster.Cluster().AddDelegate(&mOnOffDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    // Humidistat: matches the all-clusters-app zap defaults (all features enabled).
    HumidistatCluster::StartupConfiguration humidistatConfig;
    humidistatConfig.mode           = Humidistat::ModeEnum::kHumidifier;
    humidistatConfig.systemState    = Humidistat::SystemStateEnum::kIdle;
    humidistatConfig.userSetpoint   = 40;
    humidistatConfig.minSetpoint    = 30;
    humidistatConfig.maxSetpoint    = 60;
    humidistatConfig.step           = 5;
    humidistatConfig.targetSetpoint = 40;
    humidistatConfig.mistType       = BitMask<Humidistat::MistTypeBitmap>(Humidistat::MistTypeBitmap::kMistCold);
    mHumidistatCluster.Create(endpoint,
                              BitFlags<Humidistat::Feature>(Humidistat::Feature::kHumidifier, Humidistat::Feature::kDehumidifier,
                                                            Humidistat::Feature::kContinuous, Humidistat::Feature::kSensor,
                                                            Humidistat::Feature::kAuto, Humidistat::Feature::kFanOnly,
                                                            Humidistat::Feature::kOptimal, Humidistat::Feature::kWarmMist,
                                                            Humidistat::Feature::kColdMist, Humidistat::Feature::kCondPump),
                              HumidistatCluster::OptionalAttributeSet()
                                  .Set<Humidistat::Attributes::Sleep::Id>()
                                  .Set<Humidistat::Attributes::TargetSetpoint::Id>(),
                              humidistatConfig);
    mHumidistatCluster.Cluster().SetDelegate(&mHumidistatDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mHumidistatCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void HumidityConditioner::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mHumidistatCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mHumidistatCluster.Cluster()));
        mHumidistatCluster.Destroy();
    }

    if (mOnOffCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        mOnOffCluster.Destroy();
    }

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace chip::app
