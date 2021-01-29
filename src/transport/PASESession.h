/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the CHIP SPAKE2P Session object that provides
 *      APIs for constructing spake2p messages and establishing encryption
 *      keys.
 *
 */

#pragma once

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

extern const char * kSpake2pI2RSessionInfo;
extern const char * kSpake2pR2ISessionInfo;

constexpr uint16_t kPBKDFParamRandomNumberSize = 32;

using namespace Crypto;

struct PASESessionSerialized;

struct PASESessionSerializable
{
    uint16_t mKeLen;
    uint8_t mKe[kMAX_Hash_Length];
    uint8_t mPairingComplete;
    uint64_t mLocalNodeId;
    uint64_t mPeerNodeId;
    uint16_t mLocalKeyId;
    uint16_t mPeerKeyId;
};

class DLL_EXPORT PASESession
{
public:
    PASESession();
    PASESession(PASESession &&)      = default;
    PASESession(const PASESession &) = delete;
    PASESession & operator=(const PASESession &) = default;
    PASESession & operator=(PASESession &&) = default;

    virtual ~PASESession();

    /**
     * @brief
     *   Initialize using setup PIN code and wait for pairing requests.
     *
     * @param mySetUpPINCode  Setup PIN code of the local device
     * @param pbkdf2IterCount Iteration count for PBKDF2 function
     * @param salt            Salt to be used for SPAKE2P opertation
     * @param saltLen         Length of salt
     * @param myNodeId        Optional node id of local node
     * @param myKeyId         Key ID to be assigned to the secure session on the peer node
     * @param delegate        Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR WaitForPairing(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const uint8_t * salt, size_t saltLen,
                              Optional<NodeId> myNodeId, uint16_t myKeyId, SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Create a pairing request using peer's setup PIN code.
     *
     * @param peerAddress      Address of peer to pair
     * @param peerSetUpPINCode Setup PIN code of the peer device
     * @param myNodeId         Optional node id of local node
     * @param peerNodeId       Node id assigned to the peer node by commissioner
     * @param myKeyId          Key ID to be assigned to the secure session on the peer node
     * @param delegate         Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR Pair(const Transport::PeerAddress peerAddress, uint32_t peerSetUpPINCode, Optional<NodeId> myNodeId,
                    NodeId peerNodeId, uint16_t myKeyId, SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error
     *   if called before pairing is established.
     *
     * @param info        Information string used for key derivation
     * @param info_len    Length of info string
     * @param session     Referene to the sescure session that will be
     *                    initialized once pairing is complete
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(const uint8_t * info, size_t info_len, SecureSession & session);

    /**
     * @brief
     *   Handler for peer's messages, exchanged during pairing handshake.
     *
     * @param packetHeader      Message header for the received message
     * @param peerAddress Source of the message
     * @param msg         Message sent by the peer
     * @return CHIP_ERROR The result of message processing
     */
    virtual CHIP_ERROR HandlePeerMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                         System::PacketBufferHandle msg);

    /**
     * @brief
     *  Return the associated secure session peer NodeId
     *
     * @return Optional<NodeId> The associated peer NodeId
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

    /** @brief Serialize the Pairing Session to a string.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Serialize(PASESessionSerialized & output);

    /** @brief Deserialize the Pairing Session from the string.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Deserialize(PASESessionSerialized & input);

    /** @brief Serialize the PASESession to the given serializable data structure for secure pairing
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ToSerializable(PASESessionSerializable & output);

    /** @brief Reconstruct secure pairing class from the serializable data structure.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR FromSerializable(const PASESessionSerializable & output);

private:
    enum Spake2pErrorType : uint8_t
    {
        kInvalidKeyConfirmation = 0x00,
        kUnexpected             = 0xff,
    };

    CHIP_ERROR Init(Optional<NodeId> myNodeId, uint16_t myKeyId, uint32_t setupCode, SessionEstablishmentDelegate * delegate);

    CHIP_ERROR SetupSpake2p(uint32_t pbkdf2IterCount, const uint8_t * salt, size_t saltLen);

    CHIP_ERROR SendPBKDFParamRequest();
    CHIP_ERROR HandlePBKDFParamRequest(const PacketHeader & header, const System::PacketBufferHandle & msg);

    CHIP_ERROR SendPBKDFParamResponse();
    CHIP_ERROR HandlePBKDFParamResponse(const PacketHeader & header, const System::PacketBufferHandle & msg);

    CHIP_ERROR SendMsg1();

    CHIP_ERROR HandleMsg1_and_SendMsg2(const PacketHeader & header, const System::PacketBufferHandle & msg);
    CHIP_ERROR HandleMsg2_and_SendMsg3(const PacketHeader & header, const System::PacketBufferHandle & msg);
    CHIP_ERROR HandleMsg3(const PacketHeader & header, const System::PacketBufferHandle & msg);

    void SendErrorMsg(Spake2pErrorType errorCode);
    void HandleErrorMsg(const PacketHeader & header, const System::PacketBufferHandle & msg);

    CHIP_ERROR AttachHeaderAndSend(Protocols::SecureChannel::MsgType msgType, System::PacketBufferHandle msgBuf);

    void Clear();

    static constexpr size_t kSpake2p_WS_Length = kP256_FE_Length + 8;

    SessionEstablishmentDelegate * mDelegate = nullptr;

    Protocols::SecureChannel::MsgType mNextExpectedMsg = Protocols::SecureChannel::MsgType::PASE_Spake2pError;

    Spake2p_P256_SHA256_HKDF_HMAC mSpake2p;

    uint8_t mPoint[kMAX_Point_Length];

    /* w0s and w1s */
    uint8_t mWS[2][kSpake2p_WS_Length];

    uint32_t mSetupPINCode;

    Hash_SHA256_stream mCommissioningHash;
    uint32_t mIterationCount = 0;
    uint16_t mSaltLength     = 0;
    uint8_t * mSalt          = nullptr;

    struct Spake2pErrorMsg
    {
        Spake2pErrorType error;
    };

protected:
    NodeId mLocalNodeId = kUndefinedNodeId;

    uint8_t mKe[kMAX_Hash_Length];

    size_t mKeLen = sizeof(mKe);

    bool mPairingComplete = false;

    Transport::PeerConnectionState mConnectionState;
};

/*
 * The following constants are node IDs that test devices and test
 * controllers use while using the SecurePairingUsingTestSecret to
 * establish secure channel
 */
constexpr chip::NodeId kTestControllerNodeId = 112233;
constexpr chip::NodeId kTestDeviceNodeId     = 12344321;

/*
 * The following class should only be used for test usecases.
 * The class is currently also used for devices that do no yet support
 * rendezvous. Once all the non-test usecases start supporting
 * rendezvous, this class will be moved to the test code.
 */
class SecurePairingUsingTestSecret : public PASESession
{
public:
    SecurePairingUsingTestSecret()
    {
        const char * secret = "Test secret for key derivation";
        size_t secretLen    = strlen(secret);
        mKeLen              = secretLen;
        memmove(mKe, secret, mKeLen);
        mConnectionState.SetPeerKeyID(0);
        mConnectionState.SetLocalKeyID(0);
        mPairingComplete = true;
    }

    SecurePairingUsingTestSecret(Optional<NodeId> peerNodeId, uint16_t peerKeyId, uint16_t localKeyId)
    {
        const char * secret = "Test secret for key derivation";
        size_t secretLen    = strlen(secret);
        mKeLen              = secretLen;
        memmove(mKe, secret, mKeLen);
        mConnectionState.SetPeerNodeId(peerNodeId.ValueOr(kUndefinedNodeId));
        mConnectionState.SetPeerKeyID(peerKeyId);
        mConnectionState.SetLocalKeyID(localKeyId);
        mPairingComplete = true;
    }

    ~SecurePairingUsingTestSecret() override {}

    CHIP_ERROR WaitForPairing(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const uint8_t * salt, size_t saltLen,
                              Optional<NodeId> myNodeId, uint16_t myKeyId, SessionEstablishmentDelegate * delegate)
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Pair(uint32_t peerSetUpPINCode, uint32_t pbkdf2IterCount, const uint8_t * salt, size_t saltLen,
                    Optional<NodeId> myNodeId, uint16_t myKeyId, SessionEstablishmentDelegate * delegate)
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandlePeerMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                 System::PacketBufferHandle msg) override
    {
        return CHIP_NO_ERROR;
    }
};

typedef struct PASESessionSerialized
{
    // Extra uint64_t to account for padding bytes (NULL termination, and some decoding overheads)
    uint8_t inner[BASE64_ENCODED_LEN(sizeof(PASESessionSerializable) + sizeof(uint64_t))];
} PASESessionSerialized;

} // namespace chip
