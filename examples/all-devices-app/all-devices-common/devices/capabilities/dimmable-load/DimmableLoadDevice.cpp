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
#include <devices/capabilities/dimmable-load/DimmableLoadDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

DimmableLoadDevice::DimmableLoadDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context,
                                       const Delegates & delegates, const Config & config) :
    SingleEndpointDevice(deviceTypes),
    mOnOffDelegate(delegates.onOff), mLevelControlDelegate(delegates.levelControl), mEffectDelegate(delegates.effect),
    mIdentifyDelegate(delegates.identify), mContext(context), mConfig(config)
{}

CHIP_ERROR DimmableLoadDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                        EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mContext.timerDelegate).WithDelegate(&mIdentifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

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

    mOnOffCluster.Create(endpoint,
                         OnOffLightingCluster::Context{
                             .timerDelegate             = mContext.timerDelegate,
                             .effectDelegate            = mEffectDelegate,
                             .scenesIntegrationDelegate = &mScenesManagementCluster.Cluster(),
                         });
    mOnOffCluster.Cluster().AddDelegate(&mOnOffDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    LevelControlCluster::Config lvlConfig(mContext.timerDelegate, mLevelControlDelegate);
    lvlConfig.WithOnOff(mOnOffCluster.Cluster());

    if (mConfig.levelControl.startUpCurrentLevel.has_value())
    {
        lvlConfig.WithLighting(*mConfig.levelControl.startUpCurrentLevel);
    }
    if (mConfig.levelControl.onOffTransitionTime.has_value())
    {
        lvlConfig.WithOnOffTransitionTime(*mConfig.levelControl.onOffTransitionTime);
    }
    if (mConfig.levelControl.onTransitionTime.has_value())
    {
        lvlConfig.WithOnTransitionTime(*mConfig.levelControl.onTransitionTime);
    }
    if (mConfig.levelControl.offTransitionTime.has_value())
    {
        lvlConfig.WithOffTransitionTime(*mConfig.levelControl.offTransitionTime);
    }
    if (mConfig.levelControl.defaultMoveRate.has_value())
    {
        lvlConfig.WithDefaultMoveRate(*mConfig.levelControl.defaultMoveRate);
    }
    if (mConfig.levelControl.minLevel.has_value())
    {
        lvlConfig.WithMinLevel(*mConfig.levelControl.minLevel);
    }
    if (mConfig.levelControl.maxLevel.has_value())
    {
        lvlConfig.WithMaxLevel(*mConfig.levelControl.maxLevel);
    }

    mLevelControlCluster.Create(endpoint, lvlConfig);
    mOnOffCluster.Cluster().AddDelegate(&mLevelControlCluster.Cluster());
    ReturnErrorOnFailure(provider.AddCluster(mLevelControlCluster.Registration()));

    mGroupsCluster.Create(endpoint,
                          GroupsCluster::Context{
                              .groupDataProvider   = mContext.groupDataProvider,
                              .scenesIntegration   = &mScenesManagementCluster.Cluster(),
                              .identifyIntegration = &mIdentifyCluster.Cluster(),
                          });
    ReturnErrorOnFailure(provider.AddCluster(mGroupsCluster.Registration()));

    // We have scenes enabled, so make sure handlers are registered so we can
    // save and recall scenes.
    {
        Clusters::ScopedSceneTable table(mScenesTableProvider);
        table->RegisterHandler(&mOnOffCluster.Cluster());
        table->RegisterHandler(&mLevelControlCluster.Cluster());
    }

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void DimmableLoadDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    // IMPORTANT: We must strictly separate DISCONNECTION (Phase 1) from DESTRUCTION (Phase 2).
    //
    // Phase 1 (Disconnection): Remove all delegates and unregister from all shared lists/tables
    // while all cluster objects are still fully constructed and valid. This prevents any
    // use-after-free or dangling pointer access (e.g. OnOff-Level coupling delegates) during destruction.
    //
    // Phase 2 (Destruction): Once fully disconnected, the cluster objects can be safely destroyed
    // in any order.
    //
    // === PHASE 1: DISCONNECTION ===

    // Unregister the endpoint itself from the provider first so we are allowed to remove its clusters.
    UnregisterDescriptor(provider);

    // 1. Unregister all handlers from the Scenes Table
    if (mOnOffCluster.IsConstructed() && mOnOffCluster.Cluster().IsInList())
    {
        Clusters::ScopedSceneTable table(mScenesTableProvider);
        table->UnregisterHandler(&mOnOffCluster.Cluster());
        if (mLevelControlCluster.IsConstructed())
        {
            table->UnregisterHandler(&mLevelControlCluster.Cluster());
        }
    }

    // 2. Remove all delegates from OnOff cluster
    if (mOnOffCluster.IsConstructed())
    {
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        if (mLevelControlCluster.IsConstructed())
        {
            mOnOffCluster.Cluster().RemoveDelegate(&mLevelControlCluster.Cluster());
        }
    }

    // === PHASE 2: DESTRUCTION ===

    if (mGroupsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGroupsCluster.Cluster()));
        mGroupsCluster.Destroy();
    }

    if (mLevelControlCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mLevelControlCluster.Cluster()));
        mLevelControlCluster.Destroy();
    }

    if (mOnOffCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Destroy();
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
