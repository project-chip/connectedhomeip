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
 *      This file defines the CHIP Secure Session object that provides
 *      APIs for encrypting/decryting data using cryptographic keys.
 *
 */

#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/Span.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

class DLL_EXPORT CryptoContext
{
public:
    static constexpr size_t kPrivacyNonceMicFragmentOffset = 5;
    static constexpr size_t kPrivacyNonceMicFragmentLength = 11;
    static constexpr size_t kAESCCMNonceLen                = 13;
    using NonceStorage                                     = std::array<uint8_t, kAESCCMNonceLen>;
    using NonceView                                        = FixedSpan<uint8_t, kAESCCMNonceLen>;
    using ConstNonceView                                   = FixedSpan<const uint8_t, kAESCCMNonceLen>;

    CryptoContext();
    ~CryptoContext();
    CryptoContext(CryptoContext &&)      = delete;
    CryptoContext(const CryptoContext &) = delete;
    explicit CryptoContext(Crypto::SymmetricKeyContext * context) : mKeyContext(context) {}
    CryptoContext & operator=(const CryptoContext &) = delete;
    CryptoContext & operator=(CryptoContext &&) = delete;

    /**
     *    Whether the current node initiated the session, or it is responded to a session request.
     */
    enum class SessionRole : uint8_t
    {
        kInitiator, /**< We initiated the session. */
        kResponder, /**< We responded to the session request. */
    };

    enum class SessionInfoType : uint8_t
    {
        kSessionEstablishment, /**< A new secure session is established. */
        kSessionResumption,    /**< An old session is being resumed. */
    };

    /**
     * @brief
     *   Derive a shared key. The derived key will be used for encrypting/decrypting
     *   data exchanged on the secure channel.
     *
     * @param keystore           Session keystore for management of symmetric encryption keys
     * @param local_keypair      A reference to local ECP keypair
     * @param remote_public_key  A reference to peer's public key
     * @param salt               A reference to the initial salt used for deriving the keys
     * @param infoType           The info buffer to use for deriving session keys
     * @param role               Role of the new session (initiator or responder)
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR InitFromKeyPair(Crypto::SessionKeystore & keystore, const Crypto::P256Keypair & local_keypair,
                               const Crypto::P256PublicKey & remote_public_key, const ByteSpan & salt, SessionInfoType infoType,
                               SessionRole role);

    /**
     * @brief
     *   Derive a shared key. The derived key will be used for encrypting/decrypting
     *   data exchanged on the secure channel.
     *
     * @param keystore           Session keystore for management of symmetric encryption keys
     * @param secret             A reference to the shared secret
     * @param salt               A reference to the initial salt used for deriving the keys
     * @param infoType           The info buffer to use for deriving session keys
     * @param role               Role of the new session (initiator or responder)
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR InitFromSecret(Crypto::SessionKeystore & keystore, const ByteSpan & secret, const ByteSpan & salt,
                              SessionInfoType infoType, SessionRole role);

    /** @brief Build a Nonce buffer using given parameters for encrypt or decrypt. */
    static CHIP_ERROR BuildNonce(NonceView nonce, uint8_t securityFlags, uint32_t messageCounter, NodeId nodeId);

    /** @brief Build a Nonce buffer using given parameters for encrypt or decrypt. */
    static CHIP_ERROR BuildPrivacyNonce(NonceView nonce, uint16_t sessionId, const MessageAuthenticationCode & mac);

    /**
     * @brief
     *   Encrypt the input data using keys established in the secure channel
     *
     * @param input Unencrypted input data
     * @param input_length Length of the input data
     * @param output Output buffer for encrypted data
     * @param nonce Nonce buffer for encrypt
     * @param header message header structure. Encryption type will be set on the header.
     * @param mac - output the resulting mac
     *
     * @return CHIP_ERROR The result of encryption
     */
    CHIP_ERROR Encrypt(const uint8_t * input, size_t input_length, uint8_t * output, ConstNonceView nonce, PacketHeader & header,
                       MessageAuthenticationCode & mac) const;

    /**
     * @brief
     *   Decrypt the input data using keys established in the secure channel
     *
     * @param input Encrypted input data
     * @param input_length Length of the input data
     * @param output Output buffer for decrypted data
     * @param nonce Nonce buffer for decrypt
     * @param header message header structure
     * @return CHIP_ERROR The result of decryption
     * @param mac Input mac
     */
    CHIP_ERROR Decrypt(const uint8_t * input, size_t input_length, uint8_t * output, ConstNonceView nonce,
                       const PacketHeader & header, const MessageAuthenticationCode & mac) const;

    CHIP_ERROR PrivacyEncrypt(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                              MessageAuthenticationCode & mac) const;

    CHIP_ERROR PrivacyDecrypt(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                              const MessageAuthenticationCode & mac) const;

    ByteSpan GetAttestationChallenge() const { return mAttestationChallenge.Span(); }

    /**
     * @brief
     *   Memory overhead of encrypting data. The overhead is independent of size of
     *   the data being encrypted. The extra space is used for storing the common header.
     *
     * @return number of bytes.
     */
    size_t EncryptionOverhead();

    bool IsInitiator() const { return mKeyAvailable && mSessionRole == SessionRole::kInitiator; }

    bool IsResponder() const { return mKeyAvailable && mSessionRole == SessionRole::kResponder; }

private:
    SessionRole mSessionRole;

    bool mKeyAvailable;
    Crypto::Aes128KeyHandle mEncryptionKey;
    Crypto::Aes128KeyHandle mDecryptionKey;
    Crypto::AttestationChallenge mAttestationChallenge;
    Crypto::SessionKeystore * mKeystore       = nullptr;
    Crypto::SymmetricKeyContext * mKeyContext = nullptr;

    // Use unencrypted header as additional authenticated data (AAD) during encryption and decryption.
    // The encryption operations includes AAD when message authentication tag is generated. This tag
    // is used at the time of decryption to integrity check the received data.
    static CHIP_ERROR GetAdditionalAuthData(const PacketHeader & header, uint8_t * aad, uint16_t & len);
};

} // namespace chip
