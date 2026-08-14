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
#include <app/clusters/dynamic-lighting-server/DynamicLightingCluster.h>
#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <app/clusters/on-off-server/OnOffLightingCluster.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <device/capabilities/color-light/LoggingLightDriver.h>
#include <lib/support/TimerDelegate.h>

#include <optional>

namespace chip {
namespace app {

/**
 * ColorLight is the capability baseline for a dimmable light whose color is controllable.
 *
 * In the Matter Device Library that capability is shared by several leaf device types, such as
 * Color Temperature Light (0x010C) and Extended Color Light (0x010D). They register the same
 * clusters and wire them together the same way, but each has its own conformance table: the
 * features every cluster must advertise differ per device type.
 *
 * This class owns the clusters, the delegates behind them and the scene table provider, and
 * exposes one Register* helper per cluster. A leaf device type composes those helpers in its own
 * Register(), which is where its (compile-time) feature tables live - macros do not cross
 * translation units, so the conformance table and the code that applies it stay in the same file.
 *
 * Teardown is NOT split up: UnregisterAll() disconnects and destroys every cluster this class
 * owns, in the reverse of the order the helpers create them. Leaves call it from Unregister().
 */
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

    ~ColorLight() override = default;

    // Cluster getters for the application and tests. Only valid once the matching cluster has
    // been registered.
    Clusters::OnOffLightingCluster & GetOnOffCluster();
    Clusters::LevelControlCluster & GetLevelControlCluster();
    Clusters::IdentifyCluster & GetIdentifyCluster();
    Clusters::GroupsCluster & GetGroupsCluster();
    Clusters::ScenesManagementCluster & GetScenesManagementCluster();
    Clusters::ColorControlCluster & GetColorControlCluster();
    Clusters::DynamicLightingCluster & GetDynamicLightingCluster();
    Clusters::OccupancySensingCluster & GetOccupancySensingCluster();

protected:
    ColorLight(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context);

    /// Registration helpers, one per cluster. A leaf's Register() calls RegisterDescriptor()
    /// first, then these, then AddEndpoint(mEndpointRegistration).
    ///
    /// Order matters where clusters point at each other: Identify and ScenesManagement before
    /// On/Off and Groups, On/Off before Level Control and Color Control.
    ///
    /// Every feature map argument is optional: std::nullopt leaves the cluster's own default in
    /// place, which is what a leaf compiled with its device type macro turned off wants.
    CHIP_ERROR RegisterIdentify(EndpointId endpoint, CodeDrivenDataModelProvider & provider);

    /// supportsCopyScene advertises the CopyScene command, which both lighting device types list as
    /// mandatory; a leaf whose conformance table does not require it passes false.
    CHIP_ERROR RegisterScenesManagement(EndpointId endpoint, CodeDrivenDataModelProvider & provider, bool supportsCopyScene = true);
    CHIP_ERROR RegisterOnOff(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                             std::optional<BitMask<Clusters::OnOff::Feature>> featureMap = std::nullopt);
    CHIP_ERROR RegisterLevelControl(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                    std::optional<BitMask<Clusters::LevelControl::Feature>> featureMap = std::nullopt);
    CHIP_ERROR RegisterGroups(EndpointId endpoint, CodeDrivenDataModelProvider & provider);

    /// initialColor selects the mode the endpoint powers up in, so its variant alternative must be
    /// one the feature map advertises (CTColor for a color temperature light, XYColor for an
    /// extended color light).
    CHIP_ERROR RegisterColorControl(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                    std::optional<BitMask<Clusters::ColorControl::Feature>> featureMap = std::nullopt,
                                    Clusters::ColorControl::ColorValue initialColor = Clusters::ColorControl::CTColor{});
    CHIP_ERROR RegisterDynamicLighting(EndpointId endpoint, CodeDrivenDataModelProvider & provider);
    CHIP_ERROR RegisterOccupancySensing(EndpointId endpoint, CodeDrivenDataModelProvider & provider);

    /// Registers the scenable clusters (On/Off, Level Control, Color Control) with the endpoint's
    /// scene table. Call after those three are registered; skips any that are not.
    void RegisterSceneHandlers();

    /// Reverse of everything above: unhooks the scene handlers and delegates while all clusters are
    /// still alive, then removes and destroys each registered cluster in reverse creation order.
    void UnregisterAll(CodeDrivenDataModelProvider & provider);

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

    /// Owns the four delegates the clusters push state changes into.
    LoggingLightDriver mDriver;
    DefaultScenesManagementTableProvider mScenesTableProvider;

    LazyRegisteredServerCluster<Clusters::OnOffLightingCluster> mOnOffCluster;
    LazyRegisteredServerCluster<Clusters::LevelControlCluster> mLevelControlCluster;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
    LazyRegisteredServerCluster<Clusters::ScenesManagementCluster> mScenesManagementCluster;
    LazyRegisteredServerCluster<Clusters::ColorControlCluster> mColorControlCluster;
    LazyRegisteredServerCluster<Clusters::DynamicLightingCluster> mDynamicLightingCluster;
    LazyRegisteredServerCluster<Clusters::OccupancySensingCluster> mOccupancySensingCluster;
};

} // namespace app
} // namespace chip
