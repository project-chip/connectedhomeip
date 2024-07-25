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

#include <credentials/FabricTable.h>
#include <crypto/RandUtils.h>
#include <crypto/SessionKeystore.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/secure_channel/Constants.h>
#include <transport/CryptoContext.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/GroupSession.h>
#include <transport/MessageCounterManagerInterface.h>
#include <transport/SecureSessionTable.h>
#include <transport/Session.h>
#include <transport/SessionDelegate.h>
#include <transport/SessionHolder.h>
#include <transport/SessionMessageDelegate.h>
#include <transport/TransportMgr.h>
#include <transport/UnauthenticatedSessionTable.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/Tuple.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <transport/SessionConnectionDelegate.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

namespace chip {

/*
 * This enum indicates whether a session needs to be established over a
 * suitable transport that meets certain payload size requirements for
 * transmitted messages.
 *
 */
enum class TransportPayloadCapability : uint8_t
{
    kMRPPayload,               // Transport requires the maximum payload size to fit within a single
                               // IPv6 packet(1280 bytes).
    kLargePayload,             // Transport needs to handle payloads larger than the single IPv6
                               // packet, as supported by MRP. The transport of choice, in this
                               // case, is TCP.
    kMRPOrTCPCompatiblePayload // This option provides the ability to use MRP
                               // as the preferred transport, but use a large
                               // payload transport if that is already
                               // available.
};
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

class DLL_EXPORT SessionManager : public TransportMgrDelegate, public FabricTable::Delegate
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

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    void SetConnectionDelegate(SessionConnectionDelegate * cb) { mConnDelegate = cb; }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    // Test-only: create a session on the fly.
    CHIP_ERROR InjectPaseSessionWithTestKey(SessionHolder & sessionHolder, uint16_t localSessionId, NodeId peerNodeId,
                                            uint16_t peerSessionId, FabricIndex fabricIndex,
                                            const Transport::PeerAddress & peerAddress, CryptoContext::SessionRole role);
    CHIP_ERROR InjectCaseSessionWithTestKey(SessionHolder & sessionHolder, uint16_t localSessionId, uint16_t peerSessionId,
                                            NodeId localNodeId, NodeId peerNodeId, FabricIndex fabric,
                                            const Transport::PeerAddress & peerAddress, CryptoContext::SessionRole role,
                                            const CATValues & cats = CATValues{});

    /**
     * @brief
     *   Allocate a secure session and non-colliding session ID in the secure
     *   session table.
     *
     *   If we're either establishing or just finished establishing a session to a peer in either initiator or responder
     *   roles, the node id of that peer should be provided in sessionEvictionHint. Else, it should be initialized
     *   to a default-constructed ScopedNodeId().
     *
     * @return SessionHandle with a reference to a SecureSession, else NullOptional on failure
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> AllocateSession(Transport::SecureSession::Type secureSessionType,
                                            const ScopedNodeId & sessionEvictionHint);

    /**
     *  A set of templated helper function that call a provided lambda
     *  on all sessions in the underlying session table that match the provided
     *  query criteria.
     *
     */

    /**
     * Call the provided lambda on sessions whose remote side match the provided ScopedNodeId.
     *
     */
    template <typename Function>
    void ForEachMatchingSession(const ScopedNodeId & node, Function && function)
    {
        mSecureSessions.ForEachSession([&](auto * session) {
            if (session->GetPeer() == node)
            {
                function(session);
            }

            return Loop::Continue;
        });
    }

    /**
     * Call the provided lambda on sessions that match the provided fabric index.
     *
     */
    template <typename Function>
    void ForEachMatchingSession(FabricIndex fabricIndex, Function && function)
    {
        mSecureSessions.ForEachSession([&](auto * session) {
            if (session->GetFabricIndex() == fabricIndex)
            {
                function(session);
            }

            return Loop::Continue;
        });
    }

    /**
     * Call the provided lambda on all sessions whose remote side match the logical fabric
     * associated with the provided ScopedNodeId and target the same logical remote node.
     *
     * *NOTE* This is identical in behavior to ForEachMatchingSession(const ScopedNodeId ..)
     *        EXCEPT if there are multiple FabricInfo instances in the FabricTable that collide
     *        on the same logical fabric (i.e root public key + fabric ID tuple).
     *        This can ONLY happen if multiple controller instances on the same fabric is permitted
     *        and each is assigned a unique fabric index.
     */
    template <typename Function>
    CHIP_ERROR ForEachMatchingSessionOnLogicalFabric(const ScopedNodeId & node, Function && function)
    {
        Crypto::P256PublicKey targetPubKey;

        auto * targetFabric = mFabricTable->FindFabricWithIndex(node.GetFabricIndex());
        VerifyOrReturnError(targetFabric != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);

        auto err = targetFabric->FetchRootPubkey(targetPubKey);
        VerifyOrDie(err == CHIP_NO_ERROR);

        mSecureSessions.ForEachSession([&](auto * session) {
            Crypto::P256PublicKey comparePubKey;

            //
            // It's entirely possible to either come across a PASE session OR, a CASE session
            // that has yet to be activated (i.e a CASEServer holding onto a SecureSession object
            // waiting for a Sigma1 message to arrive). Let's skip those.
            //
            if (!session->IsCASESession() || session->GetFabricIndex() == kUndefinedFabricIndex)
            {
                return Loop::Continue;
            }

            auto * compareFabric = mFabricTable->FindFabricWithIndex(session->GetFabricIndex());
            VerifyOrDie(compareFabric != nullptr);

            err = compareFabric->FetchRootPubkey(comparePubKey);
            VerifyOrDie(err == CHIP_NO_ERROR);

            if (comparePubKey.Matches(targetPubKey) && targetFabric->GetFabricId() == compareFabric->GetFabricId() &&
                session->GetPeerNodeId() == node.GetNodeId())
            {
                function(session);
            }

            return Loop::Continue;
        });

        return CHIP_NO_ERROR;
    }

    /**
     * Call the provided lambda on all sessions that match the logical fabric
     * associated with the provided fabric index.
     *
     * *NOTE* This is identical in behavior to ForEachMatchingSession(FabricIndex ..)
     *        EXCEPT if there are multiple FabricInfo instances in the FabricTable that collide
     *        on the same logical fabric (i.e root public key + fabric ID tuple).
     *        This can ONLY happen if multiple controller instances on the same fabric is permitted
     *        and each is assigned a unique fabric index.
     */
    template <typename Function>
    CHIP_ERROR ForEachMatchingSessionOnLogicalFabric(FabricIndex fabricIndex, Function && function)
    {
        Crypto::P256PublicKey targetPubKey;

        auto * targetFabric = mFabricTable->FindFabricWithIndex(fabricIndex);
        VerifyOrReturnError(targetFabric != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);

        auto err = targetFabric->FetchRootPubkey(targetPubKey);
        VerifyOrDie(err == CHIP_NO_ERROR);

        mSecureSessions.ForEachSession([&](auto * session) {
            Crypto::P256PublicKey comparePubKey;

            //
            // It's entirely possible to either come across a PASE session OR, a CASE session
            // that has yet to be activated (i.e a CASEServer holding onto a SecureSession object
            // waiting for a Sigma1 message to arrive). Let's skip those.
            //
            if (!session->IsCASESession() || session->GetFabricIndex() == kUndefinedFabricIndex)
            {
                return Loop::Continue;
            }

            auto * compareFabric = mFabricTable->FindFabricWithIndex(session->GetFabricIndex());
            VerifyOrDie(compareFabric != nullptr);

            err = compareFabric->FetchRootPubkey(comparePubKey);
            VerifyOrDie(err == CHIP_NO_ERROR);

            if (comparePubKey.Matches(targetPubKey) && targetFabric->GetFabricId() == compareFabric->GetFabricId())
            {
                function(session);
            }

            return Loop::Continue;
        });

        return CHIP_NO_ERROR;
    }

    /**
     * Expire all sessions for a given peer, as identified by a specific fabric
     * index and node ID.
     */
    void ExpireAllSessions(const ScopedNodeId & node);

    /**
     * Expire all sessions associated with the given fabric index.
     *
     * *NOTE* This is generally all sessions for a given fabric _EXCEPT_ if there are multiple
     *        FabricInfo instances in the FabricTable that collide on the same logical fabric (i.e
     *        root public key + fabric ID tuple).  This can ONLY happen if multiple controller
     *        instances on the same fabric is permitted and each is assigned a unique fabric index.
     */
    void ExpireAllSessionsForFabric(FabricIndex fabricIndex);

    /**
     * Expire all sessions whose remote side matches the logical fabric
     * associated with the provided ScopedNodeId and target the same logical remote node.
     *
     * *NOTE* This is identical in behavior to ExpireAllSessions(const ScopedNodeId ..)
     *        EXCEPT if there are multiple FabricInfo instances in the FabricTable that collide
     *        on the same logical fabric (i.e root public key + fabric ID tuple).  This can ONLY happen
     *        if multiple controller instances on the same fabric is permitted and each is assigned
     *        a unique fabric index.
     *
     */
    CHIP_ERROR ExpireAllSessionsOnLogicalFabric(const ScopedNodeId & node);

    /**
     * Expire all sessions whose remote side matches the logical fabric
     * associated with the provided fabric index.
     *
     * *NOTE* This is identical in behavior to ExpireAllSessExpireAllSessionsForFabricions(FabricIndex ..)
     *        EXCEPT if there are multiple FabricInfo instances in the FabricTable that collide
     *        on the same logical fabric (i.e root public key + fabric ID tuple).  This can ONLY happen
     *        if multiple controller instances on the same fabric is permitted and each is assigned
     *        a unique fabric index.
     *
     */
    CHIP_ERROR ExpireAllSessionsOnLogicalFabric(FabricIndex fabricIndex);

    void ExpireAllPASESessions();

    /**
     * Expire all secure sessions.  See documentation for Shutdown on when it's
     * appropriate to use this.
     */
    void ExpireAllSecureSessions();

    /**
     * @brief
     *   Marks all active sessions that match provided arguments as defunct.
     *
     * @param node    Scoped node ID of the active sessions we should mark as defunct.
     * @param type    Type of session we are looking to mark as defunct. If matching
     *                against all types of sessions is desired, NullOptional should
     *                be passed into type.
     */
    void MarkSessionsAsDefunct(const ScopedNodeId & node, const Optional<Transport::SecureSession::Type> & type);

    /**
     * @brief
     *   Update all CASE sessions that match `node` with the provided transport peer address.
     *
     * @param node    Scoped node ID of the active sessions we want to update.
     * @param addr    Transport peer address that we want to update to.
     */
    void UpdateAllSessionsPeerAddress(const ScopedNodeId & node, const Transport::PeerAddress & addr);

    /**
     * @brief
     *   Return the System Layer pointer used by current SessionManager.
     */
    System::Layer * SystemLayer() { return mSystemLayer; }

    /**
     * @brief
     *   Initialize a Secure Session Manager
     *
     * @param systemLayer           System layer to use
     * @param transportMgr          Transport to use
     * @param messageCounterManager The message counter manager
     * @param storageDelegate       Persistent storage implementation
     * @param fabricTable           Fabric table to hold information about joined fabrics
     * @param sessionKeystore       Session keystore for management of symmetric encryption keys
     */
    CHIP_ERROR Init(System::Layer * systemLayer, TransportMgrBase * transportMgr,
                    Transport::MessageCounterManagerInterface * messageCounterManager,
                    chip::PersistentStorageDelegate * storageDelegate, FabricTable * fabricTable,
                    Crypto::SessionKeystore & sessionKeystore);

    /**
     * @brief
     *  Shutdown the Secure Session Manager. This terminates this instance
     *  of the object and reset it's state.
     *
     * The proper order of shutdown for SessionManager is as follows:
     *
     * 1) Call ExpireAllSecureSessions() on the SessionManager, and ensure that any unauthenticated
     *    sessions (e.g. CASEServer and CASESessionManager instances, or anything that does PASE
     *    handshakes) are released.
     * 2) Shut down the exchange manager, so that it's no longer referencing
     *    the to-be-shut-down SessionManager.
     * 3) Shut down the SessionManager.
     */
    void Shutdown();

    /**
     * @brief Notification that a fabric was removed.
     */
    void FabricRemoved(FabricIndex fabricIndex);

    TransportMgrBase * GetTransportManager() const { return mTransportMgr; }
    Transport::SecureSessionTable & GetSecureSessions() { return mSecureSessions; }

    /**
     * @brief
     *   Handle received secure message. Implements TransportMgrDelegate
     *
     * @param source    the source address of the package
     * @param msgBuf    the buffer containing a full CHIP message (except for the optional length field).
     * @param ctxt      pointer to additional context on the underlying transport. For TCP, it is a pointer
     *                  to the underlying connection object.
     */
    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf,
                           Transport::MessageTransportContext * ctxt = nullptr) override;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    CHIP_ERROR TCPConnect(const Transport::PeerAddress & peerAddress, Transport::AppTCPConnectionCallbackCtxt * appState,
                          Transport::ActiveTCPConnectionState ** peerConnState);

    CHIP_ERROR TCPDisconnect(const Transport::PeerAddress & peerAddress);

    void TCPDisconnect(Transport::ActiveTCPConnectionState * conn, bool shouldAbort = 0);

    void HandleConnectionReceived(Transport::ActiveTCPConnectionState * conn) override;

    void HandleConnectionAttemptComplete(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr) override;

    void HandleConnectionClosed(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr) override;

    // Functors for callbacks into higher layers
    using OnTCPConnectionReceivedCallback = void (*)(Transport::ActiveTCPConnectionState * conn);

    using OnTCPConnectionCompleteCallback = void (*)(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr);

    using OnTCPConnectionClosedCallback = void (*)(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr);

#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

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

    //
    // Find an existing secure session given a peer's scoped NodeId and a type of session to match against.
    // If matching against all types of sessions is desired, NullOptional should be passed into type.
    //
    // If a valid session is found, an Optional<SessionHandle> with the value set to the SessionHandle of the session
    // is returned. Otherwise, an Optional<SessionHandle> with no value set is returned.
    //
    //
    Optional<SessionHandle>
    FindSecureSessionForNode(ScopedNodeId peerNodeId, const Optional<Transport::SecureSession::Type> & type = NullOptional,
                             TransportPayloadCapability transportPayloadCapability = TransportPayloadCapability::kMRPPayload);

    using SessionHandleCallback = bool (*)(void * context, SessionHandle & sessionHandle);
    CHIP_ERROR ForEachSessionHandle(void * context, SessionHandleCallback callback);

    //// FabricTable::Delegate Implementation ////
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        (void) fabricTable;
        this->FabricRemoved(fabricIndex);
    }

    FabricTable * GetFabricTable() const { return mFabricTable; }

    Crypto::SessionKeystore * GetSessionKeystore() const { return mSessionKeystore; }

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

    System::Layer * mSystemLayer               = nullptr;
    FabricTable * mFabricTable                 = nullptr;
    Crypto::SessionKeystore * mSessionKeystore = nullptr;
    Transport::UnauthenticatedSessionTable<CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE> mUnauthenticatedSessions;
    Transport::SecureSessionTable mSecureSessions;
    State mState; // < Initialization state of the object
    chip::Transport::GroupOutgoingCounters mGroupClientCounter;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    OnTCPConnectionReceivedCallback mConnReceivedCb = nullptr;
    OnTCPConnectionCompleteCallback mConnCompleteCb = nullptr;
    OnTCPConnectionClosedCallback mConnClosedCb     = nullptr;

    // Hold the TCPConnection callback context for the receiver application in the SessionManager.
    // On receipt of a connection from a peer, the SessionManager
    Transport::AppTCPConnectionCallbackCtxt * mServerTCPConnCbCtxt = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    SessionMessageDelegate * mCB = nullptr;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    SessionConnectionDelegate * mConnDelegate = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    TransportMgrBase * mTransportMgr                                   = nullptr;
    Transport::MessageCounterManagerInterface * mMessageCounterManager = nullptr;

    GlobalUnencryptedMessageCounter mGlobalUnencryptedMessageCounter;

    /**
     * @brief Parse, decrypt, validate, and dispatch a secure unicast message.
     *
     * @param[in] partialPacketHeader The partial PacketHeader of the message after processing with DecodeFixed.
     * If the message decrypts successfully, this will be filled with a fully decoded PacketHeader.
     * @param[in] peerAddress The PeerAddress of the message as provided by the receiving Transport Endpoint.
     * @param msg The full message buffer, including header fields.
     * @param ctxt The pointer to additional context on the underlying transport. For TCP, it is a pointer
     *             to the underlying connection object.
     */
    void SecureUnicastMessageDispatch(const PacketHeader & partialPacketHeader, const Transport::PeerAddress & peerAddress,
                                      System::PacketBufferHandle && msg, Transport::MessageTransportContext * ctxt = nullptr);

    /**
     * @brief Parse, decrypt, validate, and dispatch a secure group message.
     *
     * @param partialPacketHeader The partial PacketHeader of the message once processed with DecodeFixed.
     * @param peerAddress The PeerAddress of the message as provided by the receiving Transport Endpoint.
     * @param msg The full message buffer, including header fields.
     */
    void SecureGroupMessageDispatch(const PacketHeader & partialPacketHeader, const Transport::PeerAddress & peerAddress,
                                    System::PacketBufferHandle && msg);

    /**
     * @brief Parse, decrypt, validate, and dispatch an unsecured message.
     *
     * @param partialPacketHeader The partial PacketHeader of the message after processing with DecodeFixed.
     * @param peerAddress The PeerAddress of the message as provided by the receiving Transport Endpoint.
     * @param msg The full message buffer, including header fields.
     * @param ctxt The pointer to additional context on the underlying transport. For TCP, it is a pointer
     *             to the underlying connection object.
     */
    void UnauthenticatedMessageDispatch(const PacketHeader & partialPacketHeader, const Transport::PeerAddress & peerAddress,
                                        System::PacketBufferHandle && msg, Transport::MessageTransportContext * ctxt = nullptr);

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    void MarkSecureSessionOverTCPForEviction(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    static bool IsControlMessage(PayloadHeader & payloadHeader)
    {
        return payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq) ||
            payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp);
    }
};

namespace MessagePacketBuffer {
/**
 * Maximum size of a message footer, in bytes.
 */
inline constexpr uint16_t kMaxFooterSize = kMaxTagLen;

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
