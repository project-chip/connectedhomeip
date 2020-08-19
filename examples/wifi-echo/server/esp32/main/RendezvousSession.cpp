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

#include "RendezvousSession.h"
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

using namespace ::chip;

extern CHIP_ERROR SetWiFiStationProvisioning(char * ssid, char * key);

BluetoothWidget * RendezvousSession::mVirtualLed;

Ble::BLEEndPoint * RendezvousSession::mEndPoint = nullptr;

bool RendezvousSession::mPairingInProgress = false;
SecurePairingSession RendezvousSession::mPairing;

static constexpr uint32_t kSpake2p_Iteration_Count = 50000;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Exchange Salt";

extern void PairingComplete(Optional<NodeId> peerNodeId, uint16_t peerKeyId, uint16_t localKeyId, SecurePairingSession * pairing);

RendezvousSession::RendezvousSession(BluetoothWidget * virtualLed, uint32_t setUpPINCode, NodeId myNodeId)
{
    mVirtualLed = virtualLed;

    DeviceLayer::ConnectivityMgr().AddCHIPoBLEConnectionHandler(HandleConnectionOpened);

    RendezvousSession::mPairing.WaitForPairing(setUpPINCode, kSpake2p_Iteration_Count,
                                               (const unsigned char *) kSpake2pKeyExchangeSalt, strlen(kSpake2pKeyExchangeSalt),
                                               Optional<NodeId>::Value(myNodeId), 0, this);
    RendezvousSession::mPairingInProgress = true;
    mSetUpPINCode                         = setUpPINCode;
    mNodeId                               = myNodeId;
}

CHIP_ERROR RendezvousSession::OnNewMessageForPeer(System::PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mEndPoint, err = CHIP_ERROR_INCORRECT_STATE);
    err = mEndPoint->Send(buffer);

exit:
    return err;
}

void RendezvousSession::OnPairingError(CHIP_ERROR error)
{
    ChipLogError(Ble, "RendezvousSession: failed in pairing");
    mPaired = false;
    RendezvousSession::mPairing.WaitForPairing(mSetUpPINCode, kSpake2p_Iteration_Count,
                                               (const unsigned char *) kSpake2pKeyExchangeSalt, strlen(kSpake2pKeyExchangeSalt),
                                               Optional<NodeId>::Value(mNodeId), 0, this);
    RendezvousSession::mPairingInProgress = true;
}

void RendezvousSession::OnPairingComplete(Optional<NodeId> peerNodeId, uint16_t peerKeyId, uint16_t localKeyId)
{
    ChipLogProgress(Ble, "RendezvousSession: pairing complete");
    mPaired                               = true;
    RendezvousSession::mPairingInProgress = false;
    PairingComplete(peerNodeId, peerKeyId, localKeyId, &RendezvousSession::mPairing);
}

CHIP_ERROR RendezvousSession::Send(const char * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBuffer * buffer;
    const size_t msgLen = strlen(msg);

    VerifyOrExit(mEndPoint, err = CHIP_ERROR_INCORRECT_STATE);

    buffer = System::PacketBuffer::NewWithAvailableSize(msgLen);
    memcpy(buffer->Start(), msg, msgLen);
    buffer->SetDataLength(msgLen);

    err = mEndPoint->Send(buffer);

exit:
    return err;
}

void RendezvousSession::HandleConnectionOpened(Ble::BLEEndPoint * endPoint)
{
    ChipLogProgress(Ble, "RendezvousSession: Connection opened");

    mEndPoint                     = endPoint;
    mEndPoint->OnMessageReceived  = HandleMessageReceived;
    mEndPoint->OnConnectionClosed = HandleConnectionClosed;
    mVirtualLed->Set(true);
}

void RendezvousSession::HandleConnectionClosed(Ble::BLEEndPoint * endPoint, BLE_ERROR err)
{
    ChipLogProgress(Ble, "RendezvousSession: Connection closed (%s)", ErrorStr(err));

    mEndPoint = nullptr;
    mVirtualLed->Set(false);
}

void RendezvousSession::HandleMessageReceived(Ble::BLEEndPoint * endPoint, PacketBuffer * buffer)
{
    if (RendezvousSession::mPairingInProgress)
    {
        MessageHeader header;
        size_t headerSize = 0;

        CHIP_ERROR err = header.Decode(buffer->Start(), buffer->DataLength(), &headerSize);
        SuccessOrExit(err);

        buffer->ConsumeHead(headerSize);
        RendezvousSession::mPairing.HandlePeerMessage(header, buffer);
    }
    else
    {
        const size_t bufferLen = buffer->DataLength();
        char msg[bufferLen];
        msg[bufferLen] = 0;
        memcpy(msg, buffer->Start(), bufferLen);

        ChipLogProgress(Ble, "RendezvousSession: Receive message: %s", msg);

        if ((bufferLen > 3) && (msg[0] == msg[1]) && (msg[0] == msg[bufferLen - 1]))
        {
            // WiFi credentials, of the form ‘::SSID:password:’, where ‘:’ can be any single ASCII character.
            msg[1]      = 0;
            char * ssid = strtok(&msg[2], msg);
            char * key  = strtok(NULL, msg);
            if (ssid && key)
            {
                ChipLogProgress(Ble, "RendezvousSession: SSID: %s, key: %s", ssid, key);
                SetWiFiStationProvisioning(ssid, key);
            }
            else
            {
                ChipLogError(Ble, "RendezvousSession: SSID: %p, key: %p", ssid, key);
            }
        }
        else
        {
            // Echo.
            mEndPoint->Send(buffer);
        }
    }

exit:
    return;
}
