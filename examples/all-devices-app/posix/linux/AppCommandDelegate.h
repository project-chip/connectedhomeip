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
#include <app/util/basic-types.h>
#include <json/json.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
class OnOffCluster;
class OccupancySensingCluster;
class BooleanStateCluster;

namespace BasicInformation {
template <typename Policy>
class PolicyBased;
}
class DeviceLayerBasicInformationPolicy;
using BasicInformationCluster = BasicInformation::PolicyBased<DeviceLayerBasicInformationPolicy>;

} // namespace Clusters
} // namespace app
} // namespace chip

class AllDevicesAppCommandDelegate;

class NamedPipeCommandHandler
{
public:
    virtual ~NamedPipeCommandHandler()                                                                                  = default;
    virtual const char * GetName() const                                                                                = 0;
    virtual void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, chip::EndpointId endpointId) = 0;
};

class AllDevicesAppCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;

    void RegisterOnOffCluster(chip::EndpointId endpoint, chip::app::Clusters::OnOffCluster * cluster);
    void RegisterOccupancySensingCluster(chip::EndpointId endpoint, chip::app::Clusters::OccupancySensingCluster * cluster);
    void RegisterBooleanStateCluster(chip::EndpointId endpoint, chip::app::Clusters::BooleanStateCluster * cluster);
    void RegisterBasicInformationCluster(chip::EndpointId endpoint, chip::app::Clusters::BasicInformationCluster * cluster);

    chip::app::Clusters::OnOffCluster * GetOnOffClusterByEndpoint(chip::EndpointId endpoint);
    chip::app::Clusters::OccupancySensingCluster * GetOccupancySensingClusterByEndpoint(chip::EndpointId endpoint);
    chip::app::Clusters::BooleanStateCluster * GetBooleanStateClusterByEndpoint(chip::EndpointId endpoint);
    chip::app::Clusters::BasicInformationCluster * GetBasicInformationClusterByEndpoint(chip::EndpointId endpoint);

    void RegisterCommandHandler(std::unique_ptr<NamedPipeCommandHandler> handler);
    void RegisterCommandHandlers();

private:
    static void DispatchCommand(intptr_t context);

    struct OnOffClusterEntry
    {
        chip::EndpointId endpoint;
        chip::app::Clusters::OnOffCluster * cluster;
    };
    struct OccupancySensingClusterEntry
    {
        chip::EndpointId endpoint;
        chip::app::Clusters::OccupancySensingCluster * cluster;
    };
    struct BooleanStateClusterEntry
    {
        chip::EndpointId endpoint;
        chip::app::Clusters::BooleanStateCluster * cluster;
    };
    struct BasicInformationClusterEntry
    {
        chip::EndpointId endpoint;
        chip::app::Clusters::BasicInformationCluster * cluster;
    };

    std::vector<OnOffClusterEntry> mOnOffClusters;
    std::vector<OccupancySensingClusterEntry> mOccupancySensingClusters;
    std::vector<BooleanStateClusterEntry> mBooleanStateClusters;
    std::vector<BasicInformationClusterEntry> mBasicInformationClusters;

    std::map<std::string, std::unique_ptr<NamedPipeCommandHandler>> mCommandHandlers;
};
