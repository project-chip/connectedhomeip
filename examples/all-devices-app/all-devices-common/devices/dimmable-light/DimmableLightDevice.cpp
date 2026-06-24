/*
 *
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
#include <devices/dimmable-light/DimmableLightDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

DimmableLightDevice::DimmableLightDevice(Clusters::OnOffDelegate & onOffDelegate,
                                         Clusters::LevelControlDelegate & levelControlDelegate,
                                         Clusters::OnOffEffectDelegate & effectDelegate,
                                         Clusters::IdentifyDelegate & identifyDelegate, const Context & context) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kDimmableLight, 1)),
    mOnOffDelegate(onOffDelegate), mLevelControlDelegate(levelControlDelegate), mEffectDelegate(effectDelegate),
    mIdentifyDelegate(identifyDelegate), mContext(context)
{}

CHIP_ERROR DimmableLightDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

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

    LevelControlCluster::Config lcConfig(endpoint, mContext.timerDelegate, mLevelControlDelegate);
    lcConfig.WithOnOff(mOnOffCluster.Cluster());
    lcConfig.WithLighting(DataModel::NullNullable);
    mLevelControlCluster.Create(lcConfig);
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

    return provider.AddEndpoint(mEndpointRegistration);
}

void DimmableLightDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);

    if (mGroupsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGroupsCluster.Cluster()));
        mGroupsCluster.Destroy();
    }

    if (mLevelControlCluster.IsConstructed())
    {
        if (static_cast<scenes::DefaultSceneHandlerImpl &>(mLevelControlCluster.Cluster()).IsInList())
        {
            Clusters::ScopedSceneTable table(mScenesTableProvider);
            table->UnregisterHandler(&mLevelControlCluster.Cluster());
        }
        if (mOnOffCluster.IsConstructed())
        {
            mOnOffCluster.Cluster().RemoveDelegate(&mLevelControlCluster.Cluster());
        }
        LogErrorOnFailure(provider.RemoveCluster(&mLevelControlCluster.Cluster()));
        mLevelControlCluster.Destroy();
    }

    if (mOnOffCluster.IsConstructed())
    {
        if (mOnOffCluster.Cluster().IsInList())
        {
            Clusters::ScopedSceneTable table(mScenesTableProvider);
            table->UnregisterHandler(&mOnOffCluster.Cluster());
        }

        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
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
