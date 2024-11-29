/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AdministratorCommissioning {

class AdministratorCommissioningServer : private CommandHandlerInterface
{
public:
    /**
     * Creates an administrator commissioning server instance. The Init() function needs to be called for this instance to be
     * registered and called by the interaction model at the appropriate times.
     * @param endpointId The endpoint on which this cluster exists.
     */
    AdministratorCommissioningServer(EndpointId endpointId);
    ~AdministratorCommissioningServer() override;

    /**
     * Initialise the administrator commissioning server instance.
     * @return Returns an error if the CommandHandler registration fails.
     */
    CHIP_ERROR Init();

    /**
     * Shuts down the administrator commissioning server instance.
     * @return Returns an error if the CommandHandler unregistration fails.
     */
    CHIP_ERROR Shutdown();

private:
    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & context) override;

    // Methods to handle the various commands this cluster may receive.
    void OpenCommissioningWindow(HandlerContext & context, const Commands::OpenCommissioningWindow::DecodableType & commandData);
    void OpenBasicCommissioningWindow(HandlerContext & context,
                                      const Commands::OpenBasicCommissioningWindow::DecodableType & commandData);
    void RevokeCommissioning(HandlerContext & context, const Commands::RevokeCommissioning::DecodableType & commandData);
};

} // namespace AdministratorCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
