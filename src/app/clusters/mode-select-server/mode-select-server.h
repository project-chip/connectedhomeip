/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/clusters/mode-select-server/mode-select-delegate.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{

public:
    CHIP_ERROR Init();

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    // CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
    // override; CHIP_ERROR EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void *
    // context) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    EmberAfStatus GetCurrentMode(uint8_t *) const;
    EmberAfStatus SetCurrentMode(uint8_t) const;
    EmberAfStatus GetStartUpMode(DataModel::Nullable<uint8_t> &) const;
    EmberAfStatus SetStartUpMode(uint8_t value) const;
    EmberAfStatus SetStartUpModeNull() const;
    EmberAfStatus GetOnMode(DataModel::Nullable<uint8_t> &) const;
    EmberAfStatus SetOnMode(uint8_t value) const;
    EmberAfStatus SetOnModeNull() const;

private:
    EndpointId endpointId{};
    ClusterId clusterId{};
    Delegate * msDelegate;

    void ModeSelectHandleChangeToMode(HandlerContext & ctx, const Commands::ChangeToMode::DecodableType & req);

public:
    Instance(EndpointId aEndpointId, ClusterId aClusterId, Delegate * aDelegate) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId)
    {
        // todo check that the cluster ID given is a valid mode select cluster ID.
        endpointId = aEndpointId;
        clusterId  = aClusterId;
        msDelegate = aDelegate;
    }

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);
};

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
