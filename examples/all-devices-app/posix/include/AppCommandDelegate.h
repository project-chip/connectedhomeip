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

#include <NamedPipeCommands.h>
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/clusters/boolean-state-server/BooleanStateCluster.h>
#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/util/basic-types.h>
#include <json/json.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "NamedPipeCommandHandler.h"

class AllDevicesAppCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;

    void RegisterOnOffCluster(chip::app::Clusters::OnOffCluster * cluster);
    void RegisterOccupancySensingCluster(chip::app::Clusters::OccupancySensingCluster * cluster);
    void RegisterBooleanStateCluster(chip::app::Clusters::BooleanStateCluster * cluster);
    void RegisterBasicInformationCluster(chip::app::Clusters::BasicInformationCluster * cluster);

    chip::app::Clusters::OnOffCluster * GetOnOffClusterByEndpoint(chip::EndpointId endpoint);
    chip::app::Clusters::OccupancySensingCluster * GetOccupancySensingClusterByEndpoint(chip::EndpointId endpoint);
    chip::app::Clusters::BooleanStateCluster * GetBooleanStateClusterByEndpoint(chip::EndpointId endpoint);
    chip::app::Clusters::BasicInformationCluster * GetBasicInformationClusterByEndpoint(chip::EndpointId endpoint);

    void RegisterCommandHandler(std::unique_ptr<NamedPipeCommandHandler> handler);
    void RegisterCommandHandlers();

private:
    static void DispatchCommand(intptr_t context);

    // Dynamic cluster registration lists.
    // As the application registers code-driven cluster instances during startup, they are
    // added to these vectors. The named pipe command delegate looks up the appropriate cluster
    // instance by Endpoint ID (retrieved dynamically via cluster->GetPaths()[0].mEndpointId)
    // when executing commands.
    //
    // Separate vectors are used for each cluster type because code-driven clusters do not
    // share a common polymorphic base class, which preserves type-safe API access.
    std::vector<chip::app::Clusters::OnOffCluster *> mOnOffClusters;
    std::vector<chip::app::Clusters::OccupancySensingCluster *> mOccupancySensingClusters;
    std::vector<chip::app::Clusters::BooleanStateCluster *> mBooleanStateClusters;
    std::vector<chip::app::Clusters::BasicInformationCluster *> mBasicInformationClusters;

    std::map<std::string, std::unique_ptr<NamedPipeCommandHandler>> mCommandHandlers;
};
