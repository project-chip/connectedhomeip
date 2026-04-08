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

#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/on-off-server/OnOffLightingCluster.h>
#include <app/clusters/scenes-server/SceneTable.h>
#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <devices/Types.h>
#include <devices/interface/SingleEndpointDevice.h>

namespace chip::app {

class LoggingOnOffLightDevice : public SingleEndpointDevice
{
public:
    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
    };

    LoggingOnOffLightDevice(const Context & context) :
        SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kOnOffLight, 1)), mContext(context)
    {}
    ~LoggingOnOffLightDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    class OnOffDelegate : public Clusters::OnOffDelegate
    {
        void OnOffStartup(bool on) override;
        void OnOnOffChanged(bool on) override;
    };

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

    OnOffDelegate mOnOffDelegate;
    DefaultScenesManagementTableProvider mScenesTableProvider;

    // Cluster Instances
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffLightingCluster> mOnOffCluster;
    LazyRegisteredServerCluster<Clusters::ScenesManagementCluster> mScenesManagementCluster;
    LazyRegisteredServerCluster<Clusters::GroupsCluster> mGroupsCluster;
};

} // namespace chip::app
