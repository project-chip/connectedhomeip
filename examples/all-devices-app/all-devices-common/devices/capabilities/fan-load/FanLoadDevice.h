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

#pragma once

#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/**
 * FanLoadDevice represents the generic capability baseline for any Matter device
 * that supports comfort air circulation and fan speed control.
 *
 * In the Matter Device Library, this capability is shared across multiple spec-defined
 * leaf device types, such as Fan (0x002B), Air Purifier (0x002D), and Extractor Hood (0x005A).
 *
 * This class centralizes the registration and wiring of the server clusters
 * for all fan-like loads:
 *   - FanControl
 *   - OnOff (optional, for basic power toggling)
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
class FanLoadDevice : public SingleEndpointDevice
{
public:
    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
        bool includeOnOffCluster                                                = true;
        Span<const Clusters::Globals::Structs::SemanticTagStruct::Type> tagList = {};
    };

    ~FanLoadDevice() override = default;

    // Public cluster getters for usability and external control
    Clusters::FanControlCluster & FanControlCluster() { return mFanControlCluster.Cluster(); }
    Clusters::OnOffCluster * OnOffCluster() { return mContext.includeOnOffCluster ? &mOnOffCluster.Cluster() : nullptr; }
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::ScenesManagementCluster & ScenesManagementCluster() { return mScenesManagementCluster.Cluster(); }
    Clusters::GroupsCluster & GroupsCluster() { return mGroupsCluster.Cluster(); }

protected:
    FanLoadDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes, Clusters::FanControl::Delegate & fanDelegate,
                  Clusters::OnOffDelegate * onOffDelegate, const Context & context);

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

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

    Clusters::FanControl::Delegate & mFanDelegate;
    Clusters::OnOffDelegate * mOnOffDelegate = nullptr;
    TimerDelegate & mTimerDelegate;

    const Context mContext;

    DefaultScenesManagementTableProvider mScenesTableProvider;

    LazyRegisteredServerCluster<Clusters::FanControlCluster> mFanControlCluster;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffCluster> mOnOffCluster;
    LazyRegisteredServerCluster<Clusters::ScenesManagementCluster> mScenesManagementCluster;
    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
};

} // namespace app
} // namespace chip
