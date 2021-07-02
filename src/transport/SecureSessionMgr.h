/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 * @file
 *   This file defines a secure transport layer which adds encryption to data
 *   sent over a transport.
 *
 */

#pragma once

#include <utility>

#include <core/CHIPCore.h>
#include <inet/IPAddress.h>
#include <inet/IPEndPointBasis.h>
#include <protocols/secure_channel/Constants.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <transport/AdminPairingTable.h>
#include <transport/MessageCounterManagerInterface.h>
#include <transport/PairingSession.h>
#include <transport/PeerConnections.h>
#include <transport/SecureSession.h>
#include <transport/SecureSessionHandle.h>
#include <transport/TransportMgr.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/Tuple.h>

namespace chip {

/**
 * @brief
 *  Tracks ownership of a encrypted packet buffer.
 *
 *  EncryptedPacketBufferHandle is a kind of PacketBufferHandle class and used to hold a packet buffer
 *  object whose payload has already been encrypted.
 */
class EncryptedPacketBufferHandle final : private System::PacketBufferHandle
{
public:
    EncryptedPacketBufferHandle() {}
    EncryptedPacketBufferHandle(EncryptedPacketBufferHandle && aBuffer) : PacketBufferHandle(std::move(aBuffer)) {}

    void operator=(EncryptedPacketBufferHandle && aBuffer) { PacketBufferHandle::operator=(std::move(aBuffer)); }

    using System::PacketBufferHandle::IsNull;
    // Pass-through to HasChainedBuffer on our underlying buffer without
    // exposing operator->
    bool HasChainedBuffer() const { return (*this)->HasChainedBuffer(); }

    uint32_t GetMsgId() const;

    /**
     * Creates a copy of the data in this packet.
     *
     * Does NOT support chained buffers.
     *
     * @returns empty handle on allocation failure.
     */
    EncryptedPacketBufferHandle CloneData() { return EncryptedPacketBufferHandle(PacketBufferHandle::CloneData()); }

#ifdef CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API
    /**
     * Extracts the (unencrypted) packet header from this encrypted packet
     * buffer.  Returns error if a packet header cannot be extracted (e.g. if
     * there are not enough bytes in this packet buffer).  After this call the
     * buffer does not have a packet header.  This API is meant for
     * unit tests only.   The CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API define
     * should not be defined normally.
     */
    CHIP_ERROR ExtractPacketHeader(PacketHeader & aPacketHeader) { return aPacketHeader.DecodeAndConsume(*this); }

    /**
     * Inserts a new (unencrypted) packet header in the encrypted packet buffer
     * based on the given PacketHeader.  This API is meant for
     * unit tests only.   The CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API define
     * should not be defined normally.
     */
    CHIP_ERROR InsertPacketHeader(const PacketHeader & aPacketHeader) { return aPacketHeader.EncodeBeforeData(*this); }
#endif // CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

    static EncryptedPacketBufferHandle MarkEncrypted(PacketBufferHandle && aBuffer)
    {
        return EncryptedPacketBufferHandle(std::move(aBuffer));
    }

    /**
     * Get a handle to the data that allows mutating the bytes.  This should
     * only be used if absolutely necessary, because EncryptedPacketBufferHandle
     * represents a buffer that we want to resend as-is.
     */
    PacketBufferHandle CastToWritable() const { return PacketBufferHandle::Retain(); }

private:
    EncryptedPacketBufferHandle(PacketBufferHandle && aBuffer) : PacketBufferHandle(std::move(aBuffer)) {}
};

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by SecureSssionMgrBase object on specific events. If the user of SecureSessionMgr
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each trigger in their implementation of this class.
 */
class DLL_EXPORT SecureSessionMgrDelegate
{
public:
    enum class DuplicateMessage : uint8_t
    {
        Yes,
        No,
    };

    /**
     * @brief
     *   Called when a new message is received. The function must internally release the
     *   msgBuf after processing it.
     *
     * @param packetHeader  The message header
     * @param payloadHeader The payload header
     * @param session       The handle to the secure session
     * @param source        The sender's address
     * @param isDuplicate   The message is a duplicate of previously received message
     * @param msgBuf        The received message
     */
    virtual void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                   SecureSessionHandle session, const Transport::PeerAddress & source, DuplicateMessage isDuplicate,
                                   System::PacketBufferHandle && msgBuf)
    {}

    /**
     * @brief
     *   Called when received message processing resulted in error
     *
     * @param error   error code
     * @param source  network entity that sent the message
     */
    virtual void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source) {}

    /**
     * @brief
     *   Called when a new pairing is being established
     *
     * @param session The handle to the secure session
     */
    virtual void OnNewConnection(SecureSessionHandle session) {}

    /**
     * @brief
     *   Called when a new connection is closing
     *
     * @param session The handle to the secure session
     */
    virtual void OnConnectionExpired(SecureSessionHandle session) {}

    virtual ~SecureSessionMgrDelegate() {}
};

class DLL_EXPORT SecureSessionMgr : public TransportMgrDelegate
{
public:
    SecureSessionMgr();
    ~SecureSessionMgr() override;

    /**
     * @brief
     *   This function takes the payload and returns an encrypted message which can be sent multiple times.
     *
     * @details
     *   It does the following:
     *    1. Encrypt the msgBuf
     *    2. construct the packet header
     *    3. Encode the packet header and prepend it to message.
     *   Returns a encrypted message in encryptedMessage.
     */
    CHIP_ERROR BuildEncryptedMessagePayload(SecureSessionHandle session, PayloadHeader & payloadHeader,
                                            System::PacketBufferHandle && msgBuf, EncryptedPacketBufferHandle & encryptedMessage);

    /**
     * @brief
     *   Send a prepared message to a currently connected peer.
     */
    CHIP_ERROR SendPreparedMessage(SecureSessionHandle session, const EncryptedPacketBufferHandle & preparedMessage);

    Transport::PeerConnectionState * GetPeerConnectionState(SecureSessionHandle session);

    /**
     * @brief
     *   Set the callback object.
     *
     * @details
     *   Release if there was an existing callback object
     */
    void SetDelegate(SecureSessionMgrDelegate * cb) { mCB = cb; }

    /**
     * @brief
     *   Establish a new pairing with a peer node
     *
     * @details
     *   This method sets up a new pairing with the peer node. It also
     *   establishes the security keys for secure communication with the
     *   peer node.
     */
    CHIP_ERROR NewPairing(const Optional<Transport::PeerAddress> & peerAddr, NodeId peerNodeId, PairingSession * pairing,
                          SecureSession::SessionRole direction, Transport::AdminId admin, Transport::Base * transport = nullptr);

    void ExpirePairing(SecureSessionHandle session);
    void ExpireAllPairings(NodeId peerNodeId, Transport::AdminId admin);

    /**
     * @brief
     *   Return the System Layer pointer used by current SecureSessionMgr.
     */
    System::Layer * SystemLayer() { return mSystemLayer; }

    /**
     * @brief
     *   Initialize a Secure Session Manager
     *
     * @param localNodeId           Node id for the current node
     * @param systemLayer           System, layer to use
     * @param transportMgr          Transport to use
     * @param admins                A table of device administrators
     * @param messageCounterManager The message counter manager
     */
    CHIP_ERROR Init(NodeId localNodeId, System::Layer * systemLayer, TransportMgrBase * transportMgr,
                    Transport::AdminPairingTable * admins, Transport::MessageCounterManagerInterface * messageCounterManager);

    /**
     * @brief
     *  Shutdown the Secure Session Manager. This terminates this instance
     *  of the object and reset it's state.
     */
    void Shutdown();

    /**
     * @brief
     *   Set local node ID
     *
     * @param nodeId    Node id for the current node
     */
    void SetLocalNodeId(NodeId nodeId) { mLocalNodeId = nodeId; }

    NodeId GetLocalNodeId() { return mLocalNodeId; }

    /**
     * @brief
     *   Return the transport type of current connection to the node with id peerNodeId.
     *   'Transport::Type::kUndefined' will be returned if the connection to the specified
     *   peer node does not exist.
     */
    Transport::Type GetTransportType(NodeId peerNodeId);

    TransportMgrBase * GetTransportManager() const { return mTransportMgr; }

    /**
     * @brief
     *   Handle received secure message. Implements TransportMgrDelegate
     *
     * @param source    the source address of the package
     * @param msgBuf    the buffer containing a full CHIP message (except for the optional length field).
     */
    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf) override;

private:
    /**
     *    The State of a secure transport object.
     */
    enum class State
    {
        kNotReady,    /**< State before initialization. */
        kInitialized, /**< State when the object is ready connect to other peers. */
    };

    enum class EncryptionState
    {
        kPayloadIsEncrypted,
        kPayloadIsUnencrypted,
    };

    System::Layer * mSystemLayer = nullptr;
    NodeId mLocalNodeId;                                                                // < Id of the current node
    Transport::PeerConnections<CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE> mPeerConnections; // < Active connections to other peers
    State mState;                                                                       // < Initialization state of the object

    SecureSessionMgrDelegate * mCB                                     = nullptr;
    TransportMgrBase * mTransportMgr                                   = nullptr;
    Transport::AdminPairingTable * mAdmins                             = nullptr;
    Transport::MessageCounterManagerInterface * mMessageCounterManager = nullptr;

    GlobalUnencryptedMessageCounter mGlobalUnencryptedMessageCounter;
    GlobalEncryptedMessageCounter mGlobalEncryptedMessageCounter;

    /** Schedules a new oneshot timer for checking connection expiry. */
    void ScheduleExpiryTimer();

    /** Cancels any active timers for connection expiry checks. */
    void CancelExpiryTimer();

    /**
     * Called when a specific connection expires.
     */
    void HandleConnectionExpired(const Transport::PeerConnectionState & state);

    /**
     * Callback for timer expiry check
     */
    static void ExpiryTimerCallback(System::Layer * layer, void * param, CHIP_ERROR error);

    void SecureMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                               System::PacketBufferHandle && msg);
    void MessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                         System::PacketBufferHandle && msg);

    static bool IsControlMessage(PayloadHeader & payloadHeader)
    {
        return payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq) ||
            payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp);
    }

    MessageCounter & GetSendCounterForPacket(PayloadHeader & payloadHeader, Transport::PeerConnectionState & state)
    {
        if (IsControlMessage(payloadHeader))
        {
            return mGlobalEncryptedMessageCounter;
        }
        else
        {
            return state.GetSessionMessageCounter().GetLocalMessageCounter();
        }
    }
};

namespace MessagePacketBuffer {
/**
 * Maximum size of a message footer, in bytes.
 */
constexpr uint16_t kMaxFooterSize = kMaxTagLen;

/**
 * Allocates a packet buffer with space for message headers and footers.
 *
 *  Fails and returns \c nullptr if no memory is available, or if the size requested is too large.
 *
 *  @param[in]  aAvailableSize  Minimum number of octets to for application data.
 *
 *  @return     On success, a PacketBufferHandle to the allocated buffer. On fail, \c nullptr.
 */
inline System::PacketBufferHandle New(size_t aAvailableSize)
{
    static_assert(System::PacketBuffer::kMaxSize > kMaxFooterSize, "inadequate capacity");
    if (aAvailableSize > System::PacketBuffer::kMaxSize - kMaxFooterSize)
    {
        return System::PacketBufferHandle();
    }
    return System::PacketBufferHandle::New(aAvailableSize + kMaxFooterSize);
}

/**
 * Allocates a packet buffer with initial contents.
 *
 *  @param[in]  aData           Initial buffer contents.
 *  @param[in]  aDataSize       Size of initial buffer contents.
 *
 *  @return     On success, a PacketBufferHandle to the allocated buffer. On fail, \c nullptr.
 */
inline System::PacketBufferHandle NewWithData(const void * aData, size_t aDataSize)
{
    return System::PacketBufferHandle::NewWithData(aData, aDataSize, kMaxFooterSize);
}

/**
 * Check whether a packet buffer has enough space for a message footer.
 *
 * @returns true if there is space, false otherwise.
 */
inline bool HasFooterSpace(const System::PacketBufferHandle & aBuffer)
{
    return aBuffer->AvailableDataLength() >= kMaxFooterSize;
}

} // namespace MessagePacketBuffer

} // namespace chip
