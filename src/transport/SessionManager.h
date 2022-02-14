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

#include <crypto/RandUtils.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/secure_channel/Constants.h>
#include <transport/CryptoContext.h>
#include <transport/GroupSession.h>
#include <transport/MessageCounterManagerInterface.h>
#include <transport/SecureSessionTable.h>
#include <transport/SessionDelegate.h>
#include <transport/SessionHandle.h>
#include <transport/SessionHolder.h>
#include <transport/SessionMessageDelegate.h>
#include <transport/TransportMgr.h>
#include <transport/UnauthenticatedSessionTable.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/Tuple.h>

namespace chip {

class PairingSession;

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

    uint32_t GetMessageCounter() const;

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

class DLL_EXPORT SessionManager : public TransportMgrDelegate
{
public:
    SessionManager();
    ~SessionManager() override;

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
    CHIP_ERROR PrepareMessage(const SessionHandle & session, PayloadHeader & payloadHeader, System::PacketBufferHandle && msgBuf,
                              EncryptedPacketBufferHandle & encryptedMessage);

    /**
     * @brief
     *   Send a prepared message to a currently connected peer.
     */
    CHIP_ERROR SendPreparedMessage(const SessionHandle & session, const EncryptedPacketBufferHandle & preparedMessage);

    /// @brief Set the delegate for handling incoming messages. There can be only one message delegate (probably the
    /// ExchangeManager)
    void SetMessageDelegate(SessionMessageDelegate * cb) { mCB = cb; }

    void RegisterRecoveryDelegate(SessionRecoveryDelegate & cb);
    void UnregisterRecoveryDelegate(SessionRecoveryDelegate & cb);
    void RefreshSessionOperationalData(const SessionHandle & sessionHandle);

    /**
     * @brief
     *   Establish a new pairing with a peer node
     *
     * @details
     *   This method sets up a new pairing with the peer node. It also
     *   establishes the security keys for secure communication with the
     *   peer node.
     */
    CHIP_ERROR NewPairing(SessionHolder & sessionHolder, const Optional<Transport::PeerAddress> & peerAddr, NodeId peerNodeId,
                          PairingSession * pairing, CryptoContext::SessionRole direction, FabricIndex fabric);

    void ExpirePairing(const SessionHandle & session);
    void ExpireAllPairings(NodeId peerNodeId, FabricIndex fabric);
    void ExpireAllPairingsForFabric(FabricIndex fabric);

    /**
     * @brief
     *   Return the System Layer pointer used by current SessionManager.
     */
    System::Layer * SystemLayer() { return mSystemLayer; }

    /**
     * @brief
     *   Initialize a Secure Session Manager
     *
     * @param systemLayer           System, layer to use
     * @param transportMgr          Transport to use
     * @param messageCounterManager The message counter manager
     */
    CHIP_ERROR Init(System::Layer * systemLayer, TransportMgrBase * transportMgr,
                    Transport::MessageCounterManagerInterface * messageCounterManager,
                    chip::PersistentStorageDelegate * storageDelegate);

    /**
     * @brief
     *  Shutdown the Secure Session Manager. This terminates this instance
     *  of the object and reset it's state.
     */
    void Shutdown();

    TransportMgrBase * GetTransportManager() const { return mTransportMgr; }

    /**
     * @brief
     *   Handle received secure message. Implements TransportMgrDelegate
     *
     * @param source    the source address of the package
     * @param msgBuf    the buffer containing a full CHIP message (except for the optional length field).
     */
    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf) override;

    Optional<SessionHandle> CreateUnauthenticatedSession(const Transport::PeerAddress & peerAddress,
                                                         const ReliableMessageProtocolConfig & config)
    {
        // Allocate ephemeralInitiatorNodeID in Operational Node ID range
        NodeId ephemeralInitiatorNodeID;
        do
        {
            ephemeralInitiatorNodeID = static_cast<NodeId>(Crypto::GetRandU64());
        } while (!IsOperationalNodeId(ephemeralInitiatorNodeID));
        return mUnauthenticatedSessions.AllocInitiator(ephemeralInitiatorNodeID, peerAddress, config);
    }

    // TODO: implements group sessions
    Optional<SessionHandle> CreateGroupSession(GroupId group, chip::FabricIndex fabricIndex, NodeId sourceNodeId)
    {
        return mGroupSessions.AllocEntry(group, fabricIndex, sourceNodeId);
    }
    Optional<SessionHandle> FindGroupSession(GroupId group, chip::FabricIndex fabricIndex)
    {
        return mGroupSessions.FindEntry(group, fabricIndex);
    }
    void RemoveGroupSession(Transport::GroupSession * session) { mGroupSessions.DeleteEntry(session); }

    // TODO: this is a temporary solution for legacy tests which use nodeId to send packets
    // and tv-casting-app that uses the TV's node ID to find the associated secure session
    SessionHandle FindSecureSessionForNode(NodeId peerNodeId);

    using SessionHandleCallback = bool (*)(void * context, SessionHandle & sessionHandle);
    CHIP_ERROR ForEachSessionHandle(void * context, SessionHandleCallback callback);

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
    Transport::UnauthenticatedSessionTable<CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE> mUnauthenticatedSessions;
    Transport::SecureSessionTable<CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE> mSecureSessions;
    Transport::GroupSessionTable<CHIP_CONFIG_GROUP_CONNECTION_POOL_SIZE> mGroupSessions;
    State mState; // < Initialization state of the object

    SessionMessageDelegate * mCB = nullptr;

    ObjectPool<std::reference_wrapper<SessionRecoveryDelegate>, CHIP_CONFIG_MAX_SESSION_RECOVERY_DELEGATES>
        mSessionRecoveryDelegates;

    TransportMgrBase * mTransportMgr                                   = nullptr;
    Transport::MessageCounterManagerInterface * mMessageCounterManager = nullptr;

    // Will be use once PR 14996 is merged
    chip::PersistentStorageDelegate * mStorage = nullptr;

    GlobalUnencryptedMessageCounter mGlobalUnencryptedMessageCounter;
    GlobalEncryptedMessageCounter mGlobalEncryptedMessageCounter;

    friend class SessionHandle;

    /** Schedules a new oneshot timer for checking connection expiry. */
    void ScheduleExpiryTimer();

    /** Cancels any active timers for connection expiry checks. */
    void CancelExpiryTimer();

    /**
     * Callback for timer expiry check
     */
    static void ExpiryTimerCallback(System::Layer * layer, void * param);

    void SecureUnicastMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                      System::PacketBufferHandle && msg);

    void SecureGroupMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                    System::PacketBufferHandle && msg);

    void UnauthenticatedMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                        System::PacketBufferHandle && msg);

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source);

    static bool IsControlMessage(PayloadHeader & payloadHeader)
    {
        return payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq) ||
            payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp);
    }

    MessageCounter & GetSendCounterForPacket(PayloadHeader & payloadHeader, Transport::SecureSession & state)
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
