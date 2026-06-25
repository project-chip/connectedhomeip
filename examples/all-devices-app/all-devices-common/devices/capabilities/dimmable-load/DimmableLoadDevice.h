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

#pragma once

#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <app/clusters/on-off-server/OnOffLightingCluster.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/**
 * DimmableLoadDevice represents the generic capability baseline for any Matter device
 * that supports on/off control and level/intensity adjustment (dimming).
 *
 * In the Matter Device Library, this capability is shared across multiple spec-defined
 * leaf device types, such as Dimmable Light (0x0101), Dimmable Plug-In Unit (0x010A),
 * and Mounted Dimmable Load Control (0x0104).
 *
 * Rather than duplicating cluster registration and wiring across these leaf types,
 * this class acts as the single source of truth for the shared "dimmable load" capability.
 * It automatically registers and wires the mandatory server clusters:
 *   - OnOff (with lighting context)
 *   - LevelControl (with lighting transitions)
 *   - Identify
 *   - ScenesManagement
 *   - Groups
 *
 * It exposes public getters for all underlying clusters, enabling the application
 * and test harnesses to programmatically inspect and control the device state.
 *
 * This class is abstract and is intended to be subclassed by concrete leaf device
 * implementations that pass their specific Matter Device Type ID.
 */
class DimmableLoadDevice : public SingleEndpointDevice
{
public:
    struct LevelControlConfig
    {
        // Optional LevelControl transition attributes.
        // We use std::optional to represent support (presence of the attribute on the endpoint).
        // For nullable attributes, we wrap DataModel::Nullable.
        // By default, a default-constructed LevelControlConfig has all optionals set to std::nullopt (unsupported).
        std::optional<uint16_t> onOffTransitionTime;
        std::optional<DataModel::Nullable<uint16_t>> onTransitionTime;
        std::optional<DataModel::Nullable<uint16_t>> offTransitionTime;
        std::optional<DataModel::Nullable<uint8_t>> defaultMoveRate;

        // Optional LevelControl bounds and startup settings.
        std::optional<DataModel::Nullable<uint8_t>> startUpCurrentLevel;
        std::optional<uint8_t> minLevel;
        std::optional<uint8_t> maxLevel;

        /// Returns a LevelControlConfig populated with the specific overrides required
        /// to satisfy the global CI PICS tests (present with value 0 or null).
        static constexpr LevelControlConfig CiPicsDefaults()
        {
            return LevelControlConfig{
                0,                                // onOffTransitionTime
                DataModel::Nullable<uint16_t>(0), // onTransitionTime
                DataModel::Nullable<uint16_t>(0), // offTransitionTime
                DataModel::Nullable<uint8_t>(),   // defaultMoveRate
                DataModel::Nullable<uint8_t>()    // startUpCurrentLevel
            };
        }
    };

    struct Config
    {
        LevelControlConfig levelControl = {};

        // Optional static semantic tags to associate with this dimmable load endpoint.
        Span<const Clusters::Globals::Structs::SemanticTagStruct::Type> tagList = {};
    };

    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
    };

    struct Delegates
    {
        Clusters::OnOffDelegate & onOff;
        Clusters::LevelControlDelegate & levelControl;
        Clusters::OnOffEffectDelegate & effect;
        Clusters::IdentifyDelegate & identify;
    };

    DimmableLoadDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes, const Context & context, const Delegates & delegates,
                       const Config & config);
    ~DimmableLoadDevice() override = default;

    // Public cluster getters for application and testing
    Clusters::OnOffLightingCluster & OnOffCluster() { return mOnOffCluster.Cluster(); }
    Clusters::LevelControlCluster & LevelControlCluster() { return mLevelControlCluster.Cluster(); }
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::ScenesManagementCluster & ScenesManagementCluster() { return mScenesManagementCluster.Cluster(); }
    Clusters::GroupsCluster & GroupsCluster() { return mGroupsCluster.Cluster(); }

protected:
    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    Clusters::OnOffDelegate & mOnOffDelegate;
    Clusters::LevelControlDelegate & mLevelControlDelegate;
    Clusters::OnOffEffectDelegate & mEffectDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;
    const Context mContext;
    const Config mConfig;

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

    DefaultScenesManagementTableProvider mScenesTableProvider;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffLightingCluster> mOnOffCluster;
    LazyRegisteredServerCluster<Clusters::LevelControlCluster> mLevelControlCluster;
    LazyRegisteredServerCluster<Clusters::ScenesManagementCluster> mScenesManagementCluster;
    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
};

} // namespace app
} // namespace chip
