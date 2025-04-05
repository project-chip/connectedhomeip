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

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

/**
 *  @brief App should instantiate and init one Interface per endpoint
 */
class Interface : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Interface(EndpointId endpoint, ClusterLogic & clusterLogic) :
        AttributeAccessInterface(Optional<EndpointId>(endpoint), Id), CommandHandlerInterface(Optional<EndpointId>(endpoint), Id),
        mClusterLogic(clusterLogic)
    {}

    /**
     * @brief Overides the Callback for reading attributes.
     *
     * @param [in] aPath indicates which exact data is being read.
     * @param [in] aEncoder the AttributeValueEncoder to use for encoding the
     *             data.
     * @return return error for failed read, return CHIP_NO_ERROR for succesful read.
     */
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    /**
     * @brief  Callback for writing attributes.
     *
     * @param [in] aPath indicates which exact data is being written.
     * @param [in] aDecoder the AttributeValueDecoder to use for decoding the
     *             data.
     *
     *@return return error for failed write, return CHIP_NO_ERROR for succesful write.
     */
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    /**
     * @brief This function overides callback that must be implemented to handle an invoke command request.
     *
     * @param [in] handlerContext Context that encapsulates the current invoke request.
     *                            Handlers are responsible for correctly calling SetCommandHandled()
     *                            on the context if they did handle the command.
     *
     *                            This is not necessary if the HandleCommand() method below is invoked.
     */
    void InvokeCommand(HandlerContext & handlerContext) override;

    /**
     * @brief This function registers attribute and command handlers.
     * @return CHIP_NO_ERROR when succesfully initialized or return error.
     */
    CHIP_ERROR Init();

    /**
     * @brief This function unregisters attribute and command handlers.
     * @return CHIP_NO_ERROR when succesfully initialized or return error.
     */
    CHIP_ERROR Shutdown();

private:
    // This is owned by the caller and passed to the interface for its use.
    ClusterLogic & mClusterLogic;
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
