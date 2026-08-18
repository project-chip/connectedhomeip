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

ColorLight::ColorLight(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context, const Delegates & delegates,
                       const Conformance & conformance) :
    SingleEndpoint(deviceTypes), mContext(context), mConformance(conformance), mOnOffDelegate(delegates.onOff),
    mLevelControlDelegate(delegates.levelControl), mEffectDelegate(delegates.effect), mColorDelegate(delegates.color),
    mIdentifyDelegate(delegates.identify)
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

// Creates every cluster the leaf device types require, in an order the clusters themselves impose:
// Identify and ScenesManagement before On/Off and Groups, On/Off before Level Control and Color
// Control, because each of those points at the one before it.
//
// The element requirements both device types share are satisfied here (they are mandatory in the
// 0x010C table and the 0x010D table alike):
//
//   Identify       TriggerEffect command     the Identify delegate's IsTriggerEffectEnabled()
//   Scenes Mgmt    CopyScene command         supportsCopyScene below
//   On/Off         Lighting feature          the On/Off feature map below
//   Level Control  OnOff feature             Config::WithOnOff()
//   Level Control  Lighting feature          Config::WithLighting(), which also applies the
//                  MinLevel 1 / MaxLevel 254 the table constrains them to
//   Color Control  RemainingTime attribute   always advertised by ColorControlCluster
//
// The Level Control features are not set directly: the two builders above are what apply them, and
// assigning Config::mFeatureMap afterwards would overwrite the bits they set while keeping the
// MinLevel/MaxLevel and On/Off wiring they also install.
//
// Color Control's feature map is the one requirement that differs per device type, so it comes from
// the leaf's Conformance.
CHIP_ERROR ColorLight::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mContext.timerDelegate).WithDelegate(&mIdentifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mScenesTableProvider.SetEndpoint(endpoint);
    mScenesManagementCluster.Create(endpoint,
                                    Clusters::ScenesManagementCluster::Context{
                                        .groupDataProvider  = &mContext.groupDataProvider,
                                        .fabricTable        = &mContext.fabricTable,
                                        .features           = {},
                                        .sceneTableProvider = mScenesTableProvider,
                                        .supportsCopyScene  = true,
                                    });
    ReturnErrorOnFailure(provider.AddCluster(mScenesManagementCluster.Registration()));

    mOnOffCluster.Create(endpoint,
                         Clusters::OnOffLightingCluster::Context{
                             .timerDelegate             = mContext.timerDelegate,
                             .effectDelegate            = mEffectDelegate,
                             .scenesIntegrationDelegate = &mScenesManagementCluster.Cluster(),
                             .featureMap                = BitMask<Clusters::OnOff::Feature>(Clusters::OnOff::Feature::kLighting),
                         });
    mOnOffCluster.Cluster().AddDelegate(&mOnOffDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    Clusters::LevelControlCluster::Config levelConfig(mContext.timerDelegate, mLevelControlDelegate);
    levelConfig.WithOnOff(mOnOffCluster.Cluster()).WithLighting(DataModel::Nullable<uint8_t>());
    mLevelControlCluster.Create(endpoint, levelConfig);
    mOnOffCluster.Cluster().AddDelegate(&mLevelControlCluster.Cluster());
    ReturnErrorOnFailure(provider.AddCluster(mLevelControlCluster.Registration()));

    mGroupsCluster.Create(endpoint,
                          Clusters::GroupsCluster::Context{
                              .groupDataProvider   = mContext.groupDataProvider,
                              .scenesIntegration   = &mScenesManagementCluster.Cluster(),
                              .identifyIntegration = &mIdentifyCluster.Cluster(),
                          });
    ReturnErrorOnFailure(provider.AddCluster(mGroupsCluster.Registration()));

    Clusters::ColorControlCluster::Config colorConfig(mColorDelegate);
    colorConfig.onOff = &mOnOffCluster.Cluster();
    // The ColorValue variant defaults to XY; the leaf picks the mode matching its feature map.
    colorConfig.mColorValue = mConformance.initialColor;
    colorConfig.mFeatures   = mConformance.colorFeatures;
    mColorControlCluster.Create(endpoint, colorConfig);
    ReturnErrorOnFailure(provider.AddCluster(mColorControlCluster.Registration()));

    mDynamicLightingCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mDynamicLightingCluster.Registration()));

    // Scenes are enabled, so the scenable clusters must be registered as handlers to be able to
    // save and recall scenes.
    {
        Clusters::ScopedSceneTable table(mScenesTableProvider);
        table->RegisterHandler(&mOnOffCluster.Cluster());
        table->RegisterHandler(&mLevelControlCluster.Cluster());
        table->RegisterHandler(&mColorControlCluster.Cluster());
    }

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ColorLight::Unregister(CodeDrivenDataModelProvider & provider)
{
    // Disconnection (below) is strictly separated from destruction (further down): everything that
    // points at another cluster is unhooked while all cluster objects are still fully constructed,
    // so destruction order can never leave a dangling pointer behind.
    //
    // Every cluster is checked for construction because a failed Register() rolls back through
    // here, having created only part of the endpoint.
    //
    // === PHASE 1: DISCONNECTION ===

    // The endpoint has to leave the provider before its clusters may be removed.
    UnregisterDescriptor(provider);

    // Unregister the scene handlers from the scene table. All three are registered together by
    // Register(), so On/Off being in the handler list means the other two are as well (it is the
    // only one of them whose IsInList() is unambiguous: LevelControl is also an OnOffDelegate,
    // which carries a second intrusive list node).
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
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        if (mLevelControlCluster.IsConstructed())
        {
            mOnOffCluster.Cluster().RemoveDelegate(&mLevelControlCluster.Cluster());
        }
    }

    // === PHASE 2: DESTRUCTION (reverse of the creation order in Register) ===

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
