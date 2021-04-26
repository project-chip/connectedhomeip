/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <inttypes.h>

#include <protocols/secure_channel/RendezvousSession.h>

#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>

static constexpr uint32_t kSpake2p_Iteration_Count = 100;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Salt";

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Transport;

namespace chip {

CHIP_ERROR RendezvousSession::Init(const RendezvousParameters & params, Messaging::ExchangeManager * exchangeManager,
                                   TransportMgrBase * transportMgr, SecureSessionMgr * sessionMgr,
                                   Transport::AdminPairingInfo * admin)
{
    mParams       = params;
    mTransportMgr = transportMgr;
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(sessionMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(admin != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mParams.HasSetupPINCode() || mParams.HasPASEVerifier(), CHIP_ERROR_INVALID_ARGUMENT);
#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(mParams.HasAdvertisementDelegate(), CHIP_ERROR_INVALID_ARGUMENT);
#endif

    mSecureSessionMgr = sessionMgr;
    mAdmin            = admin;
    mExchangeManager  = exchangeManager;

    // Note: Since BLE is only used for initial setup, enable BLE advertisement in rendezvous session can be expected.
    if (params.GetPeerAddress().GetTransportType() == Transport::Type::kBle)
#if CONFIG_NETWORK_LAYER_BLE
    {
        ReturnErrorOnFailure(mParams.GetAdvertisementDelegate()->StartAdvertisement());
    }
#else
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    if (!mParams.IsController())
    {
        ReturnErrorOnFailure(mExchangeManager->RegisterUnsolicitedMessageHandlerForType(
            Protocols::SecureChannel::MsgType::PBKDFParamRequest, &mPairingSession));

        if (mParams.HasPASEVerifier())
        {
            ReturnErrorOnFailure(WaitForPairing(mParams.GetPASEVerifier()));
        }
        else
        {
            ReturnErrorOnFailure(WaitForPairing(mParams.GetSetupPINCode()));
        }
    }

    // TODO: We should assume mTransportMgr not null for IP rendezvous.
    if (mTransportMgr != nullptr)
    {
        ReturnErrorOnFailure(mPairingSession.MessageDispatch().Init(mTransportMgr));
        mPairingSession.MessageDispatch().SetPeerAddress(mParams.GetPeerAddress());
    }

    return CHIP_NO_ERROR;
}

RendezvousSession::~RendezvousSession()
{
    mDelegate = nullptr;
}

void RendezvousSession::OnSessionEstablished()
{
    SecureSessionMgr::PairingDirection direction = SecureSessionMgr::PairingDirection::kInitiator;
    if (!mParams.IsController())
    {
        direction = SecureSessionMgr::PairingDirection::kResponder;
    }

    // TODO: Once Operational credentials are implemented, node id assignment should be done during opcreds configuration.
    // - can use internal node ids (0xFFFF_FFFE_xxxx_xxx - spec still being defined) if a temporary
    //   node id is required for indexing
    // - should only assign a final node id as part of setting operational credentials
    if (!mParams.GetRemoteNodeId().HasValue())
    {
        ChipLogError(Ble, "Missing node id in rendezvous parameters. Node ID is required until opcerts are implemented");
    }

    mPairingSession.PeerConnection().SetPeerNodeId(mParams.GetRemoteNodeId().ValueOr(kUndefinedNodeId));

    CHIP_ERROR err = mSecureSessionMgr->NewPairing(
        Optional<Transport::PeerAddress>::Value(mPairingSession.PeerConnection().GetPeerAddress()),
        mPairingSession.PeerConnection().GetPeerNodeId(), &mPairingSession, direction, mAdmin->GetAdminId(), nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnRendezvousError(err);
        return;
    }

    Cleanup();

    if (mParams.HasAdvertisementDelegate())
    {
        mParams.GetAdvertisementDelegate()->RendezvousComplete();
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::SecurePairingSuccess, CHIP_NO_ERROR);
        mDelegate->OnRendezvousComplete();
    }
}

void RendezvousSession::Cleanup()
{
    if (!mParams.IsController())
    {
        mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::PBKDFParamRequest);
    }

    if (mParams.HasAdvertisementDelegate())
    {
        mParams.GetAdvertisementDelegate()->StopAdvertisement();
    }
}

void RendezvousSession::OnNetworkProvisioningError(CHIP_ERROR err) {}

void RendezvousSession::OnNetworkProvisioningComplete() {}

void RendezvousSession::OnRendezvousConnectionOpened()
{
    if (!mParams.IsController())
    {
        return;
    }

    CHIP_ERROR err = Pair(mParams.GetSetupPINCode());
    if (err != CHIP_NO_ERROR)
    {
        OnRendezvousError(err);
    }
}

void RendezvousSession::OnRendezvousConnectionClosed() {}

void RendezvousSession::OnRendezvousError(CHIP_ERROR err)
{
    Cleanup();

    if (mDelegate != nullptr)
    {
        mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::SecurePairingFailed, err);
        mDelegate->OnRendezvousError(err);
    }
}

void RendezvousSession::OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                                          System::PacketBufferHandle msgBuf)
{}

CHIP_ERROR RendezvousSession::WaitForPairing(uint32_t setupPINCode)
{
    return mPairingSession.WaitForPairing(setupPINCode, kSpake2p_Iteration_Count,
                                          reinterpret_cast<const unsigned char *>(kSpake2pKeyExchangeSalt),
                                          strlen(kSpake2pKeyExchangeSalt), mNextKeyId++, this);
}

CHIP_ERROR RendezvousSession::WaitForPairing(const PASEVerifier & verifier)
{
    return mPairingSession.WaitForPairing(verifier, mNextKeyId++, this);
}

CHIP_ERROR RendezvousSession::Pair(uint32_t setupPINCode)
{
    Messaging::ExchangeContext * ctxt = mExchangeManager->NewContext(SecureSessionHandle(), &mPairingSession);
    ReturnErrorCodeIf(ctxt == nullptr, CHIP_ERROR_INTERNAL);

    CHIP_ERROR err = mPairingSession.Pair(mParams.GetPeerAddress(), setupPINCode, mNextKeyId++, ctxt, this);
    return err;
}

} // namespace chip
