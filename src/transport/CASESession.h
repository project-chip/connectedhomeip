/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 */

#pragma once

#include <credentials/CHIPCert.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <crypto/CHIPCryptoPAL.h>
#include <protocols/secure_channel/Constants.h>
#include <support/Base64.h>
#include <system/SystemPacketBuffer.h>
#include <transport/PeerConnectionState.h>
#include <transport/SecureSession.h>
#include <transport/SessionEstablishmentDelegate.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

constexpr uint16_t kAEADKeySize = 16;

constexpr uint16_t kSigmaParamRandomNumberSize = 32;
constexpr uint16_t kTrustedRootIdSize          = 20;
constexpr uint16_t kMaxTrustedRootIds          = 5;

constexpr uint16_t kIPKSize = 32;

using namespace Crypto;
using namespace Credentials;

struct CASESessionSerialized;

struct CASESessionSerializable
{
    uint16_t mSharedSecretLen;
    uint8_t mSharedSecret[kMax_ECDH_Secret_Length];
    uint16_t mMessageDigestLen;
    uint8_t mMessageDigest[kSHA256_Hash_Length];
    uint8_t mPairingComplete;
    NodeId mLocalNodeId;
    NodeId mPeerNodeId;
    uint16_t mLocalKeyId;
    uint16_t mPeerKeyId;
};

class DLL_EXPORT CASESession
{
public:
    CASESession();
    CASESession(CASESession &&)      = default;
    CASESession(const CASESession &) = default;
    CASESession & operator=(const CASESession &) = default;
    CASESession & operator=(CASESession &&) = default;

    virtual ~CASESession();

    /**
     * @brief
     *   Initialize using operational credentials code and wait for session establishment requests.
     *
     * @param operationalCredentialSet      CHIP Certificate Set used to store the chain root of trust an validate peer node
     *                                      certificates
     * @param myNodeId                      Node id of local node
     * @param myKeyId                       Key ID to be assigned to the secure session on the peer node
     * @param delegate                      Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR WaitForSessionEstablishment(OperationalCredentialSet * operationalCredentialSet, Optional<NodeId> myNodeId,
                                           uint16_t myKeyId, SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Create and send session establishment request using device's operational credentials.
     *
     * @param peerAddress                   Address of peer with which to establish a session.
     * @param operationalCredentialSet      CHIP Certificate Set used to store the chain root of trust an validate peer node
     *                                      certificates
     * @param myNodeId                      Node id of local node
     * @param peerNodeId                    Node id of the peer node
     * @param myKeyId                       Key ID to be assigned to the secure session on the peer node
     * @param delegate                      Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR EstablishSession(const Transport::PeerAddress peerAddress, OperationalCredentialSet * operationalCredentialSet,
                                Optional<NodeId> myNodeId, NodeId peerNodeId, uint16_t myKeyId,
                                SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Derive a secure session from the established session. The API will return error
     *   if called before session is established.
     *
     * @param info        Information string used for key derivation
     * @param info_len    Length of info string
     * @param session     Reference to the secure session that will be
     *                    initialized once session establishment is complete
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(const uint8_t * info, size_t info_len, SecureSession & session);

    /**
     * @brief
     *   Handler for peer's messages, exchanged during pairing handshake.
     *
     * @param packetHeader Message header for the received message
     * @param peerAddress  Source of the message
     * @param msg          Message sent by the peer
     * @return CHIP_ERROR The result of message processing
     */
    virtual CHIP_ERROR HandlePeerMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                         System::PacketBufferHandle msg);

    /**
     * @brief
     *  Return the associated secure session peer NodeId
     *
     * @return NodeId The associated peer NodeId
     */
    NodeId GetPeerNodeId() const { return mConnectionState.GetPeerNodeId(); }

    /**
     * @brief
     *  Return the associated peer key id
     *
     * @return uint16_t The associated peer key id
     */
    uint16_t GetPeerKeyId() { return mConnectionState.GetPeerKeyID(); }

    /**
     * @brief
     *  Return the associated local key id
     *
     * @return uint16_t The assocated local key id
     */
    uint16_t GetLocalKeyId() { return mConnectionState.GetLocalKeyID(); }

    Transport::PeerConnectionState & PeerConnection() { return mConnectionState; }

    /**
     * @brief Serialize the Pairing Session to a string.
     **/
    CHIP_ERROR Serialize(CASESessionSerialized & output);

    /**
     * @brief Deserialize the Pairing Session from the string.
     **/
    CHIP_ERROR Deserialize(CASESessionSerialized & input);

    /**
     * @brief Serialize the CASESession to the given serializable data structure for secure pairing
     **/
    CHIP_ERROR ToSerializable(CASESessionSerializable & output);

    /**
     * @brief Reconstruct secure pairing class from the serializable data structure.
     **/
    CHIP_ERROR FromSerializable(const CASESessionSerializable & output);

private:
    enum SigmaErrorType : uint8_t
    {
        kNoSharedTrustRoots   = 0x01,
        kInvalidSignature     = 0x04,
        kInvalidResumptionTag = 0x05,
        kUnsupportedVersion   = 0x06,
        kUnexpected           = 0xff,
    };

    CHIP_ERROR Init(OperationalCredentialSet * operationalCredentialSet, Optional<NodeId> myNodeId, uint16_t myKeyId,
                    SessionEstablishmentDelegate * delegate);

    CHIP_ERROR SendSigmaR1();
    CHIP_ERROR HandleSigmaR1_and_SendSigmaR2(const PacketHeader & header, const System::PacketBufferHandle & msg);
    CHIP_ERROR HandleSigmaR1(const PacketHeader & header, const System::PacketBufferHandle & msg);
    CHIP_ERROR SendSigmaR2();
    CHIP_ERROR HandleSigmaR2_and_SendSigmaR3(const PacketHeader & header, const System::PacketBufferHandle & msg);
    CHIP_ERROR HandleSigmaR2(const PacketHeader & header, const System::PacketBufferHandle & msg);
    CHIP_ERROR SendSigmaR3();
    CHIP_ERROR HandleSigmaR3(const PacketHeader & header, const System::PacketBufferHandle & msg);

    CHIP_ERROR SendSigmaR1Resume();
    CHIP_ERROR HandleSigmaR1Resume_and_SendSigmaR2Resume(const PacketHeader & header, const System::PacketBufferHandle & msg);

    CHIP_ERROR FindValidTrustedRoot(const uint8_t ** msgIterator, uint32_t nTrustedRoots);
    CHIP_ERROR ConstructSaltSigmaR2(const System::PacketBufferHandle & rand, const P256PublicKey & pubkey, const uint8_t * ipk,
                                    size_t ipkLen, System::PacketBufferHandle & salt);
    CHIP_ERROR Validate_and_RetrieveResponderID(const uint8_t ** msgIterator, P256PublicKey & responderID,
                                                const uint8_t ** responderOpCert, uint16_t & responderOpCertLen);
    CHIP_ERROR ConstructSaltSigmaR3(const uint8_t * ipk, size_t ipkLen, System::PacketBufferHandle & salt);
    CHIP_ERROR ConstructSignedCredentials(const uint8_t ** msgIterator, const uint8_t * responderOpCert,
                                          uint16_t responderOpCertLen, System::PacketBufferHandle & signedCredentials,
                                          P256ECDSASignature & signature, size_t sigLen);
    CHIP_ERROR ComputeIPK(const uint16_t sessionID, uint8_t * ipk, size_t ipkLen);

    void SendErrorMsg(SigmaErrorType errorCode);
    void HandleErrorMsg(const PacketHeader & header, const System::PacketBufferHandle & msg);

    // TODO: Remove this and replace with system method to retrieve current time
    CHIP_ERROR SetEffectiveTime(void);

    CHIP_ERROR AttachHeaderAndSend(Protocols::SecureChannel::MsgType msgType, System::PacketBufferHandle msgBuf);

    void Clear();

    SessionEstablishmentDelegate * mDelegate = nullptr;

    Protocols::SecureChannel::MsgType mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;

    Hash_SHA256_stream mCommissioningHash;
    P256PublicKey mRemotePubKey;
    P256Keypair mEphemeralKey;
    P256ECDHDerivedSecret mSharedSecret;
    OperationalCredentialSet * mOpCredSet;
    CertificateKeyId mTrustedRootId;
    ValidationContext mValidContext;

    uint8_t mMessageDigest[kSHA256_Hash_Length];
    uint8_t mIPK[kIPKSize];
    uint8_t mRemoteIPK[kIPKSize];

    struct SigmaErrorMsg
    {
        SigmaErrorType error;
    };

protected:
    NodeId mLocalNodeId = kUndefinedNodeId;

    bool mPairingComplete = false;

    Transport::PeerConnectionState mConnectionState;
};

typedef struct CASESessionSerialized
{
    // Extra uint64_t to account for padding bytes (NULL termination, and some decoding overheads)
    uint8_t inner[BASE64_ENCODED_LEN(sizeof(CASESessionSerializable) + sizeof(uint64_t))];
} CASESessionSerialized;

} // namespace chip
