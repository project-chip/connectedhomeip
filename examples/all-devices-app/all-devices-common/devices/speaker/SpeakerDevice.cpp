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
#include <devices/speaker/SpeakerDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

SpeakerDevice::SpeakerDevice(Clusters::LevelControlDelegate & levelDelegate, Clusters::OnOffDelegate & onOffDelegate,
                             TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kSpeaker, 1)),
    mLevelDelegate(levelDelegate), mOnOffDelegate(onOffDelegate), mTimerDelegate(timerDelegate)
{}

CHIP_ERROR SpeakerDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    // Identify
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // OnOff (Mute)
    OnOffCluster::Context onOffContext{ mTimerDelegate };

    mOnOffCluster.Create(endpoint, onOffContext);
    mOnOffCluster.Cluster().AddDelegate(&mOnOffDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    // Level Control (Volume)
    LevelControlCluster::Config lcConfig(endpoint, mTimerDelegate, mLevelDelegate);
    lcConfig.WithOnOff(mOnOffCluster.Cluster());

    // TODO: The following attributes/features are not required for a Speaker device type.
    // Enable them here temporarily to fully test the LevelControl in CI.
    // When we have a proper level light device type these can be removed.
    lcConfig.WithInitialCurrentLevel(1);
    lcConfig.WithOnOffTransitionTime(0);
    lcConfig.WithOnTransitionTime(0);
    lcConfig.WithOffTransitionTime(0);
    lcConfig.WithLighting(DataModel::NullNullable);
    lcConfig.WithDefaultMoveRate(DataModel::NullNullable);

    mLevelControlCluster.Create(lcConfig);
    mOnOffCluster.Cluster().AddDelegate(&mLevelControlCluster.Cluster());
    ReturnErrorOnFailure(provider.AddCluster(mLevelControlCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void SpeakerDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mLevelControlCluster.IsConstructed())
    {
        if (mOnOffCluster.IsConstructed())
        {
            mOnOffCluster.Cluster().RemoveDelegate(&mLevelControlCluster.Cluster());
        }
        LogErrorOnFailure(provider.RemoveCluster(&mLevelControlCluster.Cluster()));
        mLevelControlCluster.Destroy();
    }
    if (mOnOffCluster.IsConstructed())
    {
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

Clusters::OnOffCluster & SpeakerDevice::OnOffCluster()
{
    VerifyOrDie(mOnOffCluster.IsConstructed());
    return mOnOffCluster.Cluster();
}

Clusters::LevelControlCluster & SpeakerDevice::LevelControlCluster()
{
    VerifyOrDie(mLevelControlCluster.IsConstructed());
    return mLevelControlCluster.Cluster();
}

} // namespace app
} // namespace chip
