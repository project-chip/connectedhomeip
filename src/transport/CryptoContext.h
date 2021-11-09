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

#include <array>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/Serializer.h>
#include <lib/support/Span.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

class DLL_EXPORT CryptoContext
{
public:
    CryptoContext();
    ~CryptoContext();
    CryptoContext(CryptoContext &&)      = default;
    CryptoContext(const CryptoContext &) = default;
    CryptoContext & operator=(const CryptoContext &) = default;
    CryptoContext & operator=(CryptoContext &&) = default;

    bool operator==(const CryptoContext & that)
    {
        if (mKeyAvailable)
        {
            return that.mKeyAvailable && mSessionRole == that.mSessionRole && mKeys == that.mKeys;
        }
        else
        {
            return !that.mKeyAvailable;
        }
    }

    /**
     *    Whether the current node initiated the session, or it is responded to a session request.
     */
    enum class SessionRole
    {
        kInitiator, /**< We initiated the session. */
        kResponder, /**< We responded to the session request. */
    };

    enum class SessionInfoType
    {
        kSessionEstablishment, /**< A new secure session is established. */
        kSessionResumption,    /**< An old session is being resumed. */
    };

    /**
     * @brief
     *   Derive a shared key. The derived key will be used for encrypting/decrypting
     *   data exchanged on the secure channel.
     *
     * @param local_keypair      A reference to local ECP keypair
     * @param remote_public_key  A reference to peer's public key
     * @param salt               A reference to the initial salt used for deriving the keys
     * @param infoType           The info buffer to use for deriving session keys
     * @param role               Role of the new session (initiator or responder)
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR InitFromKeyPair(const Crypto::P256Keypair & local_keypair, const Crypto::P256PublicKey & remote_public_key,
                               const ByteSpan & salt, SessionInfoType infoType, SessionRole role);

    /**
     * @brief
     *   Derive a shared key. The derived key will be used for encrypting/decrypting
     *   data exchanged on the secure channel.
     *
     * @param secret             A reference to the shared secret
     * @param salt               A reference to the initial salt used for deriving the keys
     * @param infoType           The info buffer to use for deriving session keys
     * @param role               Role of the new session (initiator or responder)
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR InitFromSecret(const ByteSpan & secret, const ByteSpan & salt, SessionInfoType infoType, SessionRole role);

    /**
     * @brief
     *   Encrypt the input data using keys established in the secure channel
     *
     * @param input Unencrypted input data
     * @param input_length Length of the input data
     * @param output Output buffer for encrypted data
     * @param header message header structure. Encryption type will be set on the header.
     * @param mac - output the resulting mac
     *
     * @return CHIP_ERROR The result of encryption
     */
    CHIP_ERROR Encrypt(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                       MessageAuthenticationCode & mac) const;

    /**
     * @brief
     *   Decrypt the input data using keys established in the secure channel
     *
     * @param input Encrypted input data
     * @param input_length Length of the input data
     * @param output Output buffer for decrypted data
     * @param header message header structure
     * @return CHIP_ERROR The result of decryption
     * @param mac Input mac
     */
    CHIP_ERROR Decrypt(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                       const MessageAuthenticationCode & mac) const;

    ByteSpan GetAttestationChallenge() const
    {
        return ByteSpan(mKeys[kAttestationChallengeKey].data(), mKeys[kAttestationChallengeKey].size());
    }

private:
    using CryptoKey = std::array<uint8_t, Crypto::kAES_CCM128_Key_Length>;

    enum KeyUsage
    {
        kI2RKey                  = 0,
        kR2IKey                  = 1,
        kAttestationChallengeKey = 2,
        kNumCryptoKeys           = 3
    };

    SessionRole mSessionRole;

    bool mKeyAvailable;
    std::array<CryptoKey, KeyUsage::kNumCryptoKeys> mKeys;

    static CHIP_ERROR GetIV(const PacketHeader & header, uint8_t * iv, size_t len);

    // Use unencrypted header as additional authenticated data (AAD) during encryption and decryption.
    // The encryption operations includes AAD when message authentication tag is generated. This tag
    // is used at the time of decryption to integrity check the received data.
    static CHIP_ERROR GetAdditionalAuthData(const PacketHeader & header, uint8_t * aad, uint16_t & len);

    // Private constructor which is used to load from a save
    CryptoContext(SessionRole role, std::array<CryptoKey, KeyUsage::kNumCryptoKeys> keys) :
        mSessionRole(role), mKeyAvailable(true), mKeys(keys)
    {}

public:
    class Serializable
    {
    public:
        SessionRole mSessionRole;
        std::array<CryptoKey, KeyUsage::kNumCryptoKeys> mKeys;

        using Serializer = typename chip::Serializer<
            Serializable, FieldSerializer<Serializable, SessionRole, &Serializable::mSessionRole>,
            FieldSerializer<Serializable, std::array<CryptoKey, KeyUsage::kNumCryptoKeys>, &Serializable::mKeys>>;
    };

    static_assert(std::is_pod<Serializable>::value, "CryptoContext::Serializable is not a POD");

    Serializable Save() const
    {
        Serializable result;
        result.mSessionRole = mSessionRole;
        result.mKeys        = mKeys;
        return result;
    }

    static CryptoContext Load(const Serializable & from) { return CryptoContext(from.mSessionRole, from.mKeys); }
};

// Specialization for serialize CryptoContext::Serializable
template <>
class TypeSerializer<CryptoContext::Serializable>
{
private:
    using Serializer = CryptoContext::Serializable::Serializer;

public:
    static constexpr const size_t Space = Serializer::Space;

    static CryptoContext::Serializable LoadValue(FixedSpan<const char, Space> bytes)
    {
        CryptoContext::Serializable res;
        Serializer::LoadObject(res, bytes);
        return res;
    }

    static void SaveValue(FixedSpan<char, Space> bytes, const CryptoContext::Serializable & data)
    {
        Serializer::SaveObject(data, bytes);
    }
};

} // namespace chip
