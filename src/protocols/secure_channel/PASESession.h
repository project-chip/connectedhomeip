/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
#include <transport/CryptoContext.h>
#include <transport/PairingSession.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

extern const char * kSpake2pI2RSessionInfo;
extern const char * kSpake2pR2ISessionInfo;

constexpr uint16_t kPBKDFParamRandomNumberSize = 32;

constexpr uint32_t kSetupPINCodeMaximumValue   = 99999998;
constexpr uint32_t kSetupPINCodeUndefinedValue = 0;

using namespace Crypto;

constexpr size_t kSpake2p_WS_Length = kP256_FE_Length + 8;

struct PASESessionSerialized;

struct PASESessionSerializable
{
    uint16_t mKeLen;
    uint8_t mKe[kMAX_Hash_Length];
    uint8_t mPairingComplete;
    uint16_t mLocalSessionId;
    uint16_t mPeerSessionId;
};

struct PASEVerifier
{
    uint8_t mW0[kP256_FE_Length];
    uint8_t mL[kP256_Point_Length];

    CHIP_ERROR Serialize(MutableByteSpan & outSerialized);
    CHIP_ERROR Deserialize(ByteSpan inSerialized);
};

typedef uint8_t PASEVerifierSerialized[Crypto::kSpake2pSerializedVerifierSize];

class DLL_EXPORT PASESession : public Messaging::ExchangeDelegate, public PairingSession
{
public:
    PASESession();
    PASESession(PASESession &&)      = default;
    PASESession(const PASESession &) = delete;

    virtual ~PASESession();

    // TODO: The SetPeerNodeId method should not be exposed; PASE sessions
    // should not need to be told their peer node ID
    using PairingSession::SetPeerNodeId;

    /**
     * @brief
     *   Initialize using PASE verifier and wait for pairing requests.
     *
     * @param verifier        PASE verifier to be used for SPAKE2P pairing
     * @param pbkdf2IterCount Iteration count for PBKDF2 function
     * @param salt            Salt to be used for SPAKE2P operation
     * @param passcodeID      Passcode ID assigned by the administrator to this PASE verifier
     * @param mySessionId     Session ID to be assigned to the secure session on the peer node
     * @param delegate        Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR WaitForPairing(const PASEVerifier & verifier, uint32_t pbkdf2IterCount, const ByteSpan & salt, PasscodeId passcodeID,
                              uint16_t mySessionId, Optional<ReliableMessageProtocolConfig> mrpConfig,
                              SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Create a pairing request using peer's setup PIN code.
     *
     * @param peerAddress      Address of peer to pair
     * @param peerSetUpPINCode Setup PIN code of the peer device
     * @param mySessionId      Session ID to be assigned to the secure session on the peer node
     * @param exchangeCtxt     The exchange context to send and receive messages with the peer
     *                         Note: It's expected that the caller of this API hands over the
     *                         ownership of the exchangeCtxt to PASESession object. PASESession
     *                         will close the exchange on (successful/failed) handshake completion.
     * @param delegate         Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR Pair(const Transport::PeerAddress peerAddress, uint32_t peerSetUpPINCode, uint16_t mySessionId,
                    Optional<ReliableMessageProtocolConfig> mrpConfig, Messaging::ExchangeContext * exchangeCtxt,
                    SessionEstablishmentDelegate * delegate);

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
     * @param session     Reference to the secure session that will be
     *                    initialized once pairing is complete
     * @param role        Role of the new session (initiator or responder)
     * @return CHIP_ERROR The result of session derivation
     */
    CHIP_ERROR DeriveSecureSession(CryptoContext & session, CryptoContext::SessionRole role) override;

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
     *  @param[in]    payloadHeader A reference to the PayloadHeader object.
     *  @param[in]    payload       A handle to the PacketBuffer object holding the message payload.
     */
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    /**
     * @brief
     *   This function is the protocol callback to invoke when the timeout for the receipt
     *   of a response message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    Messaging::ExchangeMessageDispatch & GetMessageDispatch() override { return SessionEstablishmentExchangeDispatch::Instance(); }

private:
    enum Spake2pErrorType : uint8_t
    {
        kInvalidKeyConfirmation = 0x00,
        kUnexpected             = 0xff,
    };

    CHIP_ERROR Init(uint16_t mySessionId, uint32_t setupCode, SessionEstablishmentDelegate * delegate);

    CHIP_ERROR ValidateReceivedMessage(Messaging::ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                       System::PacketBufferHandle && msg);

    static CHIP_ERROR ComputeWS(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const ByteSpan & salt, uint32_t wsSize,
                                uint8_t * ws);

    CHIP_ERROR SetupSpake2p();

    CHIP_ERROR SendPBKDFParamRequest();
    CHIP_ERROR HandlePBKDFParamRequest(System::PacketBufferHandle && msg);

    CHIP_ERROR SendPBKDFParamResponse(ByteSpan initiatorRandom, bool initiatorHasPBKDFParams);
    CHIP_ERROR HandlePBKDFParamResponse(System::PacketBufferHandle && msg);

    CHIP_ERROR SendMsg1();

    CHIP_ERROR HandleMsg1_and_SendMsg2(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleMsg2_and_SendMsg3(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleMsg3(System::PacketBufferHandle && msg);

    void OnSuccessStatusReport() override;
    CHIP_ERROR OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode) override;

    void CloseExchange();

    /**
     * Clear our reference to our exchange context pointer so that it can close
     * itself at some later time.
     */
    void DiscardExchange();

    SessionEstablishmentDelegate * mDelegate = nullptr;

    Protocols::SecureChannel::MsgType mNextExpectedMsg = Protocols::SecureChannel::MsgType::PASE_PakeError;

#ifdef ENABLE_HSM_SPAKE
    Spake2pHSM_P256_SHA256_HKDF_HMAC mSpake2p;
#else
    Spake2p_P256_SHA256_HKDF_HMAC mSpake2p;
#endif

    PASEVerifier mPASEVerifier;

    PasscodeId mPasscodeID = kDefaultCommissioningPasscodeId;

    uint32_t mSetupPINCode;

    bool mHavePBKDFParameters = false;

    uint8_t mPBKDFLocalRandomData[kPBKDFParamRandomNumberSize];

    Hash_SHA256_stream mCommissioningHash;
    uint32_t mIterationCount = 0;
    uint16_t mSaltLength     = 0;
    uint8_t * mSalt          = nullptr;

    Messaging::ExchangeContext * mExchangeCtxt = nullptr;

    Optional<ReliableMessageProtocolConfig> mLocalMRPConfig;

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
    SecurePairingUsingTestSecret() : PairingSession(Transport::SecureSession::Type::kPASE)
    {
        // Do not set to 0 to prevent unwanted unsecured session
        // since the session type is unknown.
        SetLocalSessionId(1);
        SetPeerSessionId(1);
    }

    SecurePairingUsingTestSecret(uint16_t peerSessionId, uint16_t localSessionId) :
        PairingSession(Transport::SecureSession::Type::kPASE)
    {
        SetLocalSessionId(localSessionId);
        SetPeerSessionId(peerSessionId);
    }

    CHIP_ERROR DeriveSecureSession(CryptoContext & session, CryptoContext::SessionRole role) override
    {
        size_t secretLen = strlen(kTestSecret);
        return session.InitFromSecret(ByteSpan(reinterpret_cast<const uint8_t *>(kTestSecret), secretLen), ByteSpan(nullptr, 0),
                                      CryptoContext::SessionInfoType::kSessionEstablishment, role);
    }

    CHIP_ERROR ToSerializable(PASESessionSerializable & serializable)
    {
        size_t secretLen = strlen(kTestSecret);

        memset(&serializable, 0, sizeof(serializable));
        serializable.mKeLen           = static_cast<uint16_t>(secretLen);
        serializable.mPairingComplete = 1;
        serializable.mLocalSessionId  = GetLocalSessionId();
        serializable.mPeerSessionId   = GetPeerSessionId();

        memcpy(serializable.mKe, kTestSecret, secretLen);
        return CHIP_NO_ERROR;
    }

private:
    const char * kTestSecret = CHIP_CONFIG_TEST_SHARED_SECRET_VALUE;
};

typedef struct PASESessionSerialized
{
    // Extra uint64_t to account for padding bytes (NULL termination, and some decoding overheads)
    uint8_t inner[BASE64_ENCODED_LEN(sizeof(PASESessionSerializable) + sizeof(uint64_t))];
} PASESessionSerialized;

} // namespace chip
