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
 *    @file
 *      This file implements the CHIP Secure Session object.
 *
 */

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <transport/CryptoContext.h>
#include <transport/raw/MessageHeader.h>

#include <lib/support/BytesToHex.h>

#include <string.h>

namespace chip {

namespace {

constexpr size_t kMaxAADLen = 128;

/* Session Establish Key Info */
constexpr uint8_t SEKeysInfo[] = { 0x53, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79, 0x73 };

/* Session Resumption Key Info */
constexpr uint8_t RSEKeysInfo[] = { 0x53, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x52, 0x65, 0x73, 0x75,
                                    0x6d, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79, 0x73 };

} // namespace

using namespace Crypto;

CryptoContext::CryptoContext() : mKeyAvailable(false) {}

CryptoContext::~CryptoContext()
{
    if (mKeystore)
    {
        mKeystore->DestroyKey(mEncryptionKey);
        mKeystore->DestroyKey(mDecryptionKey);
    }

    mKeystore   = nullptr;
    mKeyContext = nullptr;
}

CHIP_ERROR CryptoContext::InitFromSecret(SessionKeystore & keystore, const ByteSpan & secret, const ByteSpan & salt,
                                         SessionInfoType infoType, SessionRole role)
{
    VerifyOrReturnError(mKeyAvailable == false, CHIP_ERROR_INCORRECT_STATE);

    ByteSpan info = (infoType == SessionInfoType::kSessionResumption) ? ByteSpan(RSEKeysInfo) : ByteSpan(SEKeysInfo);

    // If the secure session is created by session initiator, use the I2R key to encrypt
    // messages being transmitted. Otherwise, use the R2I key.
    auto & i2rKey = (role == SessionRole::kInitiator) ? mEncryptionKey : mDecryptionKey;
    auto & r2iKey = (role == SessionRole::kInitiator) ? mDecryptionKey : mEncryptionKey;

#if CHIP_CONFIG_SECURITY_TEST_MODE
    ReturnErrorOnFailure(InitTestMode(keystore, i2rKey, r2iKey));
#else
    ReturnErrorOnFailure(keystore.DeriveSessionKeys(secret, salt, info, i2rKey, r2iKey, mAttestationChallenge));
#endif

    mKeyAvailable = true;
    mSessionRole  = role;
    mKeystore     = &keystore;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CryptoContext::InitFromSecret(Crypto::SessionKeystore & keystore, const Crypto::HkdfKeyHandle & hkdfKey,
                                         const ByteSpan & salt, SessionInfoType infoType, SessionRole role)
{
    VerifyOrReturnError(mKeyAvailable == false, CHIP_ERROR_INCORRECT_STATE);

    ByteSpan info = (infoType == SessionInfoType::kSessionResumption) ? ByteSpan(RSEKeysInfo) : ByteSpan(SEKeysInfo);

    // If the secure session is created by session initiator, use the I2R key to encrypt
    // messages being transmitted. Otherwise, use the R2I key.
    auto & i2rKey = (role == SessionRole::kInitiator) ? mEncryptionKey : mDecryptionKey;
    auto & r2iKey = (role == SessionRole::kInitiator) ? mDecryptionKey : mEncryptionKey;

#if CHIP_CONFIG_SECURITY_TEST_MODE
    ReturnErrorOnFailure(InitTestMode(keystore, i2rKey, r2iKey));
#else
    ReturnErrorOnFailure(keystore.DeriveSessionKeys(hkdfKey, salt, info, i2rKey, r2iKey, mAttestationChallenge));
#endif

    mKeyAvailable = true;
    mSessionRole  = role;
    mKeystore     = &keystore;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CryptoContext::InitFromKeyPair(SessionKeystore & keystore, const Crypto::P256Keypair & local_keypair,
                                          const Crypto::P256PublicKey & remote_public_key, const ByteSpan & salt,
                                          SessionInfoType infoType, SessionRole role)
{

    VerifyOrReturnError(mKeyAvailable == false, CHIP_ERROR_INCORRECT_STATE);

    P256ECDHDerivedSecret secret;
    ReturnErrorOnFailure(local_keypair.ECDH_derive_secret(remote_public_key, secret));

    return InitFromSecret(keystore, secret.Span(), salt, infoType, role);
}

#if CHIP_CONFIG_SECURITY_TEST_MODE
CHIP_ERROR CryptoContext::InitTestMode(Crypto::SessionKeystore & keystore, Crypto::Aes128KeyHandle & i2rKey,
                                       Crypto::Aes128KeyHandle & r2iKey)
{
    // If enabled, override the generated session key with a known key pair
    // to allow man-in-the-middle session key recovery for testing purposes.

    constexpr uint8_t kTestSharedSecret[CHIP_CONFIG_TEST_SHARED_SECRET_LENGTH] = CHIP_CONFIG_TEST_SHARED_SECRET_VALUE;

#warning                                                                                                                           \
    "Warning: CHIP_CONFIG_SECURITY_TEST_MODE=1 bypassing key negotiation... All sessions will use known, fixed test key, and NodeID=0 in NONCE. Node can only communicate with other nodes built with this flag set. Requires build flag 'treat_warnings_as_errors=false'."
    ChipLogError(
        SecureChannel,
        "Warning: CHIP_CONFIG_SECURITY_TEST_MODE=1 bypassing key negotiation... All sessions will use known, fixed test key, "
        "and NodeID=0 in NONCE. "
        "Node can only communicate with other nodes built with this flag set.");

    return keystore.DeriveSessionKeys(ByteSpan(kTestSharedSecret), ByteSpan{} /* salt */, ByteSpan(SEKeysInfo), i2rKey, r2iKey,
                                      mAttestationChallenge);
}
#endif // CHIP_CONFIG_SECURITY_TEST_MODE

CHIP_ERROR CryptoContext::BuildNonce(NonceView nonce, uint8_t securityFlags, uint32_t messageCounter, NodeId nodeId)
{
    Encoding::LittleEndian::BufferWriter bbuf(nonce.data(), nonce.size());

    bbuf.Put8(securityFlags);
    bbuf.Put32(messageCounter);
#if CHIP_CONFIG_SECURITY_TEST_MODE
    bbuf.Put64(0); // Simplifies decryption of CASE sessions when in TEST_MODE.
#else
    bbuf.Put64(nodeId);
#endif

    return bbuf.Fit() ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR CryptoContext::BuildPrivacyNonce(NonceView nonce, uint16_t sessionId, const MessageAuthenticationCode & mac)
{
    const uint8_t * micFragment = &mac.GetTag()[kPrivacyNonceMicFragmentOffset];
    Encoding::BigEndian::BufferWriter bbuf(nonce.data(), nonce.size());

    bbuf.Put16(sessionId);
    bbuf.Put(micFragment, kPrivacyNonceMicFragmentLength);
    return bbuf.Fit() ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR CryptoContext::GetAdditionalAuthData(const PacketHeader & header, uint8_t * aad, uint16_t & len)
{
    VerifyOrReturnError(len >= header.EncodeSizeBytes(), CHIP_ERROR_INVALID_ARGUMENT);

    // Use unencrypted part of header as AAD. This will help
    // integrity protect the whole message
    uint16_t actualEncodedHeaderSize;

    ReturnErrorOnFailure(header.Encode(aad, len, &actualEncodedHeaderSize));
    VerifyOrReturnError(len >= actualEncodedHeaderSize, CHIP_ERROR_INVALID_ARGUMENT);

    len = actualEncodedHeaderSize;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CryptoContext::Encrypt(const uint8_t * input, size_t input_length, uint8_t * output, ConstNonceView nonce,
                                  PacketHeader & header, MessageAuthenticationCode & mac) const
{

    const size_t taglen = header.MICTagLength();

    VerifyOrDie(taglen <= kMaxTagLen);

    VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t AAD[kMaxAADLen];
    uint16_t aadLen = sizeof(AAD);
    uint8_t tag[kMaxTagLen];

    ReturnErrorOnFailure(GetAdditionalAuthData(header, AAD, aadLen));

    if (mKeyContext)
    {
        ByteSpan plaintext(input, input_length);
        MutableByteSpan ciphertext(output, input_length);
        MutableByteSpan mic(tag, taglen);

        ReturnErrorOnFailure(mKeyContext->MessageEncrypt(plaintext, ByteSpan(AAD, aadLen), nonce, mic, ciphertext));
    }
    else
    {
        VerifyOrReturnError(mKeyAvailable, CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
        ReturnErrorOnFailure(
            AES_CCM_encrypt(input, input_length, AAD, aadLen, mEncryptionKey, nonce.data(), nonce.size(), output, tag, taglen));
    }

    mac.SetTag(&header, tag, taglen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CryptoContext::Decrypt(const uint8_t * input, size_t input_length, uint8_t * output, ConstNonceView nonce,
                                  const PacketHeader & header, const MessageAuthenticationCode & mac) const
{
    const size_t taglen = header.MICTagLength();
    const uint8_t * tag = mac.GetTag();
    uint8_t AAD[kMaxAADLen];
    uint16_t aadLen = sizeof(AAD);

    VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(GetAdditionalAuthData(header, AAD, aadLen));

    if (nullptr != mKeyContext)
    {
        ByteSpan ciphertext(input, input_length);
        MutableByteSpan plaintext(output, input_length);
        ByteSpan mic(tag, taglen);

        CHIP_ERROR err = mKeyContext->MessageDecrypt(ciphertext, ByteSpan(AAD, aadLen), nonce, mic, plaintext);
        ReturnErrorOnFailure(err);
    }
    else
    {
        VerifyOrReturnError(mKeyAvailable, CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
        ReturnErrorOnFailure(
            AES_CCM_decrypt(input, input_length, AAD, aadLen, tag, taglen, mDecryptionKey, nonce.data(), nonce.size(), output));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CryptoContext::PrivacyEncrypt(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                                         MessageAuthenticationCode & mac) const
{
    VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Confirm group key is available. Privacy obfuscation is not supported on unicast session keys.
    VerifyOrReturnError(mKeyContext != nullptr, CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);

    ByteSpan plaintext(input, input_length);
    MutableByteSpan privacytext(output, input_length);
    CryptoContext::NonceStorage privacyNonce;
    CryptoContext::BuildPrivacyNonce(privacyNonce, header.GetSessionId(), mac);

    return mKeyContext->PrivacyEncrypt(plaintext, privacyNonce, privacytext);
}

CHIP_ERROR CryptoContext::PrivacyDecrypt(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                                         const MessageAuthenticationCode & mac) const
{
    VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Confirm group key is available. Privacy obfuscation is not supported on session keys.
    VerifyOrReturnError(mKeyContext != nullptr, CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);

    const ByteSpan privacytext(input, input_length);
    MutableByteSpan plaintext(output, input_length);
    CryptoContext::NonceStorage privacyNonce;
    CryptoContext::BuildPrivacyNonce(privacyNonce, header.GetSessionId(), mac);

    return mKeyContext->PrivacyDecrypt(privacytext, privacyNonce, plaintext);
}

} // namespace chip
