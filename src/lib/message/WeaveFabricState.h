/*
 *
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
 *      This file defines data types and objects for managing the
 *      active node state necessary to participate in a Weave fabric.
 *
 */

// Include WeaveCore.h OUTSIDE of the include guard for WeaveFabricState.h.
// This allows WeaveCore.h to enforce a canonical include order for core
// header files, making it easier to manage dependencies between these files.
#include <Weave/Core/WeaveCore.h>

#ifndef WEAVE_FABRIC_STATE_H
#define WEAVE_FABRIC_STATE_H

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inttypes.h>

#include <Weave/Support/NLDLLUtil.h>
#include <Weave/Support/WeaveCounter.h>
#include <Weave/Support/PersistedCounter.h>
#include <Weave/Support/FlagUtils.hpp>
#include <Weave/Core/WeaveKeyIds.h>
#include <Weave/Profiles/security/WeaveSecurity.h>
#include <Weave/Profiles/security/WeaveApplicationKeys.h>

namespace nl {
namespace Weave {

class NL_DLL_EXPORT WeaveConnection;
class NL_DLL_EXPORT WeaveMessageLayer;
class NL_DLL_EXPORT WeaveExchangeManager;
struct WeaveMessageInfo;

// Special node id values.
enum
{
    kNodeIdNotSpecified                                 = 0ULL,
    kAnyNodeId                                          = 0xFFFFFFFFFFFFFFFFULL
};

// Special fabric id values.
enum
{
    kFabricIdNotSpecified                               = 0ULL,

    /** Default fabric ID, which should only be used for testing purposes. */
    kFabricIdDefaultForTest                             = 1ULL,

    // Ids >= kReservedFabricIdStart and <= kMaxFabricId are reserved for special uses.  The
    // meanings of values in this range are context-specific; e.g. in the IdentifyRequest
    // message, these value are used to match devices that are/are not members of a fabric.
    kReservedFabricIdStart                              = 0xFFFFFFFFFFFFFF00ULL,

    kMaxFabricId                                        = 0xFFFFFFFFFFFFFFFFULL
};

/** Identifies the purpose or application of certificate
 *
 * A certificate type is a label that describes a certificate's purpose or application.
 * Certificate types are not carried as attributes of the corresponding certificates, but
 * rather are derived from the certificate's structure and/or the context in which it is used.
 * The certificate type enumeration includes a set of pre-defined values describing commonly
 * used certificate applications.  Developers can also extend the certificate type value
 * range with application-specific types that described custom certificates or certificates
 * with unique security properties.
 *
 * Certificate types are primarily used in the implementation of access control policies,
 * where access to application features is influenced by the type of certificate presented
 * by a requester.
 *
 * @note Cert type is an API data type only; it should never be sent over-the-wire.
 */
enum
{
    // NOTE: When adding additional types, please update the PrintCertType() function.

    kCertType_NotSpecified                              = 0x00, /**< The certificate's type has not been specified. */
    kCertType_General                                   = 0x01, /**< The certificate is of a general or non-specific type. */
    kCertType_Device                                    = 0x02, /**< A Weave device certificate. */
    kCertType_ServiceEndpoint                           = 0x03, /**< A Weave service endpoint certificate. */
    kCertType_FirmwareSigning                           = 0x04, /**< A Weave firmware signing certificate. */
    kCertType_AccessToken                               = 0x05, /**< A Weave access token certificate. */
    kCertType_CA                                        = 0x06, /**< A CA certificate. */

    kCertType_AppDefinedBase                            = 0x7F, /**< Application-specific certificate types should have values >= this value. */

    kCertType_Max                                       = 0xFF, /**< Certificate types should not be greater than this value. */
};

/** Identifies the source for the password used in a password-based authentication protocol (e.g. Weave PASE)
 *
 * @note Password Source is protocol data type; do not renumber.
 */
enum
{
    kPasswordSource_NotSpecified                        = 0x0,
    kPasswordSource_PairingCode                         = 0x1,

    kPasswordSource_Max                                 = 0xF,
};


/** Identifies how a peer node is authenticated.
 *
 * WeaveAuthMode describes the means by which a peer node has been, or should be,
 * authenticated during a Weave message exchange.  In an initiating context,
 * applications use WeaveAuthMode to express a desired peer authentication mode
 * for an exchange, thereby instructing the Weave security and messaging layers
 * to achieve the desired mode or fail the communication.  In a responding context,
 * WeaveAuthMode identifies how the requesting node (the peer) was authenticated,
 * allowing the responding application to enforce access controls based on this
 * information.
 *
 * The WeaveAuthMode assigned to an incoming Weave message is related to the nature
 * of the key that was used to encrypt that message.  The WeaveAuthMode derives from
 * the mechanism by which the key was established and the criteria used to verify the
 * identities of the communicating parties at the time of key establishment.
 *
 * WeaveAuthMode includes a set of pre-defined values describing common
 * authentication modes.  These are broken down by the key agreement mechanism
 * (CASE, PASE, GroupKey, etc.).  Developers can extend WeaveAuthMode by defining
 * application-specific modes, which they can attach to specific encryption keys.
 *
 * @note WeaveAuthMode is an API data type only; it should never be sent over-the-wire.
 */
typedef uint16_t WeaveAuthMode;
enum
{
    kWeaveAuthMode_NotSpecified                         = 0x0000, /**< Authentication mode not specified. */

    // ===== Major authentication categories =====
    kWeaveAuthModeCategory_General                      = 0x0000, /**< Peer authenticated using one of a set of general mechanisms */
    kWeaveAuthModeCategory_PASE                         = 0x1000, /**< Peer authenticated using Weave PASE protocol */
    kWeaveAuthModeCategory_CASE                         = 0x2000, /**< Peer authenticated using Weave CASE protocol */
    kWeaveAuthModeCategory_TAKE                         = 0x3000, /**< Peer authenticated using Weave TAKE protocol */
    kWeaveAuthModeCategory_GroupKey                     = 0x4000, /**< Peer authenticated using a shared group key */
    kWeaveAuthModeCategory_AppDefinedBase               = 0xC000, /**< Base value for application-defined authentication categories */

    // ===== General authentication modes =====
    kWeaveAuthMode_Unauthenticated                      = kWeaveAuthModeCategory_General | 0x001,
                                                                  /**< Peer not authenticated. */

    // ===== PASE authentication modes =====
    kWeaveAuthMode_PASE_PairingCode                     = kWeaveAuthModeCategory_PASE | kPasswordSource_PairingCode,
                                                                  /**< Peer authenticated using PASE with device pairing code. */

    // ===== CASE authentication modes =====
    kWeaveAuthMode_CASE_AnyCert                         = kWeaveAuthModeCategory_CASE | kCertType_NotSpecified,
                                                                  /**< Peer authenticated using CASE with arbitrary certificate, or certificate of unknown type. */
    kWeaveAuthMode_CASE_GeneralCert                     = kWeaveAuthModeCategory_CASE | kCertType_General,
                                                                  /**< Peer authenticated using CASE with a general, or non-specific certificate type. */
    kWeaveAuthMode_CASE_Device                          = kWeaveAuthModeCategory_CASE | kCertType_Device,
                                                                  /**< Peer authenticated using CASE with Weave device certificate. */
    kWeaveAuthMode_CASE_ServiceEndPoint                 = kWeaveAuthModeCategory_CASE | kCertType_ServiceEndpoint,
                                                                  /**< Peer authenticated using CASE with Weave service endpoint certificate. */
    kWeaveAuthMode_CASE_AccessToken                     = kWeaveAuthModeCategory_CASE | kCertType_AccessToken,
                                                                  /**< Peer authenticated using CASE with Weave access token certificate. */

    // ===== TAKE authentication modes =====
    kWeaveAuthMode_TAKE_IdentificationKey               = kWeaveAuthModeCategory_TAKE | 0x001,
                                                                /**< Peer authenticated using TAKE with a token identification key. */

    // ===== Subfield Masks =====
    kWeaveAuthModeCategory_Mask                         = 0xF000,
    kWeaveAuthMode_PASE_PasswordSourceMask              = kPasswordSource_Max,
    kWeaveAuthMode_CASE_CertTypeMask                    = kCertType_Max,
    kWeaveAuthMode_GroupKey_AppGroupLocalNumber         = WeaveKeyId::kGroupLocalNumber_Max,

    // ===== DEPRECATED ALIASES =====
    kWeaveAuthMode_None                                 = kWeaveAuthMode_Unauthenticated,
    kWeaveAuthMode_Password_PairingCode                 = kWeaveAuthMode_PASE_PairingCode,
    kWeaveAuthMode_Cert_Device                          = kWeaveAuthMode_CASE_Device,
    kWeaveAuthMode_Cert_ServiceEndPoint                 = kWeaveAuthMode_CASE_ServiceEndPoint,
};

/** True if the authentication mode is based on the Weave PASE protocol. */
inline bool IsPASEAuthMode(WeaveAuthMode authMode) { return (authMode & kWeaveAuthModeCategory_Mask) == kWeaveAuthModeCategory_PASE; }

/** True if the authentication mode is based on the Weave CASE protocol. */
inline bool IsCASEAuthMode(WeaveAuthMode authMode) { return (authMode & kWeaveAuthModeCategory_Mask) == kWeaveAuthModeCategory_CASE; }

/** True if the authentication mode is based on the Weave TAKE protocol. */
inline bool IsTAKEAuthMode(WeaveAuthMode authMode) { return (authMode & kWeaveAuthModeCategory_Mask) == kWeaveAuthModeCategory_TAKE; }

/** True if the authentication mode is based possession of a shared password. */
inline bool IsPasswordAuthMode(WeaveAuthMode authMode) { return IsPASEAuthMode(authMode); }

/** True if the authentication mode is based on possession of a private key associated with a certificate. */
inline bool IsCertAuthMode(WeaveAuthMode authMode) { return IsCASEAuthMode(authMode); }

/** True if the authentication mode is based on possession of a shared group key. */
inline bool IsGroupKeyAuthMode(WeaveAuthMode authMode) { return (authMode & kWeaveAuthModeCategory_Mask) == kWeaveAuthModeCategory_GroupKey; }

/** Returns the password source for the given authentication mode.
 *
 *  @note The result is only valid when the supplied mode is one of the password authentication modes.
 */
inline uint8_t PasswordSourceFromAuthMode(WeaveAuthMode authMode) { return (uint8_t)(authMode & kWeaveAuthMode_PASE_PasswordSourceMask); }

/** Returns the password source for the given authentication mode.
 *
 *  @note The result is only valid when the supplied mode is one of the certificate authentication modes.
 */
inline uint8_t CertTypeFromAuthMode(WeaveAuthMode authMode) { return (uint8_t)(authMode & kWeaveAuthMode_CASE_CertTypeMask); }

/** Returns the application group master key ID associated with the authentication mode.
 *
 *  @note The result is only valid when the supplied mode is one of the group key modes.
 */
inline uint8_t AppGroupMasterKeyIdFromAuthMode(WeaveAuthMode authMode) { return WeaveKeyId::MakeAppGroupMasterKeyId(authMode & kWeaveAuthMode_GroupKey_AppGroupLocalNumber); }

/** Returns a corresponding PASE authentication mode for a given password source. */
inline WeaveAuthMode PASEAuthMode(uint8_t pwSource) { return kWeaveAuthModeCategory_PASE | pwSource; }

/** Returns a corresponding CASE authentication mode for a given certificate type. */
inline WeaveAuthMode CASEAuthMode(uint8_t certType) { return kWeaveAuthModeCategory_CASE | certType; }

/** Returns a corresponding group key authentication mode for a given key ID.
 *
 *  @note The result is only valid when the supplied key ID is an application group key.
 */
inline WeaveAuthMode GroupKeyAuthMode(uint32_t keyId) { return kWeaveAuthModeCategory_GroupKey | WeaveKeyId::GetAppGroupLocalNumber(keyId); }


/**
 * @typedef FabricSecretScope
 *
 * @brief
 *   Identifies the category of devices that can possess and use fabric secret.
 *
 * @note FabricSecretScope is protocol data type; do not renumber.
 */
typedef uint8_t FabricSecretScope;
enum
{
    kFabricSecretScope_All                              = 0x00  /**< All devices can possess corresponding fabric secret. */
};

// Encryption key for the AES-128-CTR-SHA-1 message encryption type
class WeaveEncryptionKey_AES128CTRSHA1
{
public:
    enum
    {
        DataKeySize                                     = 16,
        DataKeySizeInWords                              = DataKeySize/sizeof(uint32_t),
        IntegrityKeySize                                = 20,
        KeySize                                         = DataKeySize + IntegrityKeySize
    };

    union {
        uint8_t DataKey[DataKeySize];
        uint32_t DataKeyWords[DataKeySizeInWords];
    };
    uint8_t IntegrityKey[IntegrityKeySize];
};

// Represents a key or key set used to encrypt Weave messages.
typedef union WeaveEncryptionKey
{
    WeaveEncryptionKey_AES128CTRSHA1 AES128CTRSHA1;
} WeaveEncryptionKey;

// AES128CTRSHA1 encryption and integrity test keys, which should only be used for testing purposes.
enum
{
    kTestKey_AES128CTRSHA1_DataKeyByte                  = 0xAB,  /**< Byte value that constructs encryption key, which is used only for testing. */
    kTestKey_AES128CTRSHA1_IntegrityKeyByte             = 0xBA   /**< Byte value that constructs integrity key, which is used only for testing. */
};

/**
 * @class WeaveMsgEncryptionKey
 *
 * @brief
 *    Contains information about Weave message encryption key.
 *
 */
class WeaveMsgEncryptionKey
{
public:
    uint16_t KeyId;                                     /**< The key ID. */
    uint8_t EncType;                                    /**< The encryption type supported by the key. */
    WeaveEncryptionKey EncKey;                          /**< The secret key material. */
};

/**
 * @class WeaveSessionState
 *
 * @brief
 *   Conveys the communication state needed to send/receive messages with another node.
 */
class WeaveSessionState
{
public:

    typedef uint16_t ReceiveFlagsType;

    enum
    {
        kReceiveFlags_NumMessageIdFlags                 = (sizeof(ReceiveFlagsType) * 8) - 1,
        kReceiveFlags_MessageIdSynchronized             = (1 << kReceiveFlags_NumMessageIdFlags),
        kReceiveFlags_MessageIdFlagsMask                = ~kReceiveFlags_MessageIdSynchronized
    };

    WeaveSessionState(void);
    WeaveSessionState(WeaveMsgEncryptionKey *msgEncKey, WeaveAuthMode authMode,
                      MonotonicallyIncreasingCounter *nextMsgId, uint32_t *maxRcvdMsgId, ReceiveFlagsType *rcvFlags);

    WeaveMsgEncryptionKey *MsgEncKey;
    WeaveAuthMode AuthMode;

    uint32_t NewMessageId(void);
    bool MessageIdNotSynchronized(void);
    bool IsDuplicateMessage(uint32_t msgId);

private:
    MonotonicallyIncreasingCounter *NextMsgId;
    uint32_t *MaxMsgIdRcvd;
    ReceiveFlagsType *RcvFlags;
};

/**
 * @class WeaveSessionKey
 *
 * @brief
 *   Contains information about a Weave session key.
 */
class WeaveSessionKey
{
public:
    enum FlagsEnum
    {
        kFlag_IsLocallyInitiated     = 0x01,            /**< The session was initiated by the local node. */
        kFlag_IsSharedSession        = 0x02,            /**< The session is a shared session. */
        kFlag_IsRemoveOnIdle         = 0x04,            /**< The session should be removed when idle (only applies to sessions
                                                             that are not bound to a connection). */
        kFlag_RecentlyActive         = 0x08,            /**< The session was recently active. */
        kFlag_Suspended              = 0x10,            /**< The session has been suspended. */
    };

    uint64_t NodeId;                                    /**< The id of the node with which the session key is shared. */
    MonotonicallyIncreasingCounter NextMsgId;           /**< The next message id to be used under the session key. */
    uint32_t MaxRcvdMsgId;                              /**< The maximum message id received under the session key. */
    WeaveConnection *BoundCon;                          /**< The connection to which the key is bound. */
    WeaveSessionState::ReceiveFlagsType RcvFlags;       /**< Flags tracking messages received under the key. */
    WeaveAuthMode AuthMode;                             /**< The means by which the peer node was authenticated during session establishment. */
    WeaveMsgEncryptionKey MsgEncKey;                    /**< The Weave message encryption key. */
    uint8_t ReserveCount;                               /**< Number of times the session key has been reserved. */
    uint8_t Flags;                                      /**< Various flags associated with the session. */

    void Init(void);
    void Clear(void);

    bool IsAllocated() const            { return MsgEncKey.KeyId != WeaveKeyId::kNone; }
    bool IsKeySet() const               { return MsgEncKey.EncType != 0; }
    bool IsLocallyInitiated() const     { return GetFlag(Flags, kFlag_IsLocallyInitiated); }
    void SetLocallyInitiated(bool val)  { SetFlag(Flags, kFlag_IsLocallyInitiated, val); }
    bool IsSharedSession() const        { return GetFlag(Flags, kFlag_IsSharedSession); }
    void SetSharedSession(bool val)     { SetFlag(Flags, kFlag_IsSharedSession, val); }
    bool IsRemoveOnIdle() const         { return GetFlag(Flags, kFlag_IsRemoveOnIdle); }
    void SetRemoveOnIdle(bool val)      { SetFlag(Flags, kFlag_IsRemoveOnIdle, val); }
    bool IsRecentlyActive() const       { return GetFlag(Flags, kFlag_RecentlyActive); }
    void MarkRecentlyActive()           { SetFlag(Flags, kFlag_RecentlyActive); }
    void ClearRecentlyActive()          { ClearFlag(Flags, kFlag_RecentlyActive); }
    bool IsSuspended() const            { return GetFlag(Flags, kFlag_Suspended); }
    void MarkSuspended()                { SetFlag(Flags, kFlag_Suspended); }
    void ClearSuspended()               { ClearFlag(Flags, kFlag_Suspended); }
};

/**
 * @class WeaveMsgEncryptionKeyCache
 *
 * @brief
 *   Key cache for Weave message encryption keys.
 */
class WeaveMsgEncryptionKeyCache
{
public:
    void Init(void);
    void Reset(void);
    void Shutdown(void);

    WeaveMsgEncryptionKey *FindOrAllocateKeyEntry(uint16_t keyId, uint8_t encType);

private:
    // Array of Weave message encryption keys.
    WeaveMsgEncryptionKey mKeyCache[WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS];
    // Array of key entry indexes in sorted order from most- to least- recently used.
    uint8_t mMostRecentlyUsedKeyEntries[WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS];

    void Clear(uint8_t keyEntryIndex);
};

/**
 *  @brief
 *    Key diversifier used for Weave message encryption key derivation. This value
 *    represents first 4 bytes of the SHA-1 HASH of "Nest Weave Message EK and AK" phrase.
 */
extern const uint8_t kWeaveMsgEncAppKeyDiversifier[4];

/**
 * Weave message encryption application key diversifier size.
 */
enum
{
    kWeaveMsgEncAppKeyDiversifierSize         = sizeof(kWeaveMsgEncAppKeyDiversifier) + sizeof(uint8_t)
};

/**
 * @class WeaveFabricState
 *
 * @brief
 *   Forward declaration of WeaveFabricState
 */
class WeaveFabricState;

/**
 * @class FabricStateDelegate
 *
 * @brief
 *   This abstract delegate class communicates fabric state changes.
 */
class FabricStateDelegate
{
public:
    /**
     * This method is called when WeaveFabricState joins or creates a new fabric.
     *
     * @param[in] fabricState: A pointer to WeaveFabricState that was changed.
     * @param[in] newFabricId: The new fabric ID of the WeaveFabricState.
     *
     */
    virtual void DidJoinFabric(WeaveFabricState *fabricState, uint64_t newFabricId) = 0;

    /**
     * This method is called when WeaveFabricState leaves a fabric (i.e., fabric state
     * is cleared).
     *
     * @param[in] fabricState: A pointer to the WeaveFabricState that was changed.
     * @param[in] oldFabricId: The old fabric ID that was cleared.
     *
     */
    virtual void DidLeaveFabric(WeaveFabricState *fabricState, uint64_t oldFabricId) = 0;
};

class NL_DLL_EXPORT WeaveFabricState
{
public:

#if WEAVE_CONFIG_MAX_PEER_NODES <= UINT8_MAX
    typedef uint8_t PeerIndexType;
#else
    typedef uint16_t PeerIndexType;
#endif

    enum State
    {
        kState_NotInitialized = 0, kState_Initialized = 1
    };

    WeaveFabricState(void);

    WeaveMessageLayer *MessageLayer;                    // [READ ONLY] The associated WeaveMessageLayer object.
    uint64_t FabricId;                                  // [READ ONLY] Node's Fabric Id (0 means node is not a member of a fabric).
    uint64_t LocalNodeId;
    const char *PairingCode;
    uint16_t DefaultSubnet;
    uint8_t State;                                      // [READ ONLY] State of the Weave Fabric State object
    nl::Weave::Profiles::Security::AppKeys::GroupKeyStoreBase *GroupKeyStore;

#if WEAVE_CONFIG_SECURITY_TEST_MODE
    uint64_t DebugFabricId;
    bool LogKeys;
    bool UseTestKey; // DEPRECATED -- Temporarily retained for API compaibility
    bool AutoCreateKeys; // DEPRECATED -- Temporarily retained for API compaibility
#endif

#if WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
    IPAddress ListenIPv4Addr;
    IPAddress ListenIPv6Addr;
#endif

    WEAVE_ERROR Init(void);
    WEAVE_ERROR Init(nl::Weave::Profiles::Security::AppKeys::GroupKeyStoreBase *groupKeyStore);
    WEAVE_ERROR Shutdown(void);

    WEAVE_ERROR AllocSessionKey(uint64_t peerNodeId, uint16_t keyId, WeaveConnection *boundCon, WeaveSessionKey *& sessionKey);
    WEAVE_ERROR SetSessionKey(uint16_t keyId, uint64_t peerNodeId, uint8_t encType, WeaveAuthMode authMode, const WeaveEncryptionKey *encKey);
    WEAVE_ERROR SetSessionKey(WeaveSessionKey *sessionKey, uint8_t encType, WeaveAuthMode authMode, const WeaveEncryptionKey *encKey);
    WEAVE_ERROR GetSessionKey(uint16_t keyId, uint64_t peerNodeId, WeaveSessionKey *& outSessionKey);
    WEAVE_ERROR FindSessionKey(uint16_t keyId, uint64_t peerNodeId, bool create, WeaveSessionKey *& retRec);
    WEAVE_ERROR RemoveSessionKey(uint16_t keyId, uint64_t peerNodeId);
    void RemoveSessionKey(WeaveSessionKey *sessionKey, bool wasIdle = false);
    bool RemoveIdleSessionKeys();

    WeaveSessionKey *FindSharedSession(uint64_t terminatingNodeId, WeaveAuthMode authMode, uint8_t encType);
    bool IsSharedSession(uint16_t keyId, uint64_t peerNodeId);
    WEAVE_ERROR AddSharedSessionEndNode(uint64_t endNodeId, uint64_t terminatingNodeId, uint16_t keyId);
    WEAVE_ERROR AddSharedSessionEndNode(WeaveSessionKey *sessionKey, uint64_t endNodeId);
    WEAVE_ERROR GetSharedSessionEndNodeIds(const WeaveSessionKey *sessionKey, uint64_t *endNodeIds,
                                           uint8_t endNodeIdsBufSize, uint8_t& endNodeIdsCount);
    void RemoveSharedSessionEndNodes(const WeaveSessionKey *sessionKey);

    WEAVE_ERROR SuspendSession(uint16_t keyId, uint64_t peerNodeId, uint8_t * buf, uint16_t bufSize, uint16_t & serializedSessionLen);
    WEAVE_ERROR RestoreSession(uint8_t * serializedSession, uint16_t serializedSessionLen);

    WEAVE_ERROR GetSessionState(uint64_t remoteNodeId, uint16_t keyId, uint8_t encType, WeaveConnection *con, WeaveSessionState& outSessionState);

    IPAddress SelectNodeAddress(uint64_t nodeId, uint16_t subnet) const;
    IPAddress SelectNodeAddress(uint64_t nodeId) const;
    bool IsFabricAddress(const IPAddress &addr) const;
    bool IsLocalFabricAddress(const IPAddress &addr) const;

    WEAVE_ERROR GetPassword(uint8_t pwSrc, const char *& ps, uint16_t& pwLen);

    WEAVE_ERROR CreateFabric(void);
    void ClearFabricState(void);
    WEAVE_ERROR GetFabricState(uint8_t *buf, uint32_t bufSize, uint32_t &fabricStateLen);
    WEAVE_ERROR JoinExistingFabric(const uint8_t *fabricState, uint32_t fabricStateLen);

    void HandleConnectionClosed(WeaveConnection *con);

    /**
     * This method sets the delegate object.
     * The callback methods of delegate are invoked whenever the FabricId is changed,
     * i.e., when we join/create a fabric, or when we leave a fabric (clear fabric state)
     *
     * If the delegate is previously set, then a second call to this method will overwrite
     * the previous delegate.
     *
     * @param[in] aDelegate                     The delegate object. It can be NULL if no
     *                                          delegate is required.
     */
    void SetDelegate(FabricStateDelegate *aDelegate);

#if WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    void OnMsgCounterSyncRespRcvd(uint64_t peerNodeId, uint32_t peerMsgId, uint32_t requestorMsgCounter);
    void OnMsgCounterSyncReqSent(uint32_t messageId);
    bool IsMsgCounterSyncReqInProgress(void);
    WEAVE_ERROR GetMsgEncKeyIdForAppGroup(uint32_t appGroupGlobalId, uint32_t rootKeyId, bool useRotatingKey, uint32_t& keyId);
    WEAVE_ERROR CheckMsgEncForAppGroup(const WeaveMessageInfo *msgInfo, uint32_t appGroupGlobalId, uint32_t rootKeyId, bool requireRotatingKey);
#endif // WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC

    typedef void (*SessionEndCbFunct)(uint16_t keyId, uint64_t peerNodeId, void *context);

    // Callback context provided by provisioning servers when registering with
    // WeaveFabricState to be notified when session ends.
    struct SessionEndCbCtxt
    {
        SessionEndCbFunct OnSessionRemoved;
        void *context;
        SessionEndCbCtxt *next;
    };

    WEAVE_ERROR RegisterSessionEndCallback(SessionEndCbCtxt *sessionEndCb);
private:
    PeerIndexType PeerCount;
    MonotonicallyIncreasingCounter NextUnencUDPMsgId;
    MonotonicallyIncreasingCounter NextUnencTCPMsgId;
    WeaveSessionKey SessionKeys[WEAVE_CONFIG_MAX_SESSION_KEYS];
#if WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    PersistedCounter NextGroupKeyMsgId;

    // The earliest message id that will be considered "fresh" in a message counter synchronization
    // response, but only if kReqInProgressFlag is true when the response is received.
    uint32_t GroupKeyMsgIdFreshWindowStart;

    // Status flags to control message counter synchronization protocol flow and the fresh window width.
    uint16_t MsgCounterSyncStatus;
    enum
    {
        // Flag indicates if there may be outstanding request for which we may receive a response.
        kFlag_ReqInProgress                             = 0x8000,

        // Flag indicates if request has been sent in the current timeout period.
        kFlag_ReqSentThisPeriod                         = 0x4000,

        // Mask for fresh group message counter window width.
        kMask_GroupKeyMsgIdFreshWindowWidth             = 0x3FFF,
    };

    WeaveMsgEncryptionKeyCache AppKeyCache;
#endif // WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    struct
    {
        uint64_t NodeId[WEAVE_CONFIG_MAX_PEER_NODES];
        uint32_t MaxUnencUDPMsgIdRcvd[WEAVE_CONFIG_MAX_PEER_NODES];
#if WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
        uint32_t MaxGroupKeyMsgIdRcvd[WEAVE_CONFIG_MAX_PEER_NODES];
        WeaveSessionState::ReceiveFlagsType GroupKeyRcvFlags[WEAVE_CONFIG_MAX_PEER_NODES];
#endif
        WeaveSessionState::ReceiveFlagsType UnencRcvFlags[WEAVE_CONFIG_MAX_PEER_NODES];
        // Array of peer indexes in sorted order from most- to least- recently used.
        PeerIndexType MostRecentlyUsedIndexes[WEAVE_CONFIG_MAX_PEER_NODES];
    } PeerStates;
    FabricStateDelegate *Delegate;

    // This structure contains information about shared session end node.
    struct SharedSessionEndNode
    {
        uint64_t EndNodeId;
        WeaveSessionKey *SessionKey;
    };
    // Record of all active shared session end nodes.
    SharedSessionEndNode SharedSessionsNodes[WEAVE_CONFIG_MAX_SHARED_SESSIONS_END_NODES];

    // Linked list of registered modules to be notified when session closes
    SessionEndCbCtxt *sessionEndCallbackList;

    void NotifySessionEndSubscribers(uint16_t keyId, uint64_t peerNodeId);

    bool FindSharedSessionEndNode(uint64_t endNodeId, const WeaveSessionKey *sessionKey);

#if WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    void StartMsgCounterSyncTimer(void);
    static void OnMsgCounterSyncRespTimeout(System::Layer* aSystemLayer, void* aAppState, System::Error aError);
#endif

    bool FindOrAllocPeerEntry(uint64_t peerNodeId, bool allocEntry, PeerIndexType& retPeerIndex);
    WEAVE_ERROR FindMsgEncAppKey(uint16_t keyId, uint8_t encType, WeaveMsgEncryptionKey *& retRec);
    WEAVE_ERROR DeriveMsgEncAppKey(uint32_t keyId, uint8_t encType, WeaveMsgEncryptionKey & appKey, uint32_t& appGroupGlobalId);
};

#if WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
/**
 * This method returns true if at least one peer's message counter
 * synchronization request is in progress.
 *
 * @retval bool                 Whether or not peer's message counter synchronization
 *                              is in progress.
 */
inline bool WeaveFabricState::IsMsgCounterSyncReqInProgress(void)
{
    return (MsgCounterSyncStatus & kFlag_ReqInProgress) != 0;
}
#endif



#if WEAVE_CONFIG_SECURITY_TEST_MODE

enum
{
    kMaxEncKeyStringSize   =   WeaveEncryptionKey_AES128CTRSHA1::DataKeySize * 2      // Hex digits for data key
                             + 1                                                      // Field separator (,)
                             + WeaveEncryptionKey_AES128CTRSHA1::IntegrityKeySize * 2 // Hex digits for integrity key
                             + 1,                                                     // Null terminator
};

extern void WeaveEncryptionKeyToString(uint8_t encType, const WeaveEncryptionKey& key, char *buf, size_t bufSize);

#endif // WEAVE_CONFIG_SECURITY_TEST_MODE





} // namespace nl
} // namespace Weave

#endif // WEAVE_FABRIC_STATE_H
