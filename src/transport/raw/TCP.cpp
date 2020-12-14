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
 *      This file implements the CHIP Transport object that maintains TCP connections
 *      to peers. Handles both establishing new connections and accepting peer connection
 *      requests.
 */
#include <transport/raw/TCP.h>

#include <core/CHIPEncoding.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>

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
bool ContainsCompleteMessage(const System::PacketBufferHandle & buffer, uint8_t ** start, uint16_t * size)
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
        mListenSocket->Free();
        mListenSocket = nullptr;
    }

    CloseActiveConnections();

    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        mPendingPackets[i].packetBuffer = nullptr;
    }
}

void TCPBase::CloseActiveConnections()
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i] != nullptr)
        {
            mActiveConnections[i]->Free();
            mActiveConnections[i] = nullptr;
            mUsedEndPointCount--;
        }
    }
}

CHIP_ERROR TCPBase::Init(TcpListenParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    err = params.GetInetLayer()->NewTCPEndPoint(&mListenSocket);
#else
    err = CHIP_SYSTEM_ERROR_NOT_SUPPORTED;
#endif
    SuccessOrExit(err);

    err = mListenSocket->Bind(params.GetAddressType(), Inet::IPAddress::Any, params.GetListenPort(), params.GetInterfaceId());
    SuccessOrExit(err);

    err = mListenSocket->Listen(kListenBacklogSize);
    SuccessOrExit(err);

    mListenSocket->AppState             = reinterpret_cast<void *>(this);
    mListenSocket->OnDataReceived       = OnTcpReceive;
    mListenSocket->OnConnectComplete    = OnConnectionComplete;
    mListenSocket->OnConnectionClosed   = OnConnectionClosed;
    mListenSocket->OnConnectionReceived = OnConnectionReceived;
    mListenSocket->OnAcceptError        = OnAcceptError;
    mEndpointType                       = params.GetAddressType();

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
    if (address.GetTransportType() != Type::kTcp)
    {
        return nullptr;
    }

    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i] == nullptr)
        {
            continue;
        }
        Inet::IPAddress addr;
        uint16_t port;
        mActiveConnections[i]->GetPeerInfo(&addr, &port);

        if ((addr == address.GetIPAddress()) && (port == address.GetPort()))
        {
            return mActiveConnections[i];
        }
    }

    return nullptr;
}

CHIP_ERROR TCPBase::SendMessage(const PacketHeader & header, const Transport::PeerAddress & address,
                                System::PacketBufferHandle msgBuf)
{
    // Sent buffer data format is:
    //    - packet size as a uint16_t (inludes size of header and actual data)
    //    - header
    //    - actual data
    const size_t prefixSize = header.EncodeSizeBytes() + kPacketSizeBytes;

    VerifyOrReturnError(address.GetTransportType() == Type::kTcp, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mState == State::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(prefixSize + msgBuf->DataLength() <= std::numeric_limits<uint16_t>::max(), CHIP_ERROR_INVALID_ARGUMENT);

    // The check above about prefixSize + msgBuf->DataLength() means prefixSize
    // definitely fits in uint16_t.
    VerifyOrReturnError(msgBuf->EnsureReservedSize(static_cast<uint16_t>(prefixSize)), CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - prefixSize);

    // Length is actual data, without considering the length bytes themselves
    VerifyOrReturnError(msgBuf->DataLength() >= kPacketSizeBytes, CHIP_ERROR_INTERNAL);

    uint8_t * output = msgBuf->Start();
    LittleEndian::Write16(output, static_cast<uint16_t>(msgBuf->DataLength() - kPacketSizeBytes));

    uint16_t actualEncodedHeaderSize;
    ReturnErrorOnFailure(header.Encode(output, msgBuf->DataLength(), &actualEncodedHeaderSize));

    // header encoding has to match space that we allocated
    VerifyOrReturnError(prefixSize == actualEncodedHeaderSize + kPacketSizeBytes, CHIP_ERROR_INTERNAL);

    // Reuse existing connection if one exists, otherwise a new one
    // will be established
    Inet::TCPEndPoint * endPoint = FindActiveConnection(address);

    if (endPoint != nullptr)
    {
        return endPoint->Send(std::move(msgBuf));
    }
    else
    {
        return SendAfterConnect(address, std::move(msgBuf));
    }
}

CHIP_ERROR TCPBase::SendAfterConnect(const PeerAddress & addr, System::PacketBufferHandle msg)
{
    // This will initiate a connection to the specified peer
    CHIP_ERROR err               = CHIP_NO_ERROR;
    PendingPacket * packet       = nullptr;
    bool alreadyConnecting       = false;
    Inet::TCPEndPoint * endPoint = nullptr;

    // Iterate through the ENTIRE array. If a pending packet for
    // the address already exists, this means a connection is pending and
    // does NOT need to be re-established.
    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (mPendingPackets[i].packetBuffer.IsNull())
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
                packet->peerAddress  = addr;
                packet->packetBuffer = std::move(mPendingPackets[i].packetBuffer);
                packet               = mPendingPackets + i;
            }
        }
    }

    VerifyOrExit(packet != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // If already connecting, buffer was just enqueued for more sending
    VerifyOrExit(!alreadyConnecting, err = CHIP_NO_ERROR);

    // Ensures sufficient active connections size exist
    VerifyOrExit(mUsedEndPointCount < mActiveConnectionsSize, err = CHIP_ERROR_NO_MEMORY);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    err = mListenSocket->Layer().NewTCPEndPoint(&endPoint);
#else
    err = CHIP_SYSTEM_ERROR_NOT_SUPPORTED;
#endif
    SuccessOrExit(err);

    endPoint->AppState             = reinterpret_cast<void *>(this);
    endPoint->OnDataReceived       = OnTcpReceive;
    endPoint->OnConnectComplete    = OnConnectionComplete;
    endPoint->OnConnectionClosed   = OnConnectionClosed;
    endPoint->OnConnectionReceived = OnConnectionReceived;
    endPoint->OnAcceptError        = OnAcceptError;
    endPoint->OnPeerClose          = OnPeerClosed;

    err = endPoint->Connect(addr.GetIPAddress(), addr.GetPort(), addr.GetInterface());
    SuccessOrExit(err);

    // enqueue the packet once the connection succeeds
    packet->peerAddress  = addr;
    packet->packetBuffer = std::move(msg);
    mUsedEndPointCount++;

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (endPoint != nullptr)
        {
            endPoint->Free();
        }
    }
    return err;
}

CHIP_ERROR TCPBase::ProcessSingleMessageFromBufferHead(const PeerAddress & peerAddress, const System::PacketBufferHandle & buffer,
                                                       uint16_t messageSize)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint8_t * oldStart = buffer->Start();
    uint16_t oldLength = buffer->DataLength();

    buffer->SetDataLength(messageSize);

    uint16_t headerSize = 0;

    PacketHeader header;
    err = header.Decode(buffer->Start(), buffer->DataLength(), &headerSize);
    SuccessOrExit(err);

    buffer->ConsumeHead(headerSize);

    // message receive handler will attempt to free the buffer, however as the buffer may
    // contain additional data, we retain it to prevent actual free
    HandleMessageReceived(header, peerAddress, buffer.Retain());

exit:
    buffer->SetStart(oldStart);
    buffer->SetDataLength(oldLength);

    return err;
}

CHIP_ERROR TCPBase::ProcessReceivedBuffer(Inet::TCPEndPoint * endPoint, const PeerAddress & peerAddress,
                                          System::PacketBufferHandle buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    while (!buffer.IsNull())
    {
        // when a buffer is empty, it can be released back to the app
        if (buffer->DataLength() == 0)
        {
            buffer.FreeHead();
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

            // messagesize is always consumed once processed, even on error. This is done
            // on purpose:
            //   - we already consumed the packet size above
            //   - there is no reason to believe that an error would not occur again on the
            //     same parameters (errors are likely not transient)
            //   - this guarantees data is received and progress is made.
            err = ProcessSingleMessageFromBufferHead(peerAddress, buffer, messageSize);
            buffer->ConsumeHead(messageSize);
            SuccessOrExit(err);

            err = endPoint->AckReceive(messageSize);
            SuccessOrExit(err);
            continue;
        }

        // Buffer is incomplete if we reach this point
        if (buffer->HasChainedBuffer())
        {
            buffer->CompactHead();
            continue;
        }

        if (messageSize > 0)
        {
            // Open the receive window just enough to allow the remainder of the message to be received.
            // This is necessary in the case where the message size exceeds the TCP window size to ensure
            // the peer has enough window to send us the entire message.
            uint16_t neededLen = static_cast<uint16_t>(messageSize - buffer->DataLength());
            err                = endPoint->AckReceive(neededLen);
            SuccessOrExit(err);
        }

        // Buffer is incomplete and we cannot get more data
        break;
    }

exit:
    if (!buffer.IsNull())
    {
        // Incomplete processing will be retried
        endPoint->PutBackReceivedData(std::move(buffer));
    }

    return err;
}

void TCPBase::OnTcpReceive(Inet::TCPEndPoint * endPoint, System::PacketBufferHandle buffer)
{
    Inet::IPAddress ipAddress;
    uint16_t port;

    endPoint->GetPeerInfo(&ipAddress, &port);
    PeerAddress peerAddress = PeerAddress::TCP(ipAddress, port);

    TCPBase * tcp  = reinterpret_cast<TCPBase *>(endPoint->AppState);
    CHIP_ERROR err = tcp->ProcessReceivedBuffer(endPoint, peerAddress, std::move(buffer));

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
    Inet::IPAddress ipAddress;
    uint16_t port;

    endPoint->GetPeerInfo(&ipAddress, &port);
    PeerAddress addr = PeerAddress::TCP(ipAddress, port);

    // Send any pending packets
    for (size_t i = 0; i < tcp->mPendingPacketsSize; i++)
    {
        if ((tcp->mPendingPackets[i].peerAddress != addr) || (tcp->mPendingPackets[i].packetBuffer.IsNull()))
        {
            continue;
        }
        foundPendingPacket = true;

        System::PacketBufferHandle buffer   = std::move(tcp->mPendingPackets[i].packetBuffer);
        tcp->mPendingPackets[i].peerAddress = PeerAddress::Uninitialized();

        if ((inetErr == CHIP_NO_ERROR) && (err == CHIP_NO_ERROR))
        {
            err = endPoint->Send(std::move(buffer));
        }
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
        tcp->mUsedEndPointCount--;
    }
    else
    {
        bool connectionStored = false;
        for (size_t i = 0; i < tcp->mActiveConnectionsSize; i++)
        {
            if (tcp->mActiveConnections[i] == nullptr)
            {
                tcp->mActiveConnections[i] = endPoint;
                connectionStored           = true;
                break;
            }
        }

        // since we track end points counts, we always expect to store the
        // connection.
        if (!connectionStored)
        {
            endPoint->Free();
            ChipLogError(Inet, "Internal logic error: insufficient space to store active connection");
        }
    }
}

void TCPBase::OnConnectionClosed(Inet::TCPEndPoint * endPoint, INET_ERROR err)
{
    TCPBase * tcp = reinterpret_cast<TCPBase *>(endPoint->AppState);

    ChipLogProgress(Inet, "Connection closed.");

    for (size_t i = 0; i < tcp->mActiveConnectionsSize; i++)
    {
        if (tcp->mActiveConnections[i] == endPoint)
        {
            ChipLogProgress(Inet, "Freeing closed connection.");
            tcp->mActiveConnections[i]->Free();
            tcp->mActiveConnections[i] = nullptr;
            tcp->mUsedEndPointCount--;
        }
    }
}

void TCPBase::OnConnectionReceived(Inet::TCPEndPoint * listenEndPoint, Inet::TCPEndPoint * endPoint,
                                   const Inet::IPAddress & peerAddress, uint16_t peerPort)
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
        endPoint->OnConnectionReceived = OnConnectionReceived;
        endPoint->OnAcceptError        = OnAcceptError;
        endPoint->OnPeerClose          = OnPeerClosed;
    }
    else
    {
        ChipLogError(Inet, "Insufficient connection space to accept new connections");
        endPoint->Free();
    }
}

void TCPBase::OnAcceptError(Inet::TCPEndPoint * endPoint, INET_ERROR err)
{
    ChipLogError(Inet, "Accept error: %s", ErrorStr(err));
}

void TCPBase::Disconnect(const PeerAddress & address)
{
    // Closes an existing connection
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i] != nullptr)
        {
            Inet::IPAddress ipAddress;
            uint16_t port;

            mActiveConnections[i]->GetPeerInfo(&ipAddress, &port);
            if (address == PeerAddress::TCP(ipAddress, port))
            {
                // NOTE: this leaves the socket in TIME_WAIT.
                // Calling Abort() would clean it since SO_LINGER would be set to 0,
                // however this seems not to be useful.
                mActiveConnections[i]->Free();
                mActiveConnections[i] = nullptr;
                mUsedEndPointCount--;
            }
        }
    }
}

void TCPBase::OnPeerClosed(Inet::TCPEndPoint * endPoint)
{
    TCPBase * tcp = reinterpret_cast<TCPBase *>(endPoint->AppState);

    for (size_t i = 0; i < tcp->mActiveConnectionsSize; i++)
    {
        if (tcp->mActiveConnections[i] == endPoint)
        {
            ChipLogProgress(Inet, "Freeing connection: connection closed by peer");
            tcp->mActiveConnections[i]->Free();
            tcp->mActiveConnections[i] = nullptr;
            tcp->mUsedEndPointCount--;
        }
    }
}

bool TCPBase::HasActiveConnections() const
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i] != nullptr)
        {
            return true;
        }
    }

    return false;
}

} // namespace Transport
} // namespace chip
