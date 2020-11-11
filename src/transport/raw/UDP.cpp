/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements the CHIP Connection object that maintains a UDP connection.
 */
#include <transport/raw/UDP.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <system/AutoFreePacketBuffer.h>
#include <transport/raw/MessageHeader.h>

#include <inttypes.h>

namespace chip {
namespace Transport {

UDP::~UDP()
{
    Close();
}

CHIP_ERROR UDP::Init(UdpListenParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mState != State::kNotReady)
    {
        Close();
    }

    err = params.GetInetLayer()->NewUDPEndPoint(&mUDPEndPoint);
    SuccessOrExit(err);

    err = mUDPEndPoint->Bind(params.GetAddressType(), Inet::IPAddress::Any, params.GetListenPort(), params.GetInterfaceId());
    SuccessOrExit(err);

    err = mUDPEndPoint->Listen();
    SuccessOrExit(err);

    mUDPEndPoint->AppState          = reinterpret_cast<void *>(this);
    mUDPEndPoint->OnMessageReceived = OnUdpReceive;
    mUDPEndpointType                = params.GetAddressType();

    mState = State::kInitialized;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Failed to initialize Udp transport: %s", ErrorStr(err));
        if (mUDPEndPoint)
        {
            mUDPEndPoint->Free();
            mUDPEndPoint = nullptr;
        }
    }

    return err;
}

void UDP::Close()
{
    if (mUDPEndPoint)
    {
        // Udp endpoint is only non null if udp endpoint is initialized and listening
        mUDPEndPoint->Close();
        mUDPEndPoint->Free();
        mUDPEndPoint = nullptr;
    }
    mState = State::kNotReady;
}

CHIP_ERROR UDP::SendMessage(const PacketHeader & header, Header::Flags payloadFlags, const Transport::PeerAddress & address,
                            System::PacketBuffer * msgIn)
{
    System::AutoFreePacketBuffer msgBuf(msgIn);
    const uint16_t headerSize = header.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize;
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(address.GetTransportType() == Type::kUdp, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mUDPEndPoint != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    Inet::IPPacketInfo addrInfo;
    addrInfo.Clear();

    addrInfo.DestAddress = address.GetIPAddress();
    addrInfo.DestPort    = address.GetPort();
    addrInfo.Interface   = address.GetInterface();

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = header.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize, payloadFlags);
    SuccessOrExit(err);

    // This is unexpected and means header changed while encoding
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

    err = mUDPEndPoint->SendMsg(&addrInfo, msgBuf.Release());
    SuccessOrExit(err);

exit:
    return err;
}

void UDP::OnUdpReceive(Inet::IPEndPointBasis * endPoint, System::PacketBuffer * buffer, const Inet::IPPacketInfo * pktInfo)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    UDP * udp               = reinterpret_cast<UDP *>(endPoint->AppState);
    uint16_t headerSize     = 0;
    PeerAddress peerAddress = PeerAddress::UDP(pktInfo->SrcAddress, pktInfo->SrcPort);

    PacketHeader header;
    err = header.Decode(buffer->Start(), buffer->DataLength(), &headerSize);
    SuccessOrExit(err);

    buffer->ConsumeHead(headerSize);
    udp->HandleMessageReceived(header, peerAddress, buffer);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to receive UDP message: %s", ErrorStr(err));
    }
}

} // namespace Transport
} // namespace chip
