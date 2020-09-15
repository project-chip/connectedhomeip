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
 *      This file implements the CHIP Connection object that maintains a TCP connection.
 */
#include <transport/TCP.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/MessageHeader.h>

#include <inttypes.h>

namespace chip {
namespace Transport {

constexpr int kListenBacklogSize = 2;

TCP::~TCP()
{
    if (mListenSocket != nullptr)
    {
        // Udp endpoint is only non null if udp endpoint is initialized and listening
        mListenSocket->Close();
        mListenSocket->Free();
        mListenSocket = nullptr;
    }
}

CHIP_ERROR TCP::Init(TcpListenParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);

    err = params.GetInetLayer()->NewTCPEndPoint(&mListenSocket);
    SuccessOrExit(err);

    err = mListenSocket->Bind(params.GetAddressType(), IPAddress::Any, params.GetListenPort(), params.GetInterfaceId());
    SuccessOrExit(err);

    err = mListenSocket->Listen(kListenBacklogSize);
    SuccessOrExit(err);

    mListenSocket->AppState       = reinterpret_cast<void *>(this);
    mListenSocket->OnDataReceived = OnTcpReceive;
    mEndpointType                 = params.GetAddressType();

    mState = State::kInitialized;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Failed to initialize TCP transport: %s", ErrorStr(err));
        if (mListenSocket)
        {
            mListenSocket->Free();
            mListenSocket = nullptr;
        }
    }

    return err;
}

CHIP_ERROR TCP::SendMessage(const MessageHeader & header, const Transport::PeerAddress & address, System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // FIXME: implement
    err = CHIP_ERROR_NOT_IMPLEMENTED;

    /*
    const size_t headerSize = header.EncodeSizeBytes();
    size_t actualEncodedHeaderSize;

    VerifyOrExit(address.GetTransportType() == Type::kUdp, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mUDPEndPoint != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    IPPacketInfo addrInfo;
    addrInfo.Clear();

    addrInfo.DestAddress = address.GetIPAddress();
    addrInfo.DestPort    = address.GetPort();

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = header.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize);
    SuccessOrExit(err);

    // This is unexpected and means header changed while encoding
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

    err    = mUDPEndPoint->SendMsg(&addrInfo, msgBuf);
    msgBuf = nullptr;
    SuccessOrExit(err);

exit:
    if (msgBuf != NULL)
    {
        System::PacketBuffer::Free(msgBuf);
        msgBuf = NULL;
    }
    */

    return err;
}

void TCP::OnTcpReceive(Inet::TCPEndPoint * endPoint, System::PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    /*
    TCP * tc               = reinterpret_cast<TCP *>(endPoint->AppState);
    size_t headerSize       = 0;
    PeerAddress peerAddress = PeerAddress::UDP(pktInfo->SrcAddress, pktInfo->SrcPort);

    MessageHeader header;
    err = header.Decode(buffer->Start(), buffer->DataLength(), &headerSize);
    SuccessOrExit(err);

    buffer->ConsumeHead(headerSize);
    udp->HandleMessageReceived(header, peerAddress, buffer);
    */

    err = CHIP_ERROR_NOT_IMPLEMENTED;

    // exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to receive TCP message: %s", ErrorStr(err));
    }
}

void TCP::Disconnect(const PeerAddress & address)
{
    // FIXME: implement
}

} // namespace Transport
} // namespace chip
