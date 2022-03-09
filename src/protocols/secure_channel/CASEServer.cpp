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
                                                     Ble::BleLayer * bleLayer, SessionManager * sessionManager,
                                                     FabricTable * fabrics)
{
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(fabrics != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mBleLayer        = bleLayer;
    mSessionManager  = sessionManager;
    mFabrics         = fabrics;
    mExchangeManager = exchangeManager;

    Cleanup();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::InitCASEHandshake(Messaging::ExchangeContext * ec)
{
    ReturnErrorCodeIf(ec == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Mark any PASE sessions used for commissioning as stale.
    // This is a workaround, as we currently don't have a way to identify
    // secure sessions established via PASE protocol.
    // TODO - Identify which PASE base secure channel was used
    //        for commissioning and drop it once commissioning is complete.
    mSessionManager->ExpireAllPairings(kUndefinedNodeId, kUndefinedFabricIndex);

#if CONFIG_NETWORK_LAYER_BLE
    // Close all BLE connections now since a CASE handshake has been initiated.
    if (mBleLayer != nullptr)
    {
        ChipLogProgress(Discovery, "CASE handshake initiated, closing all BLE Connections");
        mBleLayer->CloseAllBleConnections();
    }
#endif

    ReturnErrorOnFailure(mSessionIDAllocator.Allocate(mSessionKeyId));

    // Setup CASE state machine using the credentials for the current fabric.
    ReturnErrorOnFailure(GetSession().ListenForSessionEstablishment(
        mSessionKeyId, mFabrics, this, Optional<ReliableMessageProtocolConfig>::Value(GetLocalMRPConfig())));

    // Hand over the exchange context to the CASE session.
    ec->SetDelegate(&GetSession());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                         System::PacketBufferHandle && payload)
{
    ChipLogProgress(Inet, "CASE Server received Sigma1 message. Starting handshake. EC %p", ec);
    CHIP_ERROR err = InitCASEHandshake(ec);
    SuccessOrExit(err);

    // TODO - Enable multiple concurrent CASE session establishment
    // https://github.com/project-chip/connectedhomeip/issues/8342
    ChipLogProgress(Inet, "CASE Server disabling CASE session setups");
    mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1);

    err = GetSession().OnMessageReceived(ec, payloadHeader, std::move(payload));
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();
    }
    return err;
}

void CASEServer::Cleanup()
{
    // Let's re-register for CASE Sigma1 message, so that the next CASE session setup request can be processed.
    // https://github.com/project-chip/connectedhomeip/issues/8342
    ChipLogProgress(Inet, "CASE Server enabling CASE session setups");
    mExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1, this);

    GetSession().Clear();
}

void CASEServer::OnSessionEstablishmentError(CHIP_ERROR err)
{
    ChipLogProgress(Inet, "CASE Session establishment failed: %s", ErrorStr(err));
    mSessionIDAllocator.Free(mSessionKeyId);
    Cleanup();
}

void CASEServer::OnSessionEstablished()
{
    ChipLogProgress(Inet, "CASE Session established. Setting up the secure channel.");
    mSessionManager->ExpireAllPairings(GetSession().GetPeerNodeId(), GetSession().GetFabricIndex());

    SessionHolder sessionHolder;
    CHIP_ERROR err = mSessionManager->NewPairing(
        sessionHolder, Optional<Transport::PeerAddress>::Value(GetSession().GetPeerAddress()), GetSession().GetPeerNodeId(),
        &GetSession(), CryptoContext::SessionRole::kResponder, GetSession().GetFabricIndex());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogProgress(Inet, "CASE secure channel is available now.");
    Cleanup();
}
} // namespace chip
