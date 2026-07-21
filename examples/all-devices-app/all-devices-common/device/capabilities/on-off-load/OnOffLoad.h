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
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <app/clusters/on-off-server/OnOffLightingCluster.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/**
 * OnOffLoad represents the generic capability baseline for any Matter device
 * that supports basic on/off control without level adjustment.
 *
 * In the Matter Device Library, this capability is shared across multiple spec-defined
 * leaf device types, such as On/Off Light (0x0100), On/Off Plug-In Unit (0x010B),
 * and Mounted On/Off Control (0x0103).
 *
 * This class centralizes the registration and wiring of the mandatory server clusters
 * for all on/off loads:
 *   - OnOff (with lighting context)
 *   - Identify
 *   - ScenesManagement
 *   - Groups
 *
 * It exposes public getters for all underlying clusters to allow programmatic state
 * inspection and control by the application and test scripts.
 *
 * This class is abstract and is intended to be subclassed by concrete leaf device
 * implementations that pass their specific Matter Device Type ID.
 */
class OnOffLoad : public SingleEndpoint
{
public:
    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
    };

    OnOffLoad(Span<const DataModel::DeviceTypeEntry> deviceTypes, Clusters::OnOffDelegate & onOffDelegate,
              Clusters::OnOffEffectDelegate & effectDelegate, Clusters::IdentifyDelegate & identifyDelegate,
              const Context & context);
    ~OnOffLoad() override = default;

    // Public cluster getters for application and testing
    Clusters::OnOffLightingCluster & OnOffCluster() { return mOnOffCluster.Cluster(); }
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::ScenesManagementCluster & ScenesManagementCluster() { return mScenesManagementCluster.Cluster(); }
    Clusters::GroupsCluster & GroupsCluster() { return mGroupsCluster.Cluster(); }

protected:
    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    Clusters::OnOffDelegate & mOnOffDelegate;
    Clusters::OnOffEffectDelegate & mEffectDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;
    const Context mContext;

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
    LazyRegisteredServerCluster<Clusters::ScenesManagementCluster> mScenesManagementCluster;
    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
};

} // namespace app
} // namespace chip
