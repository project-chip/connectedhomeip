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
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/support/Base64.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/SessionEstablishmentDelegate.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <system/SystemPacketBuffer.h>
#include <transport/FabricTable.h>
#include <transport/PairingSession.h>
#include <transport/SecureSession.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

// TODO: move this constant over to src/crypto/CHIPCryptoPAL.h - name it CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES
constexpr uint16_t kAEADKeySize = 16;

constexpr uint16_t kSigmaParamRandomNumberSize = 32;
constexpr uint16_t kTrustedRootIdSize          = Credentials::kKeyIdentifierLength;
constexpr uint16_t kMaxTrustedRootIds          = 5;

constexpr uint16_t kIPKSize = 16;

#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
#define CASE_EPHEMERAL_KEY 0xCA5EECD0
#endif

struct CASESessionSerialized;

struct CASESessionSerializable
{
    uint16_t mSharedSecretLen;
    uint8_t mSharedSecret[Crypto::kMax_ECDH_Secret_Length];
    uint16_t mMessageDigestLen;
    uint8_t mMessageDigest[Crypto::kSHA256_Hash_Length];
    uint16_t mIPKLen;
    uint8_t mIPK[kIPKSize];
    uint8_t mPairingComplete;
    NodeId mPeerNodeId;
    uint16_t mLocalKeyId;
    uint16_t mPeerKeyId;
};

class DLL_EXPORT CASESession : public Messaging::ExchangeDelegate, public PairingSession
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
     *   Initialize using configured fabrics and wait for session establishment requests.
     *
     * @param myKeyId                       Key ID to be assigned to the secure session on the peer node
     * @param fabrics                       Table of fabrics that are currently configured on the device
     * @param delegate                      Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR ListenForSessionEstablishment(uint16_t myKeyId, Transport::FabricTable * fabrics,
                                             SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Create and send session establishment request using device's operational credentials.
     *
     * @param peerAddress                   Address of peer with which to establish a session.
     * @param fabric                        The fabric that should be used for connecting with the peer
     * @param peerNodeId                    Node id of the peer node
     * @param myKeyId                       Key ID to be assigned to the secure session on the peer node
     * @param exchangeCtxt                  The exchange context to send and receive messages with the peer
     * @param delegate                      Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR EstablishSession(const Transport::PeerAddress peerAddress, Transport::FabricInfo * fabric, NodeId peerNodeId,
                                uint16_t myKeyId, Messaging::ExchangeContext * exchangeCtxt,
                                SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Derive a secure session from the established session. The API will return error
     *   if called before session is established.
     *
     * @param session     Reference to the secure session that will be
     *                    initialized once session establishment is complete
     * @param role        Role of the new session (initiator or responder)
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role) override;

    const char * GetI2RSessionInfo() const override { return "Sigma I2R Key"; }

    const char * GetR2ISessionInfo() const override { return "Sigma R2I Key"; }

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

    SessionEstablishmentExchangeDispatch & MessageDispatch() { return mMessageDispatch; }

    //// ExchangeDelegate Implementation ////
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;
    Messaging::ExchangeMessageDispatch * GetMessageDispatch(Messaging::ReliableMessageMgr * rmMgr,
                                                            SecureSessionMgr * sessionMgr) override
    {
        return &mMessageDispatch;
    }

    FabricIndex GetFabricIndex() const
    {
        return mFabricInfo != nullptr ? mFabricInfo->GetFabricIndex() : Transport::kUndefinedFabricIndex;
    }

    // TODO: remove Clear, we should create a new instance instead reset the old instance.
    /** @brief This function zeroes out and resets the memory used by the object.
     **/
    void Clear();

private:
    enum SigmaErrorType : uint8_t
    {
        kInvalidSignature     = 0x04,
        kInvalidResumptionTag = 0x05,
        kUnsupportedVersion   = 0x06,
        kUnexpected           = 0xff,
    };

    CHIP_ERROR Init(uint16_t myKeyId, SessionEstablishmentDelegate * delegate);

    CHIP_ERROR SendSigmaR1();
    CHIP_ERROR HandleSigmaR1_and_SendSigmaR2(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleSigmaR1(System::PacketBufferHandle && msg);
    CHIP_ERROR SendSigmaR2();
    CHIP_ERROR HandleSigmaR2_and_SendSigmaR3(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleSigmaR2(System::PacketBufferHandle && msg);
    CHIP_ERROR SendSigmaR3();
    CHIP_ERROR HandleSigmaR3(System::PacketBufferHandle && msg);

    CHIP_ERROR SendSigmaR1Resume();
    CHIP_ERROR HandleSigmaR1Resume_and_SendSigmaR2Resume(const PacketHeader & header, const System::PacketBufferHandle & msg);

    CHIP_ERROR ConstructSaltSigmaR2(const ByteSpan & rand, const Crypto::P256PublicKey & pubkey, const ByteSpan & ipk,
                                    MutableByteSpan & salt);
    CHIP_ERROR Validate_and_RetrieveResponderID(const ByteSpan & responderOpCert, Crypto::P256PublicKey & responderID);
    CHIP_ERROR ConstructSaltSigmaR3(const ByteSpan & ipk, MutableByteSpan & salt);
    CHIP_ERROR ConstructTBS2Data(const ByteSpan & responderOpCert, uint8_t * tbsData, size_t & tbsDataLen);
    CHIP_ERROR ConstructTBS3Data(const ByteSpan & responderOpCert, uint8_t * tbsData, size_t & tbsDataLen);
    CHIP_ERROR RetrieveIPK(FabricId fabricId, MutableByteSpan & ipk);

    static constexpr size_t EstimateTLVStructOverhead(size_t dataLen, size_t nFields)
    {
        return dataLen + (sizeof(uint64_t) * nFields);
    }

    void SendErrorMsg(SigmaErrorType errorCode);

    // This function always returns an error. The error value corresponds to the error in the received message.
    // The returned error value helps top level message receiver/dispatcher to close the exchange context
    // in a more seamless manner.
    CHIP_ERROR HandleErrorMsg(const System::PacketBufferHandle & msg);

    void CloseExchange();

    // TODO: Remove this and replace with system method to retrieve current time
    CHIP_ERROR SetEffectiveTime(void);

    CHIP_ERROR ValidateReceivedMessage(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                       const PayloadHeader & payloadHeader, System::PacketBufferHandle & msg);

    SessionEstablishmentDelegate * mDelegate = nullptr;

    Protocols::SecureChannel::MsgType mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;

    Crypto::Hash_SHA256_stream mCommissioningHash;
    Crypto::P256PublicKey mRemotePubKey;
#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
    Crypto::P256KeypairHSM mEphemeralKey;
#else
    Crypto::P256Keypair mEphemeralKey;
#endif
    Crypto::P256ECDHDerivedSecret mSharedSecret;
    Credentials::CertificateKeyId mTrustedRootId;
    Credentials::ValidationContext mValidContext;

    uint8_t mMessageDigest[Crypto::kSHA256_Hash_Length];
    uint8_t mIPK[kIPKSize];

    Messaging::ExchangeContext * mExchangeCtxt = nullptr;
    SessionEstablishmentExchangeDispatch mMessageDispatch;

    Transport::FabricTable * mFabricsTable = nullptr;
    Transport::FabricInfo * mFabricInfo    = nullptr;

    struct SigmaErrorMsg
    {
        SigmaErrorType error;
    };

protected:
    bool mPairingComplete = false;

    virtual ByteSpan * GetIPKList() const
    {
        // TODO: Remove this list. Replace it with an actual method to retrieve an IPK list (e.g. from a Crypto Store API)
        static uint8_t sIPKList[][kIPKSize] = {
            { 0 }, /* Corresponds to the FabricID for the Commissioning Example. All zeros. */
        };
        static ByteSpan ipkListSpan[] = { ByteSpan(sIPKList[0]) };
        return ipkListSpan;
    }
    virtual size_t GetIPKListEntries() const { return 1; }
};

typedef struct CASESessionSerialized
{
    // Extra uint64_t to account for padding bytes (NULL termination, and some decoding overheads)
    uint8_t inner[BASE64_ENCODED_LEN(sizeof(CASESessionSerializable) + sizeof(uint64_t))];
} CASESessionSerialized;

} // namespace chip
