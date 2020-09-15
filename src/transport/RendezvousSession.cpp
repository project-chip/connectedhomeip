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

#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <transport/BLE.h>
#include <transport/RendezvousSession.h>

static constexpr uint32_t kSpake2p_Iteration_Count = 50000;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Exchange Salt";

namespace chip {

CHIP_ERROR RendezvousSession::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mParams.HasLocalNodeId(), err = CHIP_ERROR_INCORRECT_STATE);

    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#if CONFIG_NETWORK_LAYER_BLE
    {
        Transport::BLE * transport = new Transport::BLE();
        err                        = transport->Init(this, mParams);
        mTransport                 = transport;
        mTransport->Retain();
        transport->Release();
    }
#endif // CONFIG_NETWORK_LAYER_BLE
    SuccessOrExit(err);

    if (mParams.HasDiscriminator() == false)
    {
        err = WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
    }

exit:
    return err;
}

RendezvousSession::~RendezvousSession()
{
    if (mTransport)
    {
        mTransport->Release();
        mTransport = nullptr;
    }

    mDelegate = nullptr;
}

CHIP_ERROR RendezvousSession::SendMessage(System::PacketBuffer * buffer)
{
    // Rendezvous does not use a MessageHeader yet, but the Transport::Base API expects one, so
    // let build an empty one for now.
    MessageHeader header;
    CHIP_ERROR err = mTransport->SendMessage(header, Transport::PeerAddress::BLE(), buffer);

    return err;
}

void RendezvousSession::OnPairingError(CHIP_ERROR err)
{
    mPairingInProgress = false;
    mDelegate->OnRendezvousError(err);
}

void RendezvousSession::OnPairingComplete()
{
    mPairingInProgress = false;
    mDelegate->OnRendezvousConnectionOpened();
}

void RendezvousSession::OnRendezvousConnectionOpened()
{
    if (mParams.HasDiscriminator())
    {
        CHIP_ERROR err = Pair(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
        if (err != CHIP_NO_ERROR)
        {
            OnPairingError(err);
        }
    }
}

void RendezvousSession::OnRendezvousConnectionClosed()
{
    mDelegate->OnRendezvousConnectionClosed();

    if (!mParams.HasDiscriminator())
    {
        CHIP_ERROR err = WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
        if (err != CHIP_NO_ERROR)
        {
            OnPairingError(err);
        }
    }
}

void RendezvousSession::OnRendezvousError(CHIP_ERROR err)
{
    mDelegate->OnRendezvousError(err);
}

void RendezvousSession::OnRendezvousMessageReceived(PacketBuffer * buffer)
{
    if (mPairingInProgress)
    {
        MessageHeader header;
        size_t headerSize = 0;
        header.Decode(buffer->Start(), buffer->DataLength(), &headerSize);

        buffer->ConsumeHead(headerSize);
        mPairingSession.HandlePeerMessage(header, buffer);
    }
    else
    {
        mDelegate->OnRendezvousMessageReceived(buffer);
    }
}

CHIP_ERROR RendezvousSession::WaitForPairing(Optional<NodeId> nodeId, uint32_t setupPINCode)
{
    mPairingInProgress = true;
    return mPairingSession.WaitForPairing(setupPINCode, kSpake2p_Iteration_Count, (const unsigned char *) kSpake2pKeyExchangeSalt,
                                          strlen(kSpake2pKeyExchangeSalt), nodeId, 0, this);
}

CHIP_ERROR RendezvousSession::Pair(Optional<NodeId> nodeId, uint32_t setupPINCode)
{
    mPairingInProgress = true;
    return mPairingSession.Pair(setupPINCode, kSpake2p_Iteration_Count, (const unsigned char *) kSpake2pKeyExchangeSalt,
                                strlen(kSpake2pKeyExchangeSalt), nodeId, mNextKeyId++, this);
}

} // namespace chip
