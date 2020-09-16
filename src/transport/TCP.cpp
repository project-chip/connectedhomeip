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
namespace {

using namespace chip::Encoding;

// Packets start with a 16-bit size
constexpr size_t kPacketSizeBytes = 2;

constexpr int kListenBacklogSize = 2;

/**
 *  Determine if the given buffer contains a complete message
 */
bool ContainsCompleteMessage(System::PacketBuffer * buffer, uint8_t ** start, uint16_t * size)
{
    bool completeMessage = false;

    if (buffer->DataLength() >= kPacketSizeBytes)
    {
        *size           = LittleEndian::Get16(buffer->Start());
        *start          = buffer->Start() + kPacketSizeBytes;
        completeMessage = (buffer->DataLength() >= *size + kPacketSizeBytes);
    }

    if (!completeMessage)
    {
        *start = nullptr;
    }

    return completeMessage;
}

} // namespace

TCPBase::~TCPBase()
{
    if (mListenSocket != nullptr)
    {
        // endpoint is only non null if it is initialized and listening
        mListenSocket->Close();
        mListenSocket->Free();
        mListenSocket = nullptr;
    }
    CloseActiveConnections();

    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (mPendingPackets[i].packetBuffer != nullptr)
        {
            System::PacketBuffer::Free(mPendingPackets[i].packetBuffer);
            mPendingPackets[i].packetBuffer = nullptr;
        }
    }
}

bool TCPBase::HasActiveConnections() const
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i] != nullptr)
        {
            // An active connection exists
            return true;
        }
    }

    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (mPendingPackets[i].packetBuffer != nullptr)
        {
            // 'Connect' is pending
            return true;
        }
    }

    return false;
}

void TCPBase::CloseActiveConnections()
{
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
    mInterfaceId                        = params.GetInterfaceId();

    mState = State::kInitialized;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to initialize TCP transport: %s", ErrorStr(err));
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
    const size_t prefixSize      = header.EncodeSizeBytes() + kPacketSizeBytes;
    Inet::TCPEndPoint * endPoint = nullptr;
    size_t actualEncodedHeaderSize;

    VerifyOrExit(address.GetTransportType() == Type::kTcp, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(prefixSize + msgBuf->DataLength() <= std::numeric_limits<uint16_t>::max(), err = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(msgBuf->EnsureReservedSize(prefixSize), err = CHIP_ERROR_NO_MEMORY);

    {
        msgBuf->SetStart(msgBuf->Start() - prefixSize);

        uint8_t * output = msgBuf->Start();

        // Length is actual data, without considering the length bytes themselves
        LittleEndian::Write16(output, msgBuf->DataLength() - kPacketSizeBytes);

        err = header.Encode(output, msgBuf->DataLength(), &actualEncodedHeaderSize);
        SuccessOrExit(err);

        // header encoding has to match space that we allocated
        VerifyOrExit(prefixSize == actualEncodedHeaderSize + kPacketSizeBytes, err = CHIP_ERROR_INTERNAL);
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
        err    = SendAfterConnect(address, msgBuf);
        msgBuf = nullptr;
        SuccessOrExit(err);
    }

exit:
    if (msgBuf != NULL)
    {
        System::PacketBuffer::Free(msgBuf);
        msgBuf = NULL;
    }

    return err;
}

CHIP_ERROR TCPBase::SendAfterConnect(const PeerAddress & addr, System::PacketBuffer * msg)
{
    // This will initiate a connection to the specified peer
    CHIP_ERROR err         = CHIP_NO_ERROR;
    PendingPacket * packet = nullptr;
    bool alreadyConnecting = false;
    TCPEndPoint * endPoint = nullptr;

    // Iterate through the ENTIRE array. If a pending packet for
    // the address already exists, this means a connection is pending and
    // does NOT need to be re-established.
    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (mPendingPackets[i].packetBuffer == nullptr)
        {
            if (packet == nullptr)
            {
                // found a slot to store the packet into
                packet = mPendingPackets + i;
            }
        }
        else if (mPendingPackets[i].peerAddress == addr)
        {
            // same destination exists.
            alreadyConnecting = true;

            // ensure packets are ORDERED
            if (packet != nullptr)
            {
                packet->peerAddress             = addr;
                packet->packetBuffer            = mPendingPackets[i].packetBuffer;
                mPendingPackets[i].packetBuffer = nullptr;
                packet                          = mPendingPackets + i;
            }
        }
    }

    VerifyOrExit(packet != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // If already connecting, buffer was just enqueued for more sending
    VerifyOrExit(!alreadyConnecting, err = CHIP_NO_ERROR);

    // Ensures sufficient active connections size exist
    VerifyOrExit(mUsedEndPointCount < mActiveConnectionsSize, err = CHIP_ERROR_NO_MEMORY);

    err = mListenSocket->Layer().NewTCPEndPoint(&endPoint);
    SuccessOrExit(err);

    endPoint->AppState             = reinterpret_cast<void *>(this);
    endPoint->OnDataReceived       = OnTcpReceive;
    endPoint->OnConnectComplete    = OnConnectionComplete;
    endPoint->OnConnectionClosed   = OnConnectionClosed;
    endPoint->OnConnectionReceived = OnConnectionRecevied;
    endPoint->OnAcceptError        = OnAcceptError;

    err = endPoint->Connect(addr.GetIPAddress(), addr.GetPort(), mInterfaceId);
    SuccessOrExit(err);

    // enqueue the packet once the connection succeeds
    packet->peerAddress  = addr;
    packet->packetBuffer = msg;
    msg                  = nullptr;
    mUsedEndPointCount++;

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (msg != nullptr)
        {
            System::PacketBuffer::Free(msg);
            msg = nullptr;
        }
        if (endPoint != nullptr)
        {
            endPoint->Free();
        }
    }
    return err;
}

CHIP_ERROR TCPBase::ProcessSingleMessageFromBufferHead(const PeerAddress & peerAddress, System::PacketBuffer * buffer,
                                                       uint16_t messageSize)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint8_t * oldStart = buffer->Start();
    size_t oldLength   = buffer->DataLength();

    buffer->SetDataLength(messageSize);

    size_t headerSize = 0;

    MessageHeader header;
    err = header.Decode(buffer->Start(), buffer->DataLength(), &headerSize);
    SuccessOrExit(err);

    buffer->ConsumeHead(headerSize);
    HandleMessageReceived(header, peerAddress, buffer);

exit:
    buffer->SetStart(oldStart);
    buffer->SetDataLength(oldLength);

    return err;
}

CHIP_ERROR TCPBase::ProcessReceivedBuffer(Inet::TCPEndPoint * endPoint, const PeerAddress & peerAddress,
                                          System::PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    while (buffer != nullptr)
    {
        // when a buffer is empty, it can be released back to the app
        if (buffer->DataLength() == 0)
        {
            System::PacketBuffer * old = buffer;
            buffer                     = old->DetachTail();
            System::PacketBuffer::Free(old);
            continue;
        }

        uint8_t * messageData = nullptr;
        uint16_t messageSize  = 0;
        if (ContainsCompleteMessage(buffer, &messageData, &messageSize))
        {
            // length was read and is not needed anymore
            buffer->ConsumeHead(kPacketSizeBytes);

            // Sanity checks. These are more like an assert for invariants
            VerifyOrExit(messageData == buffer->Start(), err = CHIP_ERROR_INTERNAL);
            VerifyOrExit(buffer->DataLength() >= messageSize, err = CHIP_ERROR_INTERNAL);

            err = ProcessSingleMessageFromBufferHead(peerAddress, buffer, messageSize);
            buffer->ConsumeHead(messageSize);
            SuccessOrExit(err);

            err = endPoint->AckReceive(messageSize);
            SuccessOrExit(err);
            continue;
        }

        // Buffer is incomplete if we reach this point
        if (buffer->Next() != nullptr)
        {
            buffer->CompactHead();
            continue;
        }

        if (messageSize > 0)
        {
            // Open the receive window just enough to allow the remainder of the message to be received.
            // This is necessary in the case where the message size exceeds the TCP window size to ensure
            // the peer has enough window to send us the entire message.
            uint16_t neededLen = messageSize - buffer->DataLength();
            err                = endPoint->AckReceive(neededLen);
            SuccessOrExit(err);
        }

        // Buffer is incomplete and we cannot get more data
        break;
    }

exit:
    if (buffer != nullptr)
    {
        // Incomplete processing will be retried
        endPoint->PutBackReceivedData(buffer);
    }

    return err;
}

void TCPBase::OnTcpReceive(Inet::TCPEndPoint * endPoint, System::PacketBuffer * buffer)
{
    IPAddress ipAddress;
    uint16_t port;

    endPoint->GetPeerInfo(&ipAddress, &port);
    PeerAddress peerAddress = PeerAddress::TCP(ipAddress, port);

    TCPBase * tcp  = reinterpret_cast<TCPBase *>(endPoint->AppState);
    CHIP_ERROR err = tcp->ProcessReceivedBuffer(endPoint, peerAddress, buffer);

    if (err != CHIP_NO_ERROR)
    {
        // Connection could need to be closed at this point
        ChipLogError(Inet, "Failed to receive TCP message: %s", ErrorStr(err));
    }
}

void TCPBase::OnConnectionComplete(Inet::TCPEndPoint * endPoint, INET_ERROR inetErr)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    bool foundPendingPacket = false;
    TCPBase * tcp           = reinterpret_cast<TCPBase *>(endPoint->AppState);
    IPAddress ipAddress;
    uint16_t port;

    endPoint->GetPeerInfo(&ipAddress, &port);
    PeerAddress addr = PeerAddress::TCP(ipAddress, port);

    // Send any pending packets
    for (size_t i = 0; i < tcp->mPendingPacketsSize; i++)
    {
        if ((tcp->mPendingPackets[i].peerAddress != addr) || (tcp->mPendingPackets[i].packetBuffer == nullptr))
        {
            continue;
        }
        foundPendingPacket = true;

        if ((inetErr == CHIP_NO_ERROR) && (err == CHIP_NO_ERROR))
        {
            err = endPoint->Send(tcp->mPendingPackets[i].packetBuffer);
        }

        tcp->mPendingPackets[i].packetBuffer = nullptr;
        tcp->mPendingPackets[i].peerAddress  = PeerAddress::Uninitialized();
    }

    if (err == CHIP_NO_ERROR)
    {
        err = inetErr;
    }

    if (!foundPendingPacket && (err == CHIP_NO_ERROR))
    {
        // Force a close: new connections are only expected when a
        // new buffer is being sent.
        ChipLogError(Inet, "Connection accepted without pending buffers");
        err = CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY;
    }

    // cleanup packets or mark as free
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Connection complete encountered an error: %s", ErrorStr(err));
        endPoint->Free();
    }
    else
    {
        for (size_t i = 0; i < tcp->mActiveConnectionsSize; i++)
        {
            if (tcp->mActiveConnections[i] == nullptr)
            {
                tcp->mActiveConnections[i] = endPoint;
                break;
            }
        }
    }
}

void TCPBase::OnConnectionClosed(Inet::TCPEndPoint * endPoint, INET_ERROR err)
{
    // FIXME: implement
    ChipLogError(Inet, "%s not yet implemented", __PRETTY_FUNCTION__);
}

void TCPBase::OnConnectionRecevied(Inet::TCPEndPoint * listenEndPoint, Inet::TCPEndPoint * endPoint, const IPAddress & peerAddress,
                                   uint16_t peerPort)
{
    TCPBase * tcp = reinterpret_cast<TCPBase *>(listenEndPoint->AppState);

    if (tcp->mUsedEndPointCount < tcp->mActiveConnectionsSize)
    {
        // have space to use one more (even if considering pending connections)
        for (size_t i = 0; i < tcp->mActiveConnectionsSize; i++)
        {
            if (tcp->mActiveConnections[i] == nullptr)
            {
                tcp->mActiveConnections[i] = endPoint;
                break;
            }
        }

        endPoint->AppState             = listenEndPoint->AppState;
        endPoint->OnDataReceived       = OnTcpReceive;
        endPoint->OnConnectComplete    = OnConnectionComplete;
        endPoint->OnConnectionClosed   = OnConnectionClosed;
        endPoint->OnConnectionReceived = OnConnectionRecevied;
        endPoint->OnAcceptError        = OnAcceptError;
    }
    else
    {
        ChipLogError(Inet, "Insufficient connection space to accept new connections");
        endPoint->Free();
    }
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
