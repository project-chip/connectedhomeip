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

#include "RefreshKeySender.h"
#include "CheckInDelegate.h"
#include "controller/InvokeInteraction.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AppConfig.h>
#include <app/CommandPathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/OperationalSessionSetup.h>
#include <memory>

namespace chip {
namespace app {

RefreshKeySender::RefreshKeySender(CheckInDelegate * checkInDelegate, const ICDClientInfo & icdClientInfo,
                                   ICDClientStorage * icdClientStorage, InteractionModelEngine * engine,
                                   const RefreshKeyBuffer & refreshKeyBuffer) :
    mpCheckInDelegate(checkInDelegate),
    mICDClientInfo(icdClientInfo), mpICDClientStorage(icdClientStorage), mpImEngine(engine), mNewKey(refreshKeyBuffer),
    mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
{}

const ICDClientInfo & RefreshKeySender::GetICDClientInfo()
{
    return mICDClientInfo;
}

CHIP_ERROR RefreshKeySender::RegisterClientWithNewKey(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    auto onSuccess = [&](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(ICD, "RegisterClient command succeeded");
        CHIP_ERROR error;

        // Update the ICDClientInfo with new key and start counter and store it to persistence
        mICDClientInfo.start_icd_counter = dataResponse.ICDCounter;
        mICDClientInfo.offset            = 0;
        mpICDClientStorage->RemoveKey(mICDClientInfo);
        error = mpICDClientStorage->SetKey(mICDClientInfo, mNewKey.Span());
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(ICD, "Failed to set the new key after re-registration: %" CHIP_ERROR_FORMAT, error.Format());
            mpCheckInDelegate->OnKeyRefreshDone(this, error);
            return;
        }

        error = mpICDClientStorage->StoreEntry(mICDClientInfo);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(ICD, "Failed to store the new key after re-registration: %" CHIP_ERROR_FORMAT, error.Format());
            mpCheckInDelegate->OnKeyRefreshDone(this, error);
            return;
        }

        mpCheckInDelegate->OnCheckInComplete(mICDClientInfo);
#if CHIP_CONFIG_ENABLE_READ_CLIENT
        mpImEngine->OnActiveModeNotification(mICDClientInfo.peer_node);
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT
        mpCheckInDelegate->OnKeyRefreshDone(this, CHIP_NO_ERROR);
    };

    auto onFailure = [&](CHIP_ERROR error) {
        ChipLogError(ICD, "RegisterClient command failed: %" CHIP_ERROR_FORMAT, error.Format());
        mpCheckInDelegate->OnKeyRefreshDone(this, error);
    };

    EndpointId endpointId = 0;

    Clusters::IcdManagement::Commands::RegisterClient::Type registerClientCommand;
    registerClientCommand.checkInNodeID    = mICDClientInfo.check_in_node.GetNodeId();
    registerClientCommand.monitoredSubject = mICDClientInfo.monitored_subject;
    registerClientCommand.key              = mNewKey.Span();
    registerClientCommand.clientType       = mICDClientInfo.client_type;
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, endpointId, registerClientCommand, onSuccess, onFailure);
}

CHIP_ERROR RefreshKeySender::EstablishSessionToPeer()
{
    ChipLogProgress(ICD, "Trying to establish a CASE session for re-registering an ICD client");
    auto * caseSessionManager = mpImEngine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionManager != nullptr, CHIP_ERROR_INVALID_CASE_PARAMETER);
    caseSessionManager->FindOrEstablishSession(mICDClientInfo.peer_node, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

void RefreshKeySender::HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                             const SessionHandle & sessionHandle)
{
    RefreshKeySender * const _this = static_cast<RefreshKeySender *>(context);
    VerifyOrDie(_this != nullptr);

    CHIP_ERROR err = _this->RegisterClientWithNewKey(exchangeMgr, sessionHandle);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(ICD, "Failed to send register client command");
        _this->mpCheckInDelegate->OnKeyRefreshDone(_this, err);
    }
}

void RefreshKeySender::HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    RefreshKeySender * const _this = static_cast<RefreshKeySender *>(context);
    VerifyOrDie(_this != nullptr);

    ChipLogError(ICD, "Failed to establish CASE for re-registration with error '%" CHIP_ERROR_FORMAT "'", err.Format());
    _this->mpCheckInDelegate->OnKeyRefreshDone(_this, err);
}
} // namespace app
} // namespace chip
