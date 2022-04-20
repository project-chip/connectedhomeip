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

#include <protocols/secure_channel/CASEServer.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <transport/SessionManager.h>

using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::Credentials;

namespace chip {

CHIP_ERROR CASEServer::ListenForSessionEstablishment(Messaging::ExchangeManager * exchangeManager, TransportMgrBase * transportMgr,
                                                     SessionManager * sessionManager, FabricTable * fabrics,
                                                     SessionResumptionStorage * sessionResumptionStorage,
                                                     Credentials::GroupDataProvider * responderGroupDataProvider)
{
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(responderGroupDataProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mSessionManager           = sessionManager;
    mSessionResumptionStorage = sessionResumptionStorage;
    mFabrics                  = fabrics;
    mExchangeManager          = exchangeManager;
    mGroupDataProvider        = responderGroupDataProvider;

    ChipLogProgress(Inet, "CASE Server enabling CASE session setups");
    mExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1, this);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, Messaging::ExchangeDelegate *& newDelegate)
{
    if (mPairingSession.HasValue())
        return CHIP_ERROR_NO_MEMORY;

    CASESession * session = &mPairingSession.Emplace();

    ChipLogProgress(Inet, "CASE Server allocated pairing: %p", session);
    // Setup CASE state machine using the credentials for the current fabric.
    session->SetGroupDataProvider(mGroupDataProvider);
    ReturnErrorOnFailure(
        session->ListenForSessionEstablishment(*mSessionManager, mFabrics, mSessionResumptionStorage, this,
                                               Optional<ReliableMessageProtocolConfig>::Value(GetLocalMRPConfig())));
    newDelegate = session;

    return CHIP_NO_ERROR;
}

void CASEServer::OnExchangeCreationFailed(Messaging::ExchangeDelegate * delegate)
{
    mPairingSession.ClearValue();
}

void CASEServer::OnSessionEstablishmentError(CHIP_ERROR err)
{
    ChipLogError(Inet, "CASE Session establishment failed: %s", ErrorStr(err));
}

void CASEServer::OnSessionEstablished(const SessionHandle & session)
{
    ChipLogProgress(Inet, "CASE Session established to peer: " ChipLogFormatScopedNodeId,
                    ChipLogValueScopedNodeId(session->GetPeer()));
}

void CASEServer::OnSessionEstablishmentDone(PairingSession * pairing)
{
    CASESession * session = static_cast<CASESession *>(pairing);
    ChipLogProgress(Inet, "CASE Server releasing pairing: %p", session);
    VerifyOrDie(mPairingSession.HasValue() && session == &mPairingSession.Value());
    mPairingSession.ClearValue();
}

} // namespace chip
