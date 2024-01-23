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
#include "CheckInDelegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandPathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelTimeout.h>
#include <app/OperationalSessionSetup.h>
#include <memory>

namespace chip {
namespace app {

ICDRefreshKeyInfo::ICDRefreshKeyInfo(CheckInDelegate * apCheckInDelegate, ICDClientInfo * apICDClientInfo,
                                     ICDClientStorage * aICDClientStorage) :
    mOnConnectedCallback(HandleDeviceConnected, this),
    mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
{
    mpCheckInDelegate  = apCheckInDelegate;
    mpICDClientInfo    = apICDClientInfo;
    mpICDClientStorage = aICDClientStorage;
}

void RegisterCommandSenderDelegate::OnResponse(chip::app::CommandSender * apCommandSender,
                                               const chip::app::ConcreteCommandPath & aPath, const chip::app::StatusIB & aStatus,
                                               chip::TLV::TLVReader * aData)
{
    app::Clusters::IcdManagement::Commands::RegisterClientResponse::DecodableType responseData;
    if (aData == nullptr)
    {
        mpICDRefreshKeyInfo->GetCheckInDelegate()->OnRegistrationUpdateFailure(mpICDRefreshKeyInfo.get(),
                                                                               CHIP_ERROR_INVALID_TLV_ELEMENT);
        Platform::Delete(this);
        return;
    }

    mError = DataModel::Decode(*aData, responseData);
    if (mError != CHIP_NO_ERROR)
    {
        ChipLogError(ICD, "Failed to decode re-registration response data: %" CHIP_ERROR_FORMAT, mError.Format());
        mpICDRefreshKeyInfo->GetCheckInDelegate()->OnRegistrationUpdateFailure(mpICDRefreshKeyInfo.get(), mError);
        Platform::Delete(this);
        return;
    }
    // Update the ICDClientInfo with new key and start counter and store it to persistence
    mpICDRefreshKeyInfo->GetICDClientInfo()->start_icd_counter = responseData.ICDCounter;
    mError = mpICDRefreshKeyInfo->GetICDClientStorage()->SetKey(*mpICDRefreshKeyInfo->GetICDClientInfo(),
                                                                mpICDRefreshKeyInfo->mNewKey.Span());
    if (mError != CHIP_NO_ERROR)
    {
        ChipLogError(ICD, "Failed to store the new key after re-registration: %" CHIP_ERROR_FORMAT, mError.Format());
        Platform::Delete(this);
    }
    mpICDRefreshKeyInfo->GetCheckInDelegate()->OnRegistrationUpdateComplete(mpICDRefreshKeyInfo->GetICDClientInfo(),
                                                                            mpICDRefreshKeyInfo.get());
    Platform::Delete(this);
}
void RegisterCommandSenderDelegate::OnError(const chip::app::CommandSender * apCommandSender, CHIP_ERROR aError)
{
    mError = aError;
    mpICDRefreshKeyInfo->GetCheckInDelegate()->OnRegistrationUpdateFailure(mpICDRefreshKeyInfo.get(), mError);
    Platform::Delete(this);
}

void RegisterCommandSenderDelegate::OnDone(chip::app::CommandSender * apCommandSender) {}

void RegisterCommandSenderDelegate::OnFailure(CHIP_ERROR aError)
{
    mError = aError;
    mpICDRefreshKeyInfo->GetCheckInDelegate()->OnRegistrationUpdateFailure(mpICDRefreshKeyInfo.get(), mError);
    Platform::Delete(this);
}

CHIP_ERROR ICDRefreshKeyInfo::RegisterClientWithNewKey(Messaging::ExchangeManager & exchangeMgr,
                                                       const SessionHandle & sessionHandle)
{
    using namespace Clusters::IcdManagement;
    EndpointId endpointId = 0;
    GroupId groupId       = 0;

    mRegisterCommandSender.Emplace(mpCommandSenderDelegate, &exchangeMgr);

    Commands::RegisterClient::Type registerClientCommand;
    registerClientCommand.checkInNodeID    = mpICDClientInfo->peer_node.GetNodeId();
    registerClientCommand.monitoredSubject = mpICDClientInfo->monitored_subject;
    registerClientCommand.key              = mNewKey.Span();
    auto commandPathParams =
        CommandPathParams(endpointId, groupId, Id, Commands::RegisterClient::Id, (CommandPathFlags::kEndpointIdValid));

    ReturnErrorOnFailure(mRegisterCommandSender.Value().AddRequestData(commandPathParams, registerClientCommand));
    ReturnErrorOnFailure(mRegisterCommandSender.Value().SendCommandRequest(sessionHandle));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDRefreshKeyInfo::EstablishSessionToPeer()
{
    ChipLogProgress(ICD, "Trying to establish a CASE session for re-registering an ICD client");
    auto * caseSessionManager = InteractionModelEngine::GetInstance()->GetCASESessionManager();
    VerifyOrReturnError(caseSessionManager != nullptr, CHIP_ERROR_INVALID_CASE_PARAMETER);
    caseSessionManager->FindOrEstablishSession(mpICDClientInfo->peer_node, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

void ICDRefreshKeyInfo::HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                              const SessionHandle & sessionHandle)
{
    ICDRefreshKeyInfo * const _this = static_cast<ICDRefreshKeyInfo *>(context);
    VerifyOrDie(_this != nullptr);

    CHIP_ERROR err = _this->RegisterClientWithNewKey(exchangeMgr, sessionHandle);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(ICD, "Failed to send register client command");
        _this->GetCommandSenderDelegate()->OnFailure(err);
    }
}

void ICDRefreshKeyInfo::HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ICDRefreshKeyInfo * const _this = static_cast<ICDRefreshKeyInfo *>(context);
    VerifyOrDie(_this != nullptr);

    ChipLogError(ICD, "Failed to establish CASE for re-registration with error '%" CHIP_ERROR_FORMAT "'", err.Format());
    _this->GetCommandSenderDelegate()->OnFailure(err);
}
} // namespace app
} // namespace chip
