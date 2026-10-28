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

#include <binding-handler/BindingCommand.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <utility>
#include <variant>

namespace chip {
namespace app {

/**
 * Reusable BindingCommand for standard ZAP-generated cluster commands.
 *
 * Sends unicast targets an InvokeCommandRequest and group targets an InvokeGroupCommandRequest, and
 * logs the result.
 *
 * @tparam CommandType Standard cluster command struct, e.g.:
 *         - Clusters::Chime::Commands::PlayChimeSound::Type
 *         - Clusters::OnOff::Commands::Toggle::Type
 *         - Clusters::LevelControl::Commands::MoveToLevel::Type
 */
template <typename CommandType>
class SimpleBindingCommand : public BindingCommand
{
public:
    explicit SimpleBindingCommand(const CommandType & command) : mCommand(command) {}
    explicit SimpleBindingCommand(CommandType && command) : mCommand(std::move(command)) {}

    ClusterId GetClusterId() const override { return CommandType::GetClusterId(); }

    void Dispatch(const BindingTarget & target) override
    {
        if (const auto * unicast = std::get_if<UnicastBindingTarget>(&target))
        {
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

            CHIP_ERROR err = Controller::InvokeCommandRequest(&unicast->exchangeManager, unicast->session, unicast->endpoint,
                                                              mCommand, onSuccess, onFailure);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to invoke binding command: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
        else if (const auto * group = std::get_if<GroupBindingTarget>(&target))
        {
            CHIP_ERROR err =
                Controller::InvokeGroupCommandRequest(&group->exchangeManager, group->fabricIndex, group->groupId, mCommand);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to invoke binding group command: %" CHIP_ERROR_FORMAT, err.Format());
            }
            else
            {
                ChipLogProgress(AppServer, "Binding group command sent to group %u", group->groupId);
            }
        }
    }

private:
    CommandType mCommand;
};

} // namespace app
} // namespace chip
