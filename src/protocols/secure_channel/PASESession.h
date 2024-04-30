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
#if CHIP_CRYPTO_PSA_SPAKE2P
#include <crypto/PSASpake2p.h>
#endif
#include <lib/support/Base64.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/PairingSession.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <system/SystemPacketBuffer.h>
#include <transport/CryptoContext.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

extern const char kSpake2pI2RSessionInfo[];
extern const char kSpake2pR2ISessionInfo[];

inline constexpr uint16_t kPBKDFParamRandomNumberSize = 32;

class DLL_EXPORT PASESession : public Messaging::UnsolicitedMessageHandler,
                               public Messaging::ExchangeDelegate,
                               public PairingSession
{
public:
    ~PASESession() override;

    Transport::SecureSession::Type GetSecureSessionType() const override { return Transport::SecureSession::Type::kPASE; }
    ScopedNodeId GetPeer() const override
    {
        return ScopedNodeId(NodeIdFromPAKEKeyId(kDefaultCommissioningPasscodeId), kUndefinedFabricIndex);
    }

    ScopedNodeId GetLocalScopedNodeId() const override
    {
        // For PASE, source is always the undefined node ID
        return ScopedNodeId();
    }

    CATValues GetPeerCATs() const override { return CATValues(); };

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;

    /**
     * @brief
     *   Initialize using PASE verifier and wait for pairing requests.
     *
     * @param sessionManager      session manager from which to allocate a secure session object
     * @param verifier            PASE verifier to be used for SPAKE2P pairing
     * @param pbkdf2IterCount     Iteration count for PBKDF2 function
     * @param salt                Salt to be used for SPAKE2P operation
     * @param delegate            Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR WaitForPairing(SessionManager & sessionManager, const Crypto::Spake2pVerifier & verifier, uint32_t pbkdf2IterCount,
                              const ByteSpan & salt, Optional<ReliableMessageProtocolConfig> mrpLocalConfig,
                              SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Create a pairing request using peer's setup PIN code.
     *
     * @param sessionManager      session manager from which to allocate a secure session object
     * @param peerSetUpPINCode    Setup PIN code of the peer device
     * @param exchangeCtxt        The exchange context to send and receive messages with the peer
     *                            Note: It's expected that the caller of this API hands over the
     *                            ownership of the exchangeCtxt to PASESession object. PASESession
     *                            will close the exchange on (successful/failed) handshake completion.
     * @param delegate            Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR Pair(SessionManager & sessionManager, uint32_t peerSetUpPINCode,
                    Optional<ReliableMessageProtocolConfig> mrpLocalConfig, Messaging::ExchangeContext * exchangeCtxt,
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
    static CHIP_ERROR GeneratePASEVerifier(Crypto::Spake2pVerifier & verifier, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                           bool useRandomPIN, uint32_t & setupPIN);

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error if called before pairing is established.
     *
     * @param session     Reference to the secure session that will be initialized once pairing is complete
     * @return CHIP_ERROR The result of session derivation
     */
    CHIP_ERROR DeriveSecureSession(CryptoContext & session) override;

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

    //// SessionDelegate ////
    void OnSessionReleased() override;

private:
    enum Spake2pErrorType : uint8_t
    {
        kInvalidKeyConfirmation = 0x00,
        kUnexpected             = 0xff,
    };

    CHIP_ERROR Init(SessionManager & sessionManager, uint32_t setupCode, SessionEstablishmentDelegate * delegate);

    CHIP_ERROR ValidateReceivedMessage(Messaging::ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                       const System::PacketBufferHandle & msg);

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
    CHIP_ERROR OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode,
                                     Optional<uintptr_t> protocolData) override;

    void Finish();

    // mNextExpectedMsg is set when we are expecting a message.
    Optional<Protocols::SecureChannel::MsgType> mNextExpectedMsg;

#if CHIP_CRYPTO_PSA_SPAKE2P
    Crypto::PSASpake2p_P256_SHA256_HKDF_HMAC mSpake2p;
#else
    Crypto::Spake2p_P256_SHA256_HKDF_HMAC mSpake2p;
#endif

    Crypto::Spake2pVerifier mPASEVerifier;

    uint32_t mSetupPINCode;

    bool mHavePBKDFParameters = false;

    uint8_t mPBKDFLocalRandomData[kPBKDFParamRandomNumberSize];

    Crypto::Hash_SHA256_stream mCommissioningHash;
    uint32_t mIterationCount = 0;
    uint16_t mSaltLength     = 0;
    uint8_t * mSalt          = nullptr;

    struct Spake2pErrorMsg
    {
        Spake2pErrorType error;
    };

protected:
    uint8_t mKe[Crypto::kMAX_Hash_Length];

    size_t mKeLen = sizeof(mKe);

    bool mPairingComplete = false;
};

// The following constants are node IDs that test devices and test controllers use.
inline constexpr chip::NodeId kTestControllerNodeId = 112233;
inline constexpr chip::NodeId kTestDeviceNodeId     = 12344321;

} // namespace chip
