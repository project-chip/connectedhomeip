/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * @brief Closure Control cluster interface implementation
 *        Applications should instantiate and init one Interface per endpoint
 *
 */
class Interface : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Interface(EndpointId endpoint, ClusterLogic & clusterLogic) :
        AttributeAccessInterface(Optional<EndpointId>(endpoint), Id), CommandHandlerInterface(Optional<EndpointId>(endpoint), Id),
        mClusterLogic(clusterLogic)
    {}

    virtual ~Interface() = default;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;

    /**
     * @brief This function registers attribute access and command handler.
     *
     * @return CHIP_NO_ERROR when succesfully initialized.
     *         Aborts if registration fails.
     */
    CHIP_ERROR Init();

    /**
     * @brief This function registers attribute access and command handler.
     *
     * @return CHIP_NO_ERROR when succesfully initialized.
     *         Aborts if registration fails.
     */
    CHIP_ERROR Shutdown();

private:
    ClusterLogic & mClusterLogic;
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
