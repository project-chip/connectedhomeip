/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppConfig.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "app/CommandSender.h"
#include "app/clusters/bindings/BindingManager.h"
#include "app/server/Server.h"
#include "controller/InvokeInteraction.h"

namespace {
void BoundDeviceChangedHandler(const EmberBindingTableEntry * binding, chip::DeviceProxy * peer_device, void * context)
{
    using namespace chip;
    using namespace chip::app;

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "OnOff command succeeds");
    };
    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    if (binding->type == EMBER_MULTICAST_BINDING)
    {
        ChipLogError(NotSpecified, "Group binding is not supported now");
        return;
    }

    if (binding->type == EMBER_UNICAST_BINDING && binding->local == 1 && binding->clusterId == Clusters::OnOff::Id)
    {
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                         binding->remote, toggleCommand, onSuccess, onFailure);
    }
}
} // namespace

void SwitchToggleOnOff()
{
    chip::BindingManager::GetInstance().NotifyBoundClusterChanged(1, chip::app::Clusters::OnOff::Id, nullptr);
}

CHIP_ERROR InitBindingHandler()
{
    chip::BindingManager::GetInstance().SetAppServer(&chip::Server::GetInstance());
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(BoundDeviceChangedHandler);

    return CHIP_NO_ERROR;
}
