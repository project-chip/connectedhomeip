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
#include <devices/fan/FanDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

FanDevice::FanDevice(Clusters::FanControl::Delegate & fanDelegate, Clusters::OnOffDelegate & onOffDelegate,
                     const Context & context) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kFan, 1)),
    mFanDelegate(fanDelegate), mOnOffDelegate(onOffDelegate), mTimerDelegate(context.timerDelegate), mContext(context)
{}

CHIP_ERROR FanDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    // Identify
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // ScenesManagement
    mScenesTableProvider.SetEndpoint(endpoint);
    mScenesManagementCluster.Create(endpoint,
                                    ScenesManagementCluster::Context{
                                        .groupDataProvider  = &mContext.groupDataProvider,
                                        .fabricTable        = &mContext.fabricTable,
                                        .features           = {},
                                        .sceneTableProvider = mScenesTableProvider,
                                        .supportsCopyScene  = true,
                                    });
    ReturnErrorOnFailure(provider.AddCluster(mScenesManagementCluster.Registration()));

    // Groups
    mGroupsCluster.Create(endpoint,
                          GroupsCluster::Context{
                              .groupDataProvider   = mContext.groupDataProvider,
                              .scenesIntegration   = &mScenesManagementCluster.Cluster(),
                              .identifyIntegration = &mIdentifyCluster.Cluster(),
                          });
    ReturnErrorOnFailure(provider.AddCluster(mGroupsCluster.Registration()));

    // OnOff
    OnOffCluster::Context onOffContext{ mTimerDelegate };

    mOnOffCluster.Create(endpoint, onOffContext);
    mOnOffCluster.Cluster().AddDelegate(&mOnOffDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    // Fan
    FanControlCluster::Config fanConfig(endpoint, &mFanDelegate);
    fanConfig.WithSpeedMax(10).WithStep();
    mFanControlCluster.Create(fanConfig);
    ReturnErrorOnFailure(provider.AddCluster(mFanControlCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void FanDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);

    if (mFanControlCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mFanControlCluster.Cluster()));
        mFanControlCluster.Destroy();
    }

    if (mOnOffCluster.IsConstructed())
    {
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Destroy();
    }

    if (mGroupsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGroupsCluster.Cluster()));
        mGroupsCluster.Destroy();
    }

    if (mScenesManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mScenesManagementCluster.Cluster()));
        mScenesManagementCluster.Destroy();
    }

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

Clusters::OnOffCluster & FanDevice::OnOffCluster()
{
    VerifyOrDie(mOnOffCluster.IsConstructed());
    return mOnOffCluster.Cluster();
}

Clusters::FanControlCluster & FanDevice::FanControlCluster()
{
    VerifyOrDie(mFanControlCluster.IsConstructed());
    return mFanControlCluster.Cluster();
}

Clusters::GroupsCluster & FanDevice::GroupsCluster()
{
    VerifyOrDie(mGroupsCluster.IsConstructed());
    return mGroupsCluster.Cluster();
}

} // namespace app
} // namespace chip
