/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "binding-handler.h"

#include "app-common/zap-generated/attribute-id.h"
#include "app-common/zap-generated/attributes/Accessors.h"
#include "app-common/zap-generated/cluster-id.h"
#include "app-common/zap-generated/command-id.h"
#include "app/CommandSender.h"
#include "app/clusters/bindings/BindingManager.h"
#include "app/server/Server.h"
#include "app/util/af.h"
#include "lib/core/CHIPError.h"

static void BoundDeviceChangedHandler(chip::EndpointId localEndpoint, chip::EndpointId remoteEndpoint, chip::ClusterId clusterId,
                                      chip::OperationalDeviceProxy * peer_device)
{
    using namespace chip;
    using namespace chip::app;
    // Unfortunately generating both cluster server and client code is not supported.
    // We need to manually compose the packet here.
    // TODO: investigate code generation issue for binding
    if (localEndpoint == 1 && clusterId == ZCL_ON_OFF_CLUSTER_ID)
    {
        bool onOffValue = false;
        Clusters::OnOff::Attributes::OnOff::Get(localEndpoint, &onOffValue);
        CommandId command           = onOffValue ? Clusters::OnOff::Commands::On::Id : Clusters::OnOff::Commands::Off::Id;
        CommandPathParams cmdParams = { remoteEndpoint, /* group id */ 0, clusterId, command,
                                        (chip::app::CommandPathFlags::kEndpointIdValid) };
        CommandSender sender(nullptr, peer_device->GetExchangeManager());
        sender.PrepareCommand(cmdParams);
        sender.FinishCommand();
        peer_device->SendCommands(&sender);
    }
}

CHIP_ERROR InitBindingHandlers()
{
    chip::BindingManager::GetInstance().SetAppServer(&chip::Server::GetInstance());
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(BoundDeviceChangedHandler);
    return CHIP_NO_ERROR;
}
