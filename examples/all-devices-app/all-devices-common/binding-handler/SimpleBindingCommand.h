/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <binding-handler/BindingContext.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <utility>

namespace chip {
namespace app {

/**
 * Reusable command context for standard ZAP-generated cluster commands.
 *
 * Automatically handles unicast InvokeCommandRequest and multicast
 * InvokeGroupCommandRequest with standard logging.
 *
 * @tparam CommandType Standard cluster command struct, e.g.:
 *         - Clusters::Chime::Commands::PlayChimeSound::Type
 *         - Clusters::OnOff::Commands::Toggle::Type
 *         - Clusters::LevelControl::Commands::MoveToLevel::Type
 */
template <typename CommandType>
class SimpleBindingCommand : public BindingContext
{
public:
    explicit SimpleBindingCommand(const CommandType & command) : mCommand(command) {}
    explicit SimpleBindingCommand(CommandType && command) : mCommand(std::move(command)) {}

    ClusterId GetClusterId() const override { return CommandType::GetClusterId(); }

    void DispatchCommand(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                         Messaging::ExchangeManager & exchangeManager) override
    {
        ClusterId targetClusterId = binding.clusterId.value_or(CommandType::GetClusterId());
        if (targetClusterId != CommandType::GetClusterId())
        {
            ChipLogDetail(AppServer, "Binding cluster mismatch (binding: 0x%" PRIx32 ", command: 0x%" PRIx32 ")", targetClusterId,
                          CommandType::GetClusterId());
            return;
        }

        if (binding.type == Clusters::Binding::MATTER_UNICAST_BINDING)
        {
            VerifyOrReturn(peer_device != nullptr && peer_device->ConnectionReady(),
                           ChipLogError(AppServer, "SimpleBindingCommand: Peer device connection not ready"));

            auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
                (void) status;
                (void) dataResponse;
                ChipLogProgress(AppServer, "Binding command (0x%" PRIx32 ", 0x%" PRIx32 ") succeeded on endpoint %u",
                                CommandType::GetClusterId(), CommandType::GetCommandId(), commandPath.mEndpointId);
            };

            auto onFailure = [](CHIP_ERROR error) {
                ChipLogError(AppServer, "Binding command (0x%" PRIx32 ", 0x%" PRIx32 ") failed: %" CHIP_ERROR_FORMAT,
                             CommandType::GetClusterId(), CommandType::GetCommandId(), error.Format());
            };

            CHIP_ERROR err =
                Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                                 binding.remote, mCommand, onSuccess, onFailure);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to invoke binding command: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
        else if (binding.type == Clusters::Binding::MATTER_MULTICAST_BINDING)
        {
            CHIP_ERROR err =
                Controller::InvokeGroupCommandRequest(&exchangeManager, binding.fabricIndex, binding.groupId, mCommand);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to invoke binding group command: %" CHIP_ERROR_FORMAT, err.Format());
            }
            else
            {
                ChipLogProgress(AppServer, "Binding group command sent to group %u", binding.groupId);
            }
        }
        else
        {
            ChipLogError(AppServer, "SimpleBindingCommand: Unsupported binding type %u", static_cast<unsigned>(binding.type));
        }
    }

private:
    CommandType mCommand;
};

} // namespace app
} // namespace chip
