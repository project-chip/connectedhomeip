/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/scenes-server/ScenesManagementCluster.h>

// most ember implementations will want acces to these
#include <app/clusters/scenes-server/SceneHandlerImpl.h>

namespace chip::app::Clusters::ScenesManagement {

class ScenesServer
{
public:
    static constexpr SceneId kGlobalSceneId      = 0x00;
    static constexpr GroupId kGlobalSceneGroupId = 0x0000;

    ScenesServer()  = default;
    ~ScenesServer() = default;

    static ScenesServer & Instance();

    // Callbacks
    void GroupWillBeRemoved(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId);
    void MakeSceneInvalid(EndpointId aEndpointId, FabricIndex aFabricIx);
    void MakeSceneInvalidForAllFabrics(EndpointId aEndpointId);
    void StoreCurrentScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId);
    void RecallScene(FabricIndex aFabricIx, EndpointId aEndpointId, GroupId aGroupId, SceneId aSceneId);

    // Handlers for extension field sets
    bool IsHandlerRegistered(EndpointId endpointId, scenes::SceneHandler * handler);
    void RegisterSceneHandler(EndpointId endpointId, scenes::SceneHandler * handler);
    void UnregisterSceneHandler(EndpointId endpointId, scenes::SceneHandler * handler);

    // Fabric
    void RemoveFabric(EndpointId aEndpointId, FabricIndex aFabricIndex);
};

/// Returns the cluster instance on the given endpoint or nullptr if the cluster does not exist
/// or was not yet initialized
ScenesManagementCluster * FindClusterOnEndpoint(EndpointId endpointId);

} // namespace chip::app::Clusters::ScenesManagement
