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
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/support/Base64.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/SessionEstablishmentDelegate.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <system/SystemPacketBuffer.h>
#include <transport/PairingSession.h>
#include <transport/SecureSession.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

extern const char * kSpake2pI2RSessionInfo;
extern const char * kSpake2pR2ISessionInfo;
extern const char * kSpake2pKeyExchangeSalt;

constexpr uint16_t kPBKDFParamRandomNumberSize = 32;
constexpr uint32_t kSpake2p_Iteration_Count    = 100;

// Specifications section 3.9. Password-Based Key Derivation Function
constexpr uint32_t kPBKDFMinimumIterations = 1000;
constexpr uint32_t kPBKDFMaximumIterations = 100000;
constexpr uint32_t kPBKDFMinimumSaltLen    = 16;
constexpr uint32_t kPBKDFMaximumSaltLen    = 32;

using namespace Crypto;

constexpr size_t kSpake2p_WS_Length = kP256_FE_Length + 8;

struct PASESessionSerialized;

struct PASESessionSerializable
{
    uint16_t mKeLen;
    uint8_t mKe[kMAX_Hash_Length];
    uint8_t mPairingComplete;
    uint16_t mLocalKeyId;
    uint16_t mPeerKeyId;
};

struct PASEVerifier
{
    uint8_t mW0[kSpake2p_WS_Length];
    uint8_t mL[kSpake2p_WS_Length];
};

class DLL_EXPORT PASESession : public Messaging::ExchangeDelegate, public PairingSession
{
public:
    PASESession();
    PASESession(PASESession &&)      = default;
    PASESession(const PASESession &) = delete;
    PASESession & operator=(const PASESession &) = default;
    PASESession & operator=(PASESession &&) = default;

    virtual ~PASESession();

    // TODO: The SetPeerNodeId method should not be exposed; we should not need
    // to associate a node ID with a PASE session.
    using PairingSession::SetPeerNodeId;

    /**
     * @brief
     *   Initialize using setup PIN code and wait for pairing requests.
     *
     * @param mySetUpPINCode  Setup PIN code of the local device
     * @param pbkdf2IterCount Iteration count for PBKDF2 function
     * @param salt            Salt to be used for SPAKE2P operation
     * @param myKeyId         Key ID to be assigned to the secure session on the peer node
     * @param delegate        Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR WaitForPairing(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const ByteSpan & salt, uint16_t myKeyId,
                              SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Initialize using PASE verifier and wait for pairing requests.
     *
     * @param verifier        PASE verifier to be used for SPAKE2P pairing
     * @param pbkdf2IterCount Iteration count for PBKDF2 function
     * @param salt            Salt to be used for SPAKE2P operation
     * @param passcodeID      Passcode ID assigned by the administrator to this PASE verifier
     * @param myKeyId         Key ID to be assigned to the secure session on the peer node
     * @param delegate        Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR WaitForPairing(const PASEVerifier & verifier, uint32_t pbkdf2IterCount, const ByteSpan & salt, uint16_t passcodeID,
                              uint16_t myKeyId, SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Create a pairing request using peer's setup PIN code.
     *
     * @param peerAddress      Address of peer to pair
     * @param peerSetUpPINCode Setup PIN code of the peer device
     * @param myKeyId          Key ID to be assigned to the secure session on the peer node
     * @param exchangeCtxt     The exchange context to send and receive messages with the peer
     *                         Note: It's expected that the caller of this API hands over the
     *                         ownership of the exchangeCtxt to PASESession object. PASESession
     *                         will close the exchange on (successful/failed) handshake completion.
     * @param delegate         Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR Pair(const Transport::PeerAddress peerAddress, uint32_t peerSetUpPINCode, uint16_t myKeyId,
                    Messaging::ExchangeContext * exchangeCtxt, SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Generate a new PASE verifier.
     *
     * @param verifier        The generated PASE verifier
     * @param pbkdf2IterCount Iteration count for PBKDF2 function
     * @param salt            Salt to be used for SPAKE2P operation
     * @param useRandomPIN    Generate a random setup PIN, if true. Else, use the provided PIN
     * @param setupPIN        Provided setup PIN (if useRandomPIN is false), or the generated PIN
     *
     * @return CHIP_ERROR      The result of PASE verifier generation
     */
    static CHIP_ERROR GeneratePASEVerifier(PASEVerifier & verifier, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                           bool useRandomPIN, uint32_t & setupPIN);

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error
     *   if called before pairing is established.
     *
     * @param session     Referene to the secure session that will be
     *                    initialized once pairing is complete
     * @param role        Role of the new session (initiator or responder)
     * @return CHIP_ERROR The result of session derivation
     */
    CHIP_ERROR DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role) override;

    const char * GetI2RSessionInfo() const override { return kSpake2pI2RSessionInfo; }

    const char * GetR2ISessionInfo() const override { return kSpake2pR2ISessionInfo; }

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

    // TODO: remove Clear, we should create a new instance instead reset the old instance.
    /** @brief This function zeroes out and resets the memory used by the object.
     **/
    void Clear();

    SessionEstablishmentExchangeDispatch & MessageDispatch() { return mMessageDispatch; }

    //// ExchangeDelegate Implementation ////
    /**
     * @brief
     *   This function is the called by exchange context or exchange manager when it receives
     *   a CHIP message corresponding to the context, or registered unsolicited message handler.
     *
     *   Note: If the function is called by unsolicited message handler, the ownership of the
     *         provide exchange context is handed over to PASE Session object. The PASE Session
     *         object ensures that the exchange will be closed on completion of the handshake.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *  @param[in]    payloadHeader A reference to the PayloadHeader object.
     *  @param[in]    payload       A handle to the PacketBuffer object holding the message payload.
     */
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload) override;

    /**
     * @brief
     *   This function is the protocol callback to invoke when the timeout for the receipt
     *   of a response message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    Messaging::ExchangeMessageDispatch * GetMessageDispatch(Messaging::ReliableMessageMgr * rmMgr,
                                                            SecureSessionMgr * sessionMgr) override
    {
        return &mMessageDispatch;
    }

private:
    enum Spake2pErrorType : uint8_t
    {
        kInvalidKeyConfirmation = 0x00,
        kUnexpected             = 0xff,
    };

    CHIP_ERROR Init(uint16_t myKeyId, uint32_t setupCode, SessionEstablishmentDelegate * delegate);

    CHIP_ERROR ValidateReceivedMessage(Messaging::ExchangeContext * exchange, const PacketHeader & packetHeader,
                                       const PayloadHeader & payloadHeader, System::PacketBufferHandle && msg);

    static CHIP_ERROR ComputePASEVerifier(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                          PASEVerifier & verifier);

    CHIP_ERROR SetupSpake2p(uint32_t pbkdf2IterCount, const ByteSpan & salt);

    CHIP_ERROR SendPBKDFParamRequest();
    CHIP_ERROR HandlePBKDFParamRequest(const System::PacketBufferHandle & msg);

    CHIP_ERROR SendPBKDFParamResponse();
    CHIP_ERROR HandlePBKDFParamResponse(const System::PacketBufferHandle & msg);

    CHIP_ERROR SendMsg1();

    CHIP_ERROR HandleMsg1_and_SendMsg2(const System::PacketBufferHandle & msg);
    CHIP_ERROR HandleMsg2_and_SendMsg3(const System::PacketBufferHandle & msg);
    CHIP_ERROR HandleMsg3(const System::PacketBufferHandle & msg);

    void SendErrorMsg(Spake2pErrorType errorCode);
    CHIP_ERROR HandleErrorMsg(const System::PacketBufferHandle & msg);

    void CloseExchange();

    SessionEstablishmentDelegate * mDelegate = nullptr;

    Protocols::SecureChannel::MsgType mNextExpectedMsg = Protocols::SecureChannel::MsgType::PASE_Spake2pError;

#ifdef ENABLE_HSM_SPAKE
    Spake2pHSM_P256_SHA256_HKDF_HMAC mSpake2p;
#else
    Spake2p_P256_SHA256_HKDF_HMAC mSpake2p;
#endif
    uint8_t mPoint[kMAX_Point_Length];

    /* w0s and w1s */
    PASEVerifier mPASEVerifier;

    uint16_t mPasscodeID = 0;

    uint32_t mSetupPINCode;

    bool mComputeVerifier = true;

    Hash_SHA256_stream mCommissioningHash;
    uint32_t mIterationCount = 0;
    uint16_t mSaltLength     = 0;
    uint8_t * mSalt          = nullptr;

    Messaging::ExchangeContext * mExchangeCtxt = nullptr;

    SessionEstablishmentExchangeDispatch mMessageDispatch;

    struct Spake2pErrorMsg
    {
        Spake2pErrorType error;
    };

protected:
    uint8_t mKe[kMAX_Hash_Length];

    size_t mKeLen = sizeof(mKe);

    bool mPairingComplete = false;
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
class SecurePairingUsingTestSecret : public PairingSession
{
public:
    SecurePairingUsingTestSecret()
    {
        SetLocalKeyId(0);
        SetPeerKeyId(0);
    }

    SecurePairingUsingTestSecret(uint16_t peerKeyId, uint16_t localKeyId)
    {
        SetLocalKeyId(localKeyId);
        SetPeerKeyId(peerKeyId);
    }

    CHIP_ERROR DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role) override
    {
        size_t secretLen = strlen(kTestSecret);
        return session.InitFromSecret(ByteSpan(reinterpret_cast<const uint8_t *>(kTestSecret), secretLen), ByteSpan(nullptr, 0),
                                      SecureSession::SessionInfoType::kSessionEstablishment, role);
    }

    CHIP_ERROR ToSerializable(PASESessionSerializable & serializable)
    {
        size_t secretLen = strlen(kTestSecret);

        memset(&serializable, 0, sizeof(serializable));
        serializable.mKeLen           = static_cast<uint16_t>(secretLen);
        serializable.mPairingComplete = 1;
        serializable.mLocalKeyId      = GetLocalKeyId();
        serializable.mPeerKeyId       = GetPeerKeyId();

        memcpy(serializable.mKe, kTestSecret, secretLen);
        return CHIP_NO_ERROR;
    }

    const char * GetI2RSessionInfo() const override { return "i2r"; }

    const char * GetR2ISessionInfo() const override { return "r2i"; }

private:
    const char * kTestSecret = "Test secret for key derivation";
};

typedef struct PASESessionSerialized
{
    // Extra uint64_t to account for padding bytes (NULL termination, and some decoding overheads)
    uint8_t inner[BASE64_ENCODED_LEN(sizeof(PASESessionSerializable) + sizeof(uint64_t))];
} PASESessionSerialized;

} // namespace chip
