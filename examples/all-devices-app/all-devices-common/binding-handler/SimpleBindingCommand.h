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

#include <cstdint>
#include <utility>
#include <variant>

namespace chip {
namespace app {

/**
 * Reusable BindingCommand for standard ZAP-generated cluster commands.
 *
 * Sends unicast targets an InvokeCommandRequest and group targets an InvokeGroupCommandRequest, and
 * reports one completion per target through options.onComplete. Failures are also logged.
 *
 * Commands that require a timed invoke (CommandType::MustUseTimedInvoke()) are sent to unicast targets
 * with a kTimedInvokeTimeoutMs timeout. Group targets are skipped for these commands because group
 * invokes cannot be timed. Skipped targets complete with CHIP_ERROR_INVALID_ARGUMENT.
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
    static constexpr uint16_t kTimedInvokeTimeoutMs = 10000;

    explicit SimpleBindingCommand(const CommandType & command, BindingCommandOptions options = {}) :
        mCommand(command), mOptions(options)
    {}
    explicit SimpleBindingCommand(CommandType && command, BindingCommandOptions options = {}) :
        mCommand(std::move(command)), mOptions(options)
    {}

    ClusterId GetClusterId() const override { return CommandType::GetClusterId(); }

    void Failed(const Clusters::Binding::TableEntry & target, CHIP_ERROR error) override
    {
        BindingCommand::Failed(target, error);
        mOptions.onComplete(target, error);
    }

    void Dispatch(const BindingTarget & target) override
    {
        const auto callback = mOptions.onComplete;
        if (const auto * unicast = std::get_if<UnicastBindingTarget>(&target))
        {
            const auto peer    = unicast->session->GetPeer();
            const auto binding = unicast->binding != nullptr
                ? *unicast->binding
                : Clusters::Binding::TableEntry::ForNode(peer.GetFabricIndex(), peer.GetNodeId(), kInvalidEndpointId,
                                                         unicast->endpoint, GetClusterId());
            auto onSuccess     = [callback, binding](const ConcreteCommandPath &, const StatusIB &, const auto &) {
                callback(binding, CHIP_NO_ERROR);
            };
            auto onFailure = [callback, binding](CHIP_ERROR error) {
                ChipLogError(AppServer, "Binding command failed: %" CHIP_ERROR_FORMAT, error.Format());
                callback(binding, error);
            };
            Optional<uint16_t> timedTimeout;
            if constexpr (CommandType::MustUseTimedInvoke())
            {
                timedTimeout.SetValue(kTimedInvokeTimeoutMs);
            }
            CHIP_ERROR error =
                Controller::InvokeCommandRequest(&unicast->exchangeManager, unicast->session, unicast->endpoint, mCommand,
                                                 onSuccess, onFailure, timedTimeout, MakeOptional(mOptions.responseTimeout));
            if (error != CHIP_NO_ERROR)
            {
                onFailure(error);
            }
        }
        else if (const auto * group = std::get_if<GroupBindingTarget>(&target))
        {
            const auto binding = group->binding != nullptr
                ? *group->binding
                : Clusters::Binding::TableEntry::ForGroup(group->fabricIndex, group->groupId, kInvalidEndpointId, GetClusterId());
            CHIP_ERROR error   = CHIP_ERROR_INVALID_ARGUMENT;
            if constexpr (!CommandType::MustUseTimedInvoke())
            {
                error =
                    Controller::InvokeGroupCommandRequest(&group->exchangeManager, group->fabricIndex, group->groupId, mCommand);
            }
            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Binding group command failed: %" CHIP_ERROR_FORMAT, error.Format());
            }
            callback(binding, error);
        }
    }

private:
    CommandType mCommand;
    BindingCommandOptions mOptions;
};

} // namespace app
} // namespace chip
