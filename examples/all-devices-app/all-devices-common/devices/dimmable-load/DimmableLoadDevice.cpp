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
#include <devices/dimmable-load/DimmableLoadDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

DimmableLoadDevice::DimmableLoadDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes,
                                       Clusters::OnOffDelegate & onOffDelegate,
                                       Clusters::LevelControlDelegate & levelControlDelegate,
                                       Clusters::OnOffEffectDelegate & effectDelegate,
                                       Clusters::IdentifyDelegate & identifyDelegate, const Context & context) :
    SingleEndpointDevice(deviceTypes),
    mOnOffDelegate(onOffDelegate), mLevelControlDelegate(levelControlDelegate), mEffectDelegate(effectDelegate),
    mIdentifyDelegate(identifyDelegate), mContext(context)
{}

CHIP_ERROR DimmableLoadDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                         EndpointComposition composition)
{
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

    // Optional LevelControl transition attributes enabled in CI PICS (ci-pics-values).
    // Initialize with demo defaults (0 transition, null move rate) to pass Test_TC_LVL_2_1.
    mLevelControlCluster.Create(LevelControlCluster::Config(endpoint, mContext.timerDelegate, mLevelControlDelegate)
                                    .WithOnOff(mOnOffCluster.Cluster())
                                    .WithLighting(DataModel::NullNullable)
                                    .WithOnOffTransitionTime(0)
                                    .WithOnTransitionTime(0)
                                    .WithOffTransitionTime(0)
                                    .WithDefaultMoveRate(DataModel::NullNullable));
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

    return CHIP_NO_ERROR;
}

void DimmableLoadDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

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
        if (mOnOffCluster.Cluster().IsInList())
        {
            Clusters::ScopedSceneTable table(mScenesTableProvider);
            table->UnregisterHandler(&mOnOffCluster.Cluster());
            table->UnregisterHandler(&mLevelControlCluster.Cluster());
        }

        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        mOnOffCluster.Cluster().RemoveDelegate(&mLevelControlCluster.Cluster());
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
