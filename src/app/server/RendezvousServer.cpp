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

#include <app/server/RendezvousServer.h>

#include <app/server/StorablePeerConnection.h>
#include <core/CHIPError.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <transport/SecureSessionMgr.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif
#include <mdns/Advertiser.h>

using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

namespace chip {
static constexpr uint32_t kSpake2p_Iteration_Count = 100;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Salt";

CHIP_ERROR RendezvousServer::WaitForPairing(const RendezvousParameters & params, Messaging::ExchangeManager * exchangeManager,
                                            TransportMgrBase * transportMgr, SecureSessionMgr * sessionMgr,
                                            Transport::AdminPairingInfo * admin)
{
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(admin != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.HasSetupPINCode() || params.HasPASEVerifier(), CHIP_ERROR_INVALID_ARGUMENT);

#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(params.HasAdvertisementDelegate(), CHIP_ERROR_INVALID_ARGUMENT);
#endif

    mAdvDelegate = params.GetAdvertisementDelegate();

    // Note: Since BLE is only used for initial setup, enable BLE advertisement in rendezvous session can be expected.
    if (params.GetPeerAddress().GetTransportType() == Transport::Type::kBle)
#if CONFIG_NETWORK_LAYER_BLE
    {
        ReturnErrorOnFailure(GetAdvertisementDelegate()->StartAdvertisement());
    }
#else
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
#endif
    mSessionMgr      = sessionMgr;
    mAdmin           = admin;
    mExchangeManager = exchangeManager;

    ReturnErrorOnFailure(mExchangeManager->RegisterUnsolicitedMessageHandlerForType(
        Protocols::SecureChannel::MsgType::PBKDFParamRequest, &mPairingSession));

    if (params.HasPASEVerifier())
    {
        ReturnErrorOnFailure(mPairingSession.WaitForPairing(params.GetPASEVerifier(), mNextKeyId++, this));
    }
    else
    {
        ReturnErrorOnFailure(mPairingSession.WaitForPairing(params.GetSetupPINCode(), kSpake2p_Iteration_Count,
                                                            reinterpret_cast<const unsigned char *>(kSpake2pKeyExchangeSalt),
                                                            strlen(kSpake2pKeyExchangeSalt), mNextKeyId++, this));
    }

    return CHIP_NO_ERROR;
}

void RendezvousServer::Cleanup()
{
    mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::PBKDFParamRequest);

    if (HasAdvertisementDelegate())
    {
        GetAdvertisementDelegate()->StopAdvertisement();
    }
}

void RendezvousServer::OnSessionEstablishmentError(CHIP_ERROR err)
{
    Cleanup();

    ChipLogProgress(AppServer, "OnSessionEstablishmentError: %s", ErrorStr(err));
    ChipLogProgress(AppServer, "Failed in SPAKE2+ handshake");

    if (mDelegate != nullptr)
    {
        mDelegate->OnRendezvousStopped();
    }
}

void RendezvousServer::OnSessionEstablished()
{
    CHIP_ERROR err =
        mSessionMgr->NewPairing(Optional<Transport::PeerAddress>::Value(mPairingSession.PeerConnection().GetPeerAddress()),
                                mPairingSession.PeerConnection().GetPeerNodeId(), &mPairingSession,
                                SecureSession::SessionRole::kResponder, mAdmin->GetAdminId(), nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogProgress(AppServer, "Device completed SPAKE2+ handshake");
    if (mDelegate != nullptr)
    {
        mDelegate->OnRendezvousStarted();
    }

    Cleanup();

    ChipLogProgress(AppServer, "Device completed Rendezvous process");
    StorablePeerConnection connection(mPairingSession, mAdmin->GetAdminId());

    VerifyOrReturn(mStorage != nullptr,
                   ChipLogError(AppServer, "Storage delegate is not available. Cannot store the connection state"));
    VerifyOrReturn(connection.StoreIntoKVS(*mStorage) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to store the connection state"));

    mStorage->SyncSetKeyValue(kStorablePeerConnectionCountKey, &mNextKeyId, sizeof(mNextKeyId));
}
} // namespace chip
