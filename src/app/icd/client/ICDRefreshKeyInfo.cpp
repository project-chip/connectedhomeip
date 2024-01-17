/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ICDRefreshKeyInfo.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandPathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelTimeout.h>
#include <app/OperationalSessionSetup.h>

namespace chip {
namespace app {

ICDRefreshKeyInfo::ICDRefreshKeyInfo(CheckInDelegate * apCheckInDelegate, ICDClientInfo aICDClientInfo) :
    mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
{
    mpCheckInDelegate = apCheckInDelegate;
    mICDClientInfo    = aICDClientInfo;
}

CHIP_ERROR ICDRefreshKeyInfo::RegisterClientWithNewKey(Messaging::ExchangeManager & exchangeMgr,
                                                       const SessionHandle & sessionHandle)
{
    using namespace Clusters::IcdManagement;

    mRegisterCommandSender.SetExchangeManager(&exchangeMgr);

    auto commandPathParams = CommandPathParams(0, 0, Id, Commands::RegisterClient::Id, (CommandPathFlags::kEndpointIdValid));

    ReturnErrorOnFailure(mRegisterCommandSender.PrepareCommand(commandPathParams));

    chip::TLV::TLVWriter * writer = mRegisterCommandSender.GetCommandDataIBTLVWriter();

    ReturnErrorOnFailure(
        writer->Put(chip::TLV::ContextTag(Commands::RegisterClient::Fields::kCheckInNodeID), mICDClientInfo.peer_node.GetNodeId()));
    ReturnErrorOnFailure(
        writer->Put(chip::TLV::ContextTag(Commands::RegisterClient::Fields::kMonitoredSubject), mICDClientInfo.monitored_subject));
    ReturnErrorOnFailure(writer->Put(chip::TLV::ContextTag(Commands::RegisterClient::Fields::kKey), mNewKey.Span()));

    // TODO : We don't have plain data for the old key

    ReturnErrorOnFailure(mRegisterCommandSender.FinishCommand());
    ReturnErrorOnFailure(mRegisterCommandSender.SendCommandRequest(sessionHandle));

    return CHIP_NO_ERROR;
}

void ICDRefreshKeyInfo::EstablishSessionToPeer()
{
    ChipLogProgress(ICD, "Trying to establish a CASE session for re-registering an ICD client");
    auto * caseSessionManager = InteractionModelEngine::GetInstance()->GetCASESessionManager();
    VerifyOrReturn(caseSessionManager != nullptr);
    caseSessionManager->FindOrEstablishSession(mICDClientInfo.peer_node, &mOnConnectedCallback, &mOnConnectionFailureCallback);
}

void ICDRefreshKeyInfo::HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                              const SessionHandle & sessionHandle)
{
    ICDRefreshKeyInfo * const _this = static_cast<ICDRefreshKeyInfo *>(context);
    VerifyOrDie(_this != nullptr);

    if (CHIP_NO_ERROR != _this->RegisterClientWithNewKey(exchangeMgr, sessionHandle))
    {
        ChipLogError(ICD, "Failed to send register client command");
    }
}

void ICDRefreshKeyInfo::HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ICDRefreshKeyInfo * const _this = static_cast<ICDRefreshKeyInfo *>(context);
    VerifyOrDie(_this != nullptr);

    ChipLogError(ICD, "Failed to establish CASE for re-registration with error '%" CHIP_ERROR_FORMAT "'", err.Format());
}
} // namespace app
} // namespace chip
