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

#include <core/CHIPEncoding.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/MessageHeader.h>

#include <inttypes.h>

namespace chip {
namespace Transport {

using namespace chip::Encoding;

constexpr int kListenBacklogSize = 2;

TCPBase::~TCPBase()
{
    if (mListenSocket != nullptr)
    {
        // endpoint is only non null if it is initialized and listening
        mListenSocket->Close();
        mListenSocket->Free();
        mListenSocket = nullptr;
    }

    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i] != nullptr)
        {
            mActiveConnections[i]->Close();
            mActiveConnections[i]->Free();
            mActiveConnections[i] = nullptr;
        }
    }
}

CHIP_ERROR TCPBase::Init(TcpListenParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);

    err = params.GetInetLayer()->NewTCPEndPoint(&mListenSocket);
    SuccessOrExit(err);

    err = mListenSocket->Bind(params.GetAddressType(), IPAddress::Any, params.GetListenPort(), params.GetInterfaceId());
    SuccessOrExit(err);

    err = mListenSocket->Listen(kListenBacklogSize);
    SuccessOrExit(err);

    mListenSocket->AppState             = reinterpret_cast<void *>(this);
    mListenSocket->OnDataReceived       = OnTcpReceive;
    mListenSocket->OnConnectComplete    = OnConnectionComplete;
    mListenSocket->OnConnectionClosed   = OnConnectionClosed;
    mListenSocket->OnConnectionReceived = OnConnectionRecevied;
    mListenSocket->OnAcceptError        = OnAcceptError;
    mEndpointType                       = params.GetAddressType();

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

Inet::TCPEndPoint * TCPBase::FindActiveConnection(const PeerAddress & address)
{
    Inet::TCPEndPoint * endPoint = nullptr;

    VerifyOrExit(address.GetTransportType() == Type::kTcp, endPoint = nullptr);

    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i] == nullptr)
        {
            continue;
        }
        IPAddress addr;
        uint16_t port;
        mActiveConnections[i]->GetPeerInfo(&addr, &port);

        if ((addr == address.GetIPAddress()) && (port == address.GetPort()))
        {
            ExitNow(endPoint = mActiveConnections[i]);
        }
    }

exit:
    return endPoint;
}

CHIP_ERROR TCPBase::SendMessage(const MessageHeader & header, const Transport::PeerAddress & address, System::PacketBuffer * msgBuf)
{
    // Sent buffer data format is:
    //    - packet size as a uint16_t
    //    - header
    //    - actual data
    CHIP_ERROR err               = CHIP_NO_ERROR;
    const size_t prefixSize      = header.EncodeSizeBytes() + sizeof(uint16_t);
    Inet::TCPEndPoint * endPoint = nullptr;
    size_t actualEncodedHeaderSize;

    VerifyOrExit(address.GetTransportType() == Type::kTcp, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(prefixSize + msgBuf->DataLength() <= std::numeric_limits<uint16_t>::max(), err = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(msgBuf->EnsureReservedSize(prefixSize), err = CHIP_ERROR_NO_MEMORY);

    {
        msgBuf->SetStart(msgBuf->Start() - prefixSize);

        uint8_t * output = msgBuf->Start();
        LittleEndian::Write16(output, msgBuf->DataLength());

        err = header.Encode(output, msgBuf->DataLength(), &actualEncodedHeaderSize);
        SuccessOrExit(err);

        // header encoding has to match space that we allocated
        VerifyOrExit(prefixSize == actualEncodedHeaderSize + sizeof(uint16_t), err = CHIP_ERROR_INTERNAL);
    }

    // Reuse existing connection if one exists, otherwise a new one
    // will be established

    endPoint = FindActiveConnection(address);
    if (endPoint != nullptr)
    {
        err    = endPoint->Send(msgBuf);
        msgBuf = nullptr;
        SuccessOrExit(err);
    }
    else
    {
        // FIXME: connect
        // FIXME: implement
        err = CHIP_ERROR_NOT_IMPLEMENTED;
        ChipLogError(Inet, "%s not yet implemented", __PRETTY_FUNCTION__);
    }

exit:
    if (msgBuf != NULL)
    {
        System::PacketBuffer::Free(msgBuf);
        msgBuf = NULL;
    }

    return err;
}

void TCPBase::OnTcpReceive(Inet::TCPEndPoint * endPoint, System::PacketBuffer * buffer)
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

    ChipLogError(Inet, "%s not yet implemented", __PRETTY_FUNCTION__);

    err = CHIP_ERROR_NOT_IMPLEMENTED;

    // exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to receive TCP message: %s", ErrorStr(err));
    }
}

void TCPBase::OnConnectionComplete(Inet::TCPEndPoint * endPoint, INET_ERROR err)
{
    // FIXME: implement
    ChipLogError(Inet, "%s not yet implemented", __PRETTY_FUNCTION__);
}

void TCPBase::OnConnectionClosed(Inet::TCPEndPoint * endPoint, INET_ERROR err)
{
    // FIXME: implement
    ChipLogError(Inet, "%s not yet implemented", __PRETTY_FUNCTION__);
}

void TCPBase::OnConnectionRecevied(Inet::TCPEndPoint * listenEndPoint, Inet::TCPEndPoint * endPoint, const IPAddress & peerAddress,
                                   uint16_t peerPort)
{
    // FIXME: implement
    ChipLogError(Inet, "%s not yet implemented", __PRETTY_FUNCTION__);
}

void TCPBase::OnAcceptError(Inet::TCPEndPoint * endPoint, INET_ERROR err)
{
    // FIXME: implement
    ChipLogError(Inet, "%s not yet implemented", __PRETTY_FUNCTION__);
}

void TCPBase::Disconnect(const PeerAddress & address)
{
    // FIXME: implement
    ChipLogError(Inet, "%s not yet implemented", __PRETTY_FUNCTION__);
}

} // namespace Transport
} // namespace chip
