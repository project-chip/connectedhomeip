/**
 *
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
 *
 */
#pragma once
#include "closure-dimension-cluster-logic.h"

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <app/util/config.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

// App should instantiate and init one Interface per endpoint
class Interface : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Interface(EndpointId endpoint, ClusterLogic & clusterLogic) :
        AttributeAccessInterface(Optional<EndpointId>(endpoint), Id), CommandHandlerInterface(Optional<EndpointId>(endpoint), Id),
        mClusterLogic(clusterLogic)
    {}
    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;

    // Registers this handler.
    CHIP_ERROR Init();

    // TODO: Shutdown

private:
    // This is owned by the caller and passed to the interface for its use.
    ClusterLogic & mClusterLogic;
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
