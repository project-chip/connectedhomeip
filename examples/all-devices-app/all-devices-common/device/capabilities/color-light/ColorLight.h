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

#pragma once

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/clusters/color-control-server/ColorControlDelegate.h>
#include <app/clusters/dynamic-lighting-server/DynamicLightingCluster.h>
#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <app/clusters/on-off-server/OnOffLightingCluster.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

//   ColorLight is the capability baseline for a dimmable light whose color is controllable.

//   In the Matter Device Library that capability is shared by several leaf device types, such as
//   Color Temperature Light (0x010C) and Extended Color Light (0x010D). Their element requirements
//   are identical except for the Color Control feature map, so Register() applies the shared ones
//   (the table above its definition lists them) and a leaf supplies only its Conformance.

//   This class owns the clusters, the delegates behind them and the scene table provider, and both
//   builds and tears the endpoint down. A leaf device type is therefore just a constructor: it
//   names its device type and passes its Conformance.

class ColorLight : public SingleEndpoint
{
public:
    /// The platform objects the clusters need. Everything else this endpoint requires - the
    /// delegates behind On/Off, Level Control and Color Control, and the scene table provider -
    /// is owned by this class.
    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
    };

    /// The output side of the light: what the clusters push state changes into. This class only
    /// wires them up, so a product supplies its own implementations here and reuses everything
    /// else. LoggingLightDriver is one such implementation.
    struct Delegates
    {
        Clusters::OnOffDelegate & onOff;
        Clusters::LevelControlDelegate & levelControl;
        Clusters::OnOffEffectDelegate & effect;
        Clusters::ColorControlDelegate & color;
        Clusters::IdentifyDelegate & identify;
    };

    ~ColorLight() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Cluster getters for the application and tests. Only valid once Register() has run.
    Clusters::OnOffLightingCluster & GetOnOffCluster();
    Clusters::LevelControlCluster & GetLevelControlCluster();
    Clusters::IdentifyCluster & GetIdentifyCluster();
    Clusters::GroupsCluster & GetGroupsCluster();
    Clusters::ScenesManagementCluster & GetScenesManagementCluster();
    Clusters::ColorControlCluster & GetColorControlCluster();
    Clusters::DynamicLightingCluster & GetDynamicLightingCluster();

protected:
    /// The only part of the Device Library conformance that differs between the leaves built on
    /// this class. It belongs to the device type, not to the application, which is why it is a
    /// constructor argument of the leaf rather than a field of Context.
    struct Conformance
    {
        BitMask<Clusters::ColorControl::Feature> colorFeatures;

        /// The mode the endpoint powers up in. Its variant alternative must be one colorFeatures
        /// advertises: CTColor with kColorTemperature, XYColor with kXy.
        Clusters::ColorControl::ColorValue initialColor;
    };

    ColorLight(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context, const Delegates & delegates,
               const Conformance & conformance);

private:
    class DefaultScenesManagementTableProvider : public Clusters::ScenesManagementTableProvider
    {
    public:
        Clusters::ScenesManagementSceneTable * Take() override
        {
            return scenes::GetSceneTableImpl(mEndpointId, scenes::kMaxScenesPerEndpoint);
        }
        void Release(Clusters::ScenesManagementSceneTable *) override {}

        void SetEndpoint(EndpointId endpoint) { mEndpointId = endpoint; }

    private:
        EndpointId mEndpointId = kInvalidEndpointId;
    };

    const Context mContext;
    const Conformance mConformance;

    Clusters::OnOffDelegate & mOnOffDelegate;
    Clusters::LevelControlDelegate & mLevelControlDelegate;
    Clusters::OnOffEffectDelegate & mEffectDelegate;
    Clusters::ColorControlDelegate & mColorDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;

    DefaultScenesManagementTableProvider mScenesTableProvider;

    LazyRegisteredServerCluster<Clusters::OnOffLightingCluster> mOnOffCluster;
    LazyRegisteredServerCluster<Clusters::LevelControlCluster> mLevelControlCluster;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
    LazyRegisteredServerCluster<Clusters::ScenesManagementCluster> mScenesManagementCluster;
    LazyRegisteredServerCluster<Clusters::ColorControlCluster> mColorControlCluster;
    LazyRegisteredServerCluster<Clusters::DynamicLightingCluster> mDynamicLightingCluster;
};

} // namespace app
} // namespace chip
