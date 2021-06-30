/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>

#include <inttypes.h>

namespace chip {
namespace Transport {
namespace {

using namespace chip::Encoding;

// Packets start with a 16-bit size
constexpr size_t kPacketSizeBytes = 2;

// TODO: Actual limit may be lower (spec issue #2119)
constexpr uint16_t kMaxMessageSize = static_cast<uint16_t>(System::PacketBuffer::kMaxSizeWithoutReserve - kPacketSizeBytes);

constexpr int kListenBacklogSize = 2;

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
        if (mActiveConnections[i].InUse())
        {
            mActiveConnections[i].Free();
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
    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
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

void TCPBase::Close()
{
    if (mListenSocket)
    {
        mListenSocket->Free();
        mListenSocket = nullptr;
    }
    mState = State::kNotReady;
}

TCPBase::ActiveConnectionState * TCPBase::FindActiveConnection(const PeerAddress & address)
{
    if (address.GetTransportType() != Type::kTcp)
    {
        return nullptr;
    }

    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (!mActiveConnections[i].InUse())
        {
            continue;
        }
        Inet::IPAddress addr;
        uint16_t port;
        mActiveConnections[i].mEndPoint->GetPeerInfo(&addr, &port);

        if ((addr == address.GetIPAddress()) && (port == address.GetPort()))
        {
            return &mActiveConnections[i];
        }
    }

    return nullptr;
}

TCPBase::ActiveConnectionState * TCPBase::FindActiveConnection(const Inet::TCPEndPoint * endPoint)
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].mEndPoint == endPoint)
        {
            return &mActiveConnections[i];
        }
    }
    return nullptr;
}

CHIP_ERROR TCPBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    // Sent buffer data format is:
    //    - packet size as a uint16_t
    //    - actual data

    VerifyOrReturnError(address.GetTransportType() == Type::kTcp, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mState == State::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(kPacketSizeBytes + msgBuf->DataLength() <= std::numeric_limits<uint16_t>::max(),
                        CHIP_ERROR_INVALID_ARGUMENT);

    // The check above about kPacketSizeBytes + msgBuf->DataLength() means it definitely fits in uint16_t.
    VerifyOrReturnError(msgBuf->EnsureReservedSize(static_cast<uint16_t>(kPacketSizeBytes)), CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - kPacketSizeBytes);

    uint8_t * output = msgBuf->Start();
    LittleEndian::Write16(output, static_cast<uint16_t>(msgBuf->DataLength() - kPacketSizeBytes));

    // Reuse existing connection if one exists, otherwise a new one
    // will be established
    ActiveConnectionState * connection = FindActiveConnection(address);

    if (connection != nullptr)
    {
        return connection->mEndPoint->Send(std::move(msgBuf));
    }
    else
    {
        return SendAfterConnect(address, std::move(msgBuf));
    }
}

CHIP_ERROR TCPBase::SendAfterConnect(const PeerAddress & addr, System::PacketBufferHandle && msg)
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
    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
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

CHIP_ERROR TCPBase::ProcessReceivedBuffer(Inet::TCPEndPoint * endPoint, const PeerAddress & peerAddress,
                                          System::PacketBufferHandle && buffer)
{
    ActiveConnectionState * state = FindActiveConnection(endPoint);
    VerifyOrReturnError(state != nullptr, CHIP_ERROR_INTERNAL);
    state->mReceived.AddToEnd(std::move(buffer));

    while (!state->mReceived.IsNull())
    {
        uint8_t messageSizeBuf[kPacketSizeBytes];
        CHIP_ERROR err = state->mReceived->Read(messageSizeBuf);
        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            // We don't have enough data to read the message size. Wait until there's more.
            return CHIP_NO_ERROR;
        }
        else if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        uint16_t messageSize = LittleEndian::Get16(messageSizeBuf);
        if (messageSize >= kMaxMessageSize)
        {
            // This message is too long for upper layers.
            return CHIP_ERROR_MESSAGE_TOO_LONG;
        }
        // The subtraction will not underflow because we successfully read kPacketSizeBytes.
        if (messageSize > (state->mReceived->TotalLength() - kPacketSizeBytes))
        {
            // We have not yet received the complete message.
            return CHIP_NO_ERROR;
        }
        state->mReceived.Consume(kPacketSizeBytes);
        ReturnErrorOnFailure(ProcessSingleMessage(peerAddress, state, messageSize));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPBase::ProcessSingleMessage(const PeerAddress & peerAddress, ActiveConnectionState * state, uint16_t messageSize)
{
    // We enter with `state->mReceived` containing at least one full message, perhaps in a chain.
    // `state->mReceived->Start()` currently points to the message data.
    // On exit, `state->mReceived` will have had `messageSize` bytes consumed, no matter what.
    System::PacketBufferHandle message;
    if (state->mReceived->DataLength() == messageSize)
    {
        // In this case, the head packet buffer contains exactly the message.
        // This is common because typical messages fit in a network packet, and are delivered as such.
        // Peel off the head to pass upstream, which effectively consumes it from `state->mReceived`.
        message = state->mReceived.PopHead();
    }
    else
    {
        // The message is either longer or shorter than the head buffer.
        // In either case, copy the message to a fresh linear buffer to pass upstream. We always copy, rather than provide
        // a shared reference to the current buffer, in case upper layers manipulate the buffer in ways that would affect
        // our use, e.g. chaining it elsewhere or reusing space beyond the current message.
        message = System::PacketBufferHandle::New(messageSize, 0);
        if (message.IsNull())
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        CHIP_ERROR err = state->mReceived->Read(message->Start(), messageSize);
        state->mReceived.Consume(messageSize);
        ReturnErrorOnFailure(err);
        message->SetDataLength(messageSize);
    }

    HandleMessageReceived(peerAddress, std::move(message));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPBase::OnTcpReceive(Inet::TCPEndPoint * endPoint, System::PacketBufferHandle && buffer)
{
    Inet::IPAddress ipAddress;
    uint16_t port;
    Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID;

    endPoint->GetPeerInfo(&ipAddress, &port);
    endPoint->GetInterfaceId(&interfaceId);
    PeerAddress peerAddress = PeerAddress::TCP(ipAddress, port, interfaceId);

    TCPBase * tcp  = reinterpret_cast<TCPBase *>(endPoint->AppState);
    CHIP_ERROR err = tcp->ProcessReceivedBuffer(endPoint, peerAddress, std::move(buffer));

    if (err != CHIP_NO_ERROR)
    {
        // Connection could need to be closed at this point
        ChipLogError(Inet, "Failed to accept received TCP message: %s", ErrorStr(err));
        return CHIP_ERROR_UNEXPECTED_EVENT;
    }
    return CHIP_NO_ERROR;
}

void TCPBase::OnConnectionComplete(Inet::TCPEndPoint * endPoint, CHIP_ERROR inetErr)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    bool foundPendingPacket = false;
    TCPBase * tcp           = reinterpret_cast<TCPBase *>(endPoint->AppState);
    Inet::IPAddress ipAddress;
    uint16_t port;
    Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID;

    endPoint->GetPeerInfo(&ipAddress, &port);
    endPoint->GetInterfaceId(&interfaceId);
    PeerAddress addr = PeerAddress::TCP(ipAddress, port, interfaceId);

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
            if (!tcp->mActiveConnections[i].InUse())
            {
                tcp->mActiveConnections[i].Init(endPoint);
                connectionStored = true;
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

void TCPBase::OnConnectionClosed(Inet::TCPEndPoint * endPoint, CHIP_ERROR err)
{
    TCPBase * tcp = reinterpret_cast<TCPBase *>(endPoint->AppState);

    ChipLogProgress(Inet, "Connection closed.");

    for (size_t i = 0; i < tcp->mActiveConnectionsSize; i++)
    {
        if (tcp->mActiveConnections[i].mEndPoint == endPoint)
        {
            ChipLogProgress(Inet, "Freeing closed connection.");
            tcp->mActiveConnections[i].Free();
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
            if (!tcp->mActiveConnections[i].InUse())
            {
                tcp->mActiveConnections[i].Init(endPoint);
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

void TCPBase::OnAcceptError(Inet::TCPEndPoint * endPoint, CHIP_ERROR err)
{
    ChipLogError(Inet, "Accept error: %s", ErrorStr(err));
}

void TCPBase::Disconnect(const PeerAddress & address)
{
    // Closes an existing connection
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].InUse())
        {
            Inet::IPAddress ipAddress;
            uint16_t port;
            Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID;

            mActiveConnections[i].mEndPoint->GetPeerInfo(&ipAddress, &port);
            mActiveConnections[i].mEndPoint->GetInterfaceId(&interfaceId);
            if (address == PeerAddress::TCP(ipAddress, port, interfaceId))
            {
                // NOTE: this leaves the socket in TIME_WAIT.
                // Calling Abort() would clean it since SO_LINGER would be set to 0,
                // however this seems not to be useful.
                mActiveConnections[i].Free();
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
        if (tcp->mActiveConnections[i].mEndPoint == endPoint)
        {
            ChipLogProgress(Inet, "Freeing connection: connection closed by peer");
            tcp->mActiveConnections[i].Free();
            tcp->mUsedEndPointCount--;
        }
    }
}

bool TCPBase::HasActiveConnections() const
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].InUse())
        {
            return true;
        }
    }

    return false;
}

} // namespace Transport
} // namespace chip
