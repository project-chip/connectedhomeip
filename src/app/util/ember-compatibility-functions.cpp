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

/**
 *    @file
 *          Contains the functions for compatibility with ember ZCL inner state
 *          when calling ember callbacks.
 */

#include <app/util/ember-compatibility-functions.h>

#include <app/Command.h>
#include <app/util/util.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Compatibility {
namespace {
EmberAfClusterCommand imCompatibilityEmberAfCluster;
EmberApsFrame imCompatibilityEmberApsFrame;
EmberAfInterpanHeader imCompatibilityInterpanHeader;
Command * currentCommandObject;
} // namespace

void SetupEmberAfObjects(Command * command, ClusterId clusterId, CommandId commandId, EndpointId endpointId)
{
    const Messaging::ExchangeContext * commandExchangeCtx = command->GetExchangeContext();

    imCompatibilityEmberApsFrame.clusterId           = clusterId;
    imCompatibilityEmberApsFrame.destinationEndpoint = endpointId;
    imCompatibilityEmberApsFrame.sourceEndpoint      = 1; // source endpoint is fixed to 1 for now.
    imCompatibilityEmberApsFrame.sequence = (commandExchangeCtx != nullptr ? commandExchangeCtx->GetExchangeId() & 0xFF : 0);

    imCompatibilityEmberAfCluster.commandId      = commandId;
    imCompatibilityEmberAfCluster.apsFrame       = &imCompatibilityEmberApsFrame;
    imCompatibilityEmberAfCluster.interPanHeader = &imCompatibilityInterpanHeader;
    imCompatibilityEmberAfCluster.source =
        (commandExchangeCtx != nullptr ? commandExchangeCtx->GetSecureSessionHandle().GetPeerNodeId() : 0); // 0 is "Any" NodeId.

    emAfCurrentCommand   = &imCompatibilityEmberAfCluster;
    currentCommandObject = command;
}

bool IMEmberAfSendDefaultResponseWithCallback(EmberAfStatus status)
{
    if (currentCommandObject == nullptr)
    {
        // If this command is not handled by IM, then let ember send response.
        return false;
    }

    chip::app::CommandPathParams returnStatusParam = { imCompatibilityEmberApsFrame.sourceEndpoint,
                                                       0, // GroupId
                                                       imCompatibilityEmberApsFrame.clusterId,
                                                       imCompatibilityEmberAfCluster.commandId,
                                                       (chip::app::CommandPathFlags::kEndpointIdValid) };

    CHIP_ERROR err =
        currentCommandObject->AddStatusCode(&returnStatusParam, chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                            chip::Protocols::InteractionModel::Id, status);
    return CHIP_NO_ERROR == err;
}

void ResetEmberAfObjects()
{
    emAfCurrentCommand   = nullptr;
    currentCommandObject = nullptr;
}

} // namespace Compatibility
} // namespace app
} // namespace chip
