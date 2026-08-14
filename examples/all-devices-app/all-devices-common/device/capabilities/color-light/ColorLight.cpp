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

#include <device/capabilities/color-light/ColorLight.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

ColorLight::ColorLight(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context) :
    SingleEndpoint(deviceTypes), mContext(context)
{}

Clusters::OnOffLightingCluster & ColorLight::GetOnOffCluster()
{
    VerifyOrDie(mOnOffCluster.IsConstructed());
    return mOnOffCluster.Cluster();
}

Clusters::LevelControlCluster & ColorLight::GetLevelControlCluster()
{
    VerifyOrDie(mLevelControlCluster.IsConstructed());
    return mLevelControlCluster.Cluster();
}

Clusters::IdentifyCluster & ColorLight::GetIdentifyCluster()
{
    VerifyOrDie(mIdentifyCluster.IsConstructed());
    return mIdentifyCluster.Cluster();
}

Clusters::GroupsCluster & ColorLight::GetGroupsCluster()
{
    VerifyOrDie(mGroupsCluster.IsConstructed());
    return mGroupsCluster.Cluster();
}

Clusters::ScenesManagementCluster & ColorLight::GetScenesManagementCluster()
{
    VerifyOrDie(mScenesManagementCluster.IsConstructed());
    return mScenesManagementCluster.Cluster();
}

Clusters::ColorControlCluster & ColorLight::GetColorControlCluster()
{
    VerifyOrDie(mColorControlCluster.IsConstructed());
    return mColorControlCluster.Cluster();
}

Clusters::DynamicLightingCluster & ColorLight::GetDynamicLightingCluster()
{
    VerifyOrDie(mDynamicLightingCluster.IsConstructed());
    return mDynamicLightingCluster.Cluster();
}

CHIP_ERROR ColorLight::RegisterIdentify(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mContext.timerDelegate));
    return provider.AddCluster(mIdentifyCluster.Registration());
}

CHIP_ERROR ColorLight::RegisterScenesManagement(EndpointId endpoint, CodeDrivenDataModelProvider & provider, bool supportsCopyScene)
{
    mScenesTableProvider.SetEndpoint(endpoint);
    mScenesManagementCluster.Create(endpoint,
                                    Clusters::ScenesManagementCluster::Context{
                                        .groupDataProvider  = &mContext.groupDataProvider,
                                        .fabricTable        = &mContext.fabricTable,
                                        .features           = {},
                                        .sceneTableProvider = mScenesTableProvider,
                                        .supportsCopyScene  = supportsCopyScene,
                                    });
    return provider.AddCluster(mScenesManagementCluster.Registration());
}

CHIP_ERROR ColorLight::RegisterOnOff(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                     std::optional<BitMask<Clusters::OnOff::Feature>> featureMap)
{
    // The scenes integration cannot come from the caller: the cluster it points at is created by
    // RegisterScenesManagement(), which must already have run.
    VerifyOrReturnError(mScenesManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

    Clusters::OnOffLightingCluster::Context onOffContext{
        .timerDelegate             = mContext.timerDelegate,
        .effectDelegate            = mDriver,
        .scenesIntegrationDelegate = &mScenesManagementCluster.Cluster(),
    };
    if (featureMap.has_value())
    {
        onOffContext.featureMap = *featureMap;
    }

    mOnOffCluster.Create(endpoint, onOffContext);
    mOnOffCluster.Cluster().AddDelegate(&mDriver);
    return provider.AddCluster(mOnOffCluster.Registration());
}

CHIP_ERROR ColorLight::RegisterLevelControl(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                            std::optional<BitMask<Clusters::LevelControl::Feature>> featureMap)
{
    VerifyOrReturnError(mOnOffCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

    Clusters::LevelControlCluster::Config config(mContext.timerDelegate, mDriver);
    // WithLighting() also applies the MinLevel/MaxLevel the Lighting feature mandates, which
    // setting the feature bit alone would not.
    config.WithOnOff(mOnOffCluster.Cluster()).WithLighting(DataModel::Nullable<uint8_t>());
    if (featureMap.has_value())
    {
        config.mFeatureMap = *featureMap;
    }

    mLevelControlCluster.Create(endpoint, config);
    mOnOffCluster.Cluster().AddDelegate(&mLevelControlCluster.Cluster());
    return provider.AddCluster(mLevelControlCluster.Registration());
}

CHIP_ERROR ColorLight::RegisterGroups(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    VerifyOrReturnError(mScenesManagementCluster.IsConstructed() && mIdentifyCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

    mGroupsCluster.Create(endpoint,
                          Clusters::GroupsCluster::Context{
                              .groupDataProvider   = mContext.groupDataProvider,
                              .scenesIntegration   = &mScenesManagementCluster.Cluster(),
                              .identifyIntegration = &mIdentifyCluster.Cluster(),
                          });
    return provider.AddCluster(mGroupsCluster.Registration());
}

CHIP_ERROR ColorLight::RegisterColorControl(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                            std::optional<BitMask<Clusters::ColorControl::Feature>> featureMap,
                                            Clusters::ColorControl::ColorValue initialColor)
{
    VerifyOrReturnError(mOnOffCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);

    Clusters::ColorControlCluster::Config config(mDriver);
    config.onOff = &mOnOffCluster.Cluster();
    // The ColorValue variant defaults to XY; the leaf picks the mode matching its feature map.
    config.mColorValue = initialColor;
    if (featureMap.has_value())
    {
        config.mFeatures = *featureMap;
    }

    mColorControlCluster.Create(endpoint, config);
    return provider.AddCluster(mColorControlCluster.Registration());
}

CHIP_ERROR ColorLight::RegisterDynamicLighting(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mDynamicLightingCluster.Create(endpoint);
    return provider.AddCluster(mDynamicLightingCluster.Registration());
}

void ColorLight::RegisterSceneHandlers()
{
    Clusters::ScopedSceneTable table(mScenesTableProvider);
    if (mOnOffCluster.IsConstructed())
    {
        table->RegisterHandler(&mOnOffCluster.Cluster());
    }
    if (mLevelControlCluster.IsConstructed())
    {
        table->RegisterHandler(&mLevelControlCluster.Cluster());
    }
    if (mColorControlCluster.IsConstructed())
    {
        table->RegisterHandler(&mColorControlCluster.Cluster());
    }
}

void ColorLight::UnregisterAll(CodeDrivenDataModelProvider & provider)
{
    // Disconnection (below) is strictly separated from destruction (further down): everything that
    // points at another cluster is unhooked while all cluster objects are still fully constructed,
    // so destruction order can never leave a dangling pointer behind.
    //
    // === PHASE 1: DISCONNECTION ===

    // The endpoint has to leave the provider before its clusters may be removed.
    UnregisterDescriptor(provider);

    // Unregister the scene handlers from the scene table. All three are registered together by
    // RegisterSceneHandlers(), so On/Off being in the handler list means the other two are as well
    // (it is the only one of them whose IsInList() is unambiguous: LevelControl is also an
    // OnOffDelegate, which carries a second intrusive list node).
    if (mOnOffCluster.IsConstructed() && mOnOffCluster.Cluster().IsInList())
    {
        Clusters::ScopedSceneTable table(mScenesTableProvider);
        table->UnregisterHandler(&mOnOffCluster.Cluster());
        if (mLevelControlCluster.IsConstructed())
        {
            table->UnregisterHandler(&mLevelControlCluster.Cluster());
        }
        if (mColorControlCluster.IsConstructed())
        {
            table->UnregisterHandler(&mColorControlCluster.Cluster());
        }
    }

    // Remove the delegates from the On/Off cluster.
    if (mOnOffCluster.IsConstructed())
    {
        mOnOffCluster.Cluster().RemoveDelegate(&mDriver);
        if (mLevelControlCluster.IsConstructed())
        {
            mOnOffCluster.Cluster().RemoveDelegate(&mLevelControlCluster.Cluster());
        }
    }

    // === PHASE 2: DESTRUCTION (reverse of the helper order in a leaf's Register) ===

    if (mDynamicLightingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mDynamicLightingCluster.Cluster()));
        mDynamicLightingCluster.Destroy();
    }

    if (mColorControlCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mColorControlCluster.Cluster()));
        mColorControlCluster.Destroy();
    }

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
