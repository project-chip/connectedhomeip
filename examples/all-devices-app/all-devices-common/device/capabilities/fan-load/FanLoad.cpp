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

#include <clusters/FanControl/Enums.h>
#include <device/capabilities/fan-load/FanLoad.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;
using chip::BitMask;

namespace chip {
namespace app {

FanLoad::FanLoad(Span<const DataModel::DeviceTypeEntry> deviceTypes, Clusters::FanControl::Delegate & fanDelegate,
                 Clusters::OnOffDelegate * onOffDelegate, const Context & context) :
    SingleEndpoint(deviceTypes),
    mFanDelegate(fanDelegate), mOnOffDelegate(onOffDelegate), mTimerDelegate(context.timerDelegate), mContext(context)
{
    VerifyOrDie(mContext.includeOnOffCluster == (mOnOffDelegate != nullptr));
}

CHIP_ERROR FanLoad::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(
        RegisterDescriptor(endpoint, provider, EndpointComposition(composition.parentId, composition.pattern, mContext.tagList)));

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
    if (mOnOffDelegate != nullptr)
    {
        OnOffCluster::Context onOffContext{ mTimerDelegate };

        mOnOffCluster.Create(endpoint, onOffContext);
        mOnOffCluster.Cluster().AddDelegate(mOnOffDelegate);
        ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

        {
            ScopedSceneTable table(mScenesTableProvider);
            table->RegisterHandler(&mOnOffCluster.Cluster());
        }
    }

    // Fan
    FanControlCluster::Config fanConfig(endpoint, mFanDelegate);
    fanConfig.WithSpeedMax(10)
        .WithStep()
        .WithWindSupport(BitMask<FanControl::WindBitmap>(FanControl::WindBitmap::kSleepWind, FanControl::WindBitmap::kNaturalWind))
        .WithRockSupport(BitMask<FanControl::RockBitmap>(FanControl::RockBitmap::kRockLeftRight))
        .WithAirflowDirection();
    mFanControlCluster.Create(fanConfig);
    ReturnErrorOnFailure(provider.AddCluster(mFanControlCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void FanLoad::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mFanControlCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mFanControlCluster.Cluster()));
        mFanControlCluster.Destroy();
    }

    if (mOnOffCluster.IsConstructed())
    {
        if (mOnOffCluster.Cluster().IsInList())
        {
            ScopedSceneTable table(mScenesTableProvider);
            table->UnregisterHandler(&mOnOffCluster.Cluster());
        }

        mOnOffCluster.Cluster().RemoveDelegate(mOnOffDelegate);
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

} // namespace app
} // namespace chip
