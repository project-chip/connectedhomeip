/*
*
*    Copyright (c) 2023 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <app/clusters/mode-base-server/mode-base-delegate.h>
#include <app/util/af.h>
#include <map>


namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {


class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{

public:
    // This map holds pointers to all initialised ModeBase instances. It provides a way to access all ModeBase clusters.
    static std::map<uint32_t, Instance*> ModeBaseAliasesInstanceMap;

    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Generic accessor functions
    EmberAfStatus GetFeature(uint32_t * value) const;
    EmberAfStatus SetFeatureMap(uint32_t value) const;
    EmberAfStatus GetCurrentMode(uint8_t *) const;
    EmberAfStatus SetCurrentMode(uint8_t) const;
    EmberAfStatus GetStartUpMode(DataModel::Nullable<uint8_t> &) const;
    EmberAfStatus SetStartUpMode(uint8_t value) const;
    EmberAfStatus SetStartUpModeNull() const;
    EmberAfStatus GetOnMode(DataModel::Nullable<uint8_t> &) const;
    EmberAfStatus SetOnMode(uint8_t value) const;
    EmberAfStatus SetOnModeNull() const;
    EndpointId GetEndpointId() const {return endpointId;}

private:
    EndpointId endpointId{};
    ClusterId clusterId{};
    Delegate *delegate;

    /**
     * This checks to see if this clusters instance is a valid ModeBase aliased cluster based on the AliasedClusters list.
     * @return true if the clusterId of this instance is a valid ModeBase cluster.
     */
    bool isAliasCluster() const;
    void HandleChangeToMode(HandlerContext & ctx, const Commands::ChangeToModeWithStatus::DecodableType & req);

public:
    /**
     * Creates a mode base cluster instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId The ID of the ModeBase aliased cluster to be instantiated.
     * @param aDelegate A pointer to a delegate that will handle application layer logic.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, Delegate * aDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        delegate(aDelegate)
    {
        endpointId = aEndpointId;
        clusterId  = aClusterId;
    }

    ~Instance() override
    {
        ModeBaseAliasesInstanceMap.erase(clusterId);
    }

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);
};

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip




// todo have this be part of the auto generated code
namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

const std::array<ClusterId , 5> AliasedClusters = {0x51, 0x52, 0x54, 0x55, 0x59};

namespace Attributes {

namespace SupportedModes {
static constexpr AttributeId Id = 0x00000000;
} // namespace SupportedModes

namespace CurrentMode {
static constexpr AttributeId Id = 0x00000001;
} // namespace CurrentMode

namespace StartUpMode {
static constexpr AttributeId Id = 0x00000002;
} // namespace StartUpMode

namespace OnMode {
static constexpr AttributeId Id = 0x00000003;
} // namespace OnMode

namespace GeneratedCommandList {
static constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
static constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace EventList {
static constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList

namespace AttributeList {
static constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
static constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
static constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
