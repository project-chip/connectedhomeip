/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      FuzzTest harness for the AEAD and KDF primitives that sit underneath
 *      every encrypted Matter session: CASE and PASE derive keys with
 *      HKDF-SHA256, and every secured message is AES-CCM.
 *
 *      Each parameter has its own typed domain, seeded from this directory's
 *      AES-CCM and HKDF test vectors, so the checks are functional invariants
 *      rather than only "does not crash":
 *
 *        - AES-CCM round-trip: decrypt(encrypt(pt)) recovers pt.
 *        - AES-CCM authenticity: a tampered tag or ciphertext must not decrypt.
 *        - HKDF determinism: identical inputs produce identical output.
 *
 *      Seeds are attached to the individual domains rather than to the
 *      FUZZ_TEST registration, because only domain-level seeds are injected
 *      when the suite runs under the libFuzzer compatibility engine.
 */

#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include "AES_CCM_128_test_vectors.h"
#include "DerSigConversion_test_vectors.h"
#include "HKDF_SHA256_test_vectors.h"

namespace {

using namespace chip;
using namespace chip::Crypto;
using namespace fuzztest;

using KeyArray = std::array<uint8_t, kAES_CCM128_Key_Length>;
using Bytes    = std::vector<uint8_t>;

// The test vectors carry nonces of 8 and 12 bytes; Matter itself mandates 13.
// Allow a little headroom either side so nonce-length handling is exercised.
constexpr size_t kMaxNonceLen = kAES_CCM128_Nonce_Length + 3;
constexpr size_t kMaxAadLen   = 1024;
constexpr size_t kMaxTextLen  = 2048;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
#if CHIP_CRYPTO_PSA
        // The PSA backend rejects every key operation until the driver is
        // initialized, which would otherwise turn each property below into a
        // vacuous pass.
        VerifyOrDie(psa_crypto_init() == PSA_SUCCESS);
#endif
        return true;
    }();
    (void) sInitialized;
}

Bytes ToBytes(const uint8_t * data, size_t len)
{
    return (data == nullptr) ? Bytes{} : Bytes(data, data + len);
}

// Pull one field out of every AES-CCM test vector to use as domain seeds.
template <typename Selector>
std::vector<Bytes> CcmSeeds(Selector select, size_t maxLen)
{
    std::vector<Bytes> seeds;
    for (const auto * vector : ccm_128_test_vectors)
    {
        Bytes value = select(vector);
        if (value.size() <= maxLen)
        {
            seeds.push_back(std::move(value));
        }
    }
    return seeds;
}

std::vector<KeyArray> CcmKeySeeds()
{
    std::vector<KeyArray> seeds;
    for (const auto * vector : ccm_128_test_vectors)
    {
        if (vector->key_len != kAES_CCM128_Key_Length)
        {
            continue;
        }
        KeyArray key{};
        memcpy(key.data(), vector->key, key.size());
        seeds.push_back(key);
    }
    return seeds;
}

auto AnyKey()
{
    return Arbitrary<KeyArray>().WithSeeds(CcmKeySeeds());
}

auto AnyNonce()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kMaxNonceLen)
        .WithSeeds(CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->nonce, v->nonce_len); }, kMaxNonceLen));
}

auto AnyAad()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kMaxAadLen)
        .WithSeeds(CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->aad, v->aad_len); }, kMaxAadLen));
}

auto AnyPlaintext()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kMaxTextLen)
        .WithSeeds(CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->pt, v->pt_len); }, kMaxTextLen));
}

auto AnyCiphertext()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kMaxTextLen)
        .WithSeeds(CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->ct, v->ct_len); }, kMaxTextLen));
}

auto AnyTag()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kAES_CCM128_Tag_Length)
        .WithSeeds(CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->tag, v->tag_len); }, kAES_CCM128_Tag_Length));
}

// AES-CCM permits these MIC/tag lengths.
auto AnyTagLen()
{
    return ElementOf<size_t>({ 4, 6, 8, 10, 12, 14, 16 });
}

bool MakeKey(const KeyArray & raw, DefaultSessionKeystore & keystore, Aes128KeyHandle & out)
{
    Symmetric128BitsKeyByteArray material;
    memcpy(material, raw.data(), raw.size());
    return keystore.CreateKey(material, out) == CHIP_NO_ERROR;
}

// Drive the decryptor with fully fuzzer-controlled inputs. The authenticity
// check is expected to reject them; the contract under test is that it does so
// without reading or writing out of bounds.
void AesCcmDecryptNoCrash(const KeyArray & key, const Bytes & nonce, const Bytes & aad, const Bytes & ciphertext,
                          const Bytes & tagBytes, size_t tagLen)
{
    EnsureInitialized();

    DefaultSessionKeystore keystore;
    Aes128KeyHandle handle;
    if (!MakeKey(key, keystore, handle))
    {
        return;
    }

    Bytes tag(tagLen, 0);
    for (size_t i = 0; i < tagLen && i < tagBytes.size(); ++i)
    {
        tag[i] = tagBytes[i];
    }

    Bytes plaintext(ciphertext.size());
    RETURN_SAFELY_IGNORED AES_CCM_decrypt(ciphertext.data(), ciphertext.size(), aad.data(), aad.size(), tag.data(), tagLen, handle,
                                          nonce.data(), nonce.size(), plaintext.data());
    keystore.DestroyKey(handle);
}

FUZZ_TEST(CryptoPrimitives, AesCcmDecryptNoCrash)
    .WithDomains(AnyKey(), AnyNonce(), AnyAad(), AnyCiphertext(), AnyTag(), AnyTagLen());

// Round-trip plus authenticity: an honest decrypt must recover the plaintext,
// and any single-bit change to the tag or the ciphertext must be rejected.
void AesCcmRoundtrip(const KeyArray & key, const Bytes & nonce, const Bytes & aad, const Bytes & plaintext, size_t tagLen)
{
    EnsureInitialized();

    DefaultSessionKeystore keystore;
    Aes128KeyHandle handle;
    if (!MakeKey(key, keystore, handle))
    {
        return;
    }

    Bytes ciphertext(plaintext.size());
    Bytes tag(tagLen);
    CHIP_ERROR encryptError = AES_CCM_encrypt(plaintext.data(), plaintext.size(), aad.data(), aad.size(), handle, nonce.data(),
                                              nonce.size(), ciphertext.data(), tag.data(), tagLen);
    if (encryptError != CHIP_NO_ERROR)
    {
        keystore.DestroyKey(handle);
        return;
    }

    Bytes recovered(plaintext.size());
    ASSERT_EQ(AES_CCM_decrypt(ciphertext.data(), ciphertext.size(), aad.data(), aad.size(), tag.data(), tagLen, handle,
                              nonce.data(), nonce.size(), recovered.data()),
              CHIP_NO_ERROR);
    ASSERT_EQ(recovered, plaintext);

    Bytes sink(plaintext.size());

    Bytes tamperedTag = tag;
    tamperedTag[0] ^= 0x01;
    ASSERT_NE(AES_CCM_decrypt(ciphertext.data(), ciphertext.size(), aad.data(), aad.size(), tamperedTag.data(), tagLen, handle,
                              nonce.data(), nonce.size(), sink.data()),
              CHIP_NO_ERROR);

    if (!ciphertext.empty())
    {
        Bytes tamperedCiphertext = ciphertext;
        tamperedCiphertext[0] ^= 0x01;
        ASSERT_NE(AES_CCM_decrypt(tamperedCiphertext.data(), tamperedCiphertext.size(), aad.data(), aad.size(), tag.data(), tagLen,
                                  handle, nonce.data(), nonce.size(), sink.data()),
                  CHIP_NO_ERROR);
    }

    keystore.DestroyKey(handle);
}

FUZZ_TEST(CryptoPrimitives, AesCcmRoundtrip).WithDomains(AnyKey(), AnyNonce(), AnyAad(), AnyPlaintext(), AnyTagLen());

// AES-CTR carries no authentication tag: it is the keystream XOR used to
// obfuscate the privacy header of a group message, and on receive it runs over
// the datagram before the AES-CCM tag is checked.
void AesCtrCryptNoCrash(const KeyArray & key, const Bytes & nonce, const Bytes & input)
{
    EnsureInitialized();

    DefaultSessionKeystore keystore;
    Aes128KeyHandle handle;
    if (!MakeKey(key, keystore, handle))
    {
        return;
    }

    Bytes output(input.size());
    RETURN_SAFELY_IGNORED AES_CTR_crypt(input.data(), input.size(), handle, nonce.data(), nonce.size(), output.data());
    keystore.DestroyKey(handle);
}

FUZZ_TEST(CryptoPrimitives, AesCtrCryptNoCrash).WithDomains(AnyKey(), AnyNonce(), AnyCiphertext());

// CTR mode is its own inverse, so applying it twice must return the input.
void AesCtrRoundtrip(const KeyArray & key, const Bytes & nonce, const Bytes & plaintext)
{
    EnsureInitialized();

    DefaultSessionKeystore keystore;
    Aes128KeyHandle handle;
    if (!MakeKey(key, keystore, handle))
    {
        return;
    }

    Bytes ciphertext(plaintext.size());
    if (AES_CTR_crypt(plaintext.data(), plaintext.size(), handle, nonce.data(), nonce.size(), ciphertext.data()) == CHIP_NO_ERROR)
    {
        Bytes recovered(plaintext.size());
        ASSERT_EQ(AES_CTR_crypt(ciphertext.data(), ciphertext.size(), handle, nonce.data(), nonce.size(), recovered.data()),
                  CHIP_NO_ERROR);
        ASSERT_EQ(recovered, plaintext);
    }
    keystore.DestroyKey(handle);
}

FUZZ_TEST(CryptoPrimitives, AesCtrRoundtrip).WithDomains(AnyKey(), AnyNonce(), AnyPlaintext());

// Epoch keys and compressed fabric ids from the group key derivation vectors in
// TestChipCryptoPAL.cpp.
const uint8_t kEpochKey1[] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf };
const uint8_t kEpochKey2[] = { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf };
const uint8_t kEpochKey3[] = { 0x23, 0x5b, 0xf7, 0xe6, 0x28, 0x23, 0xd3, 0x58, 0xdc, 0xa4, 0xba, 0x50, 0xb1, 0x53, 0x5f, 0x4b };
const uint8_t kCompressedFabricId1[] = { 0x29, 0x06, 0xc9, 0x08, 0xd1, 0x15, 0xd3, 0x62 };
const uint8_t kCompressedFabricId2[] = { 0x87, 0xe1, 0xb0, 0x04, 0xe2, 0x35, 0xa1, 0x30 };

auto AnyEpochKey()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES * 2)
        .WithSeeds({ Bytes(std::begin(kEpochKey1), std::end(kEpochKey1)), Bytes(std::begin(kEpochKey2), std::end(kEpochKey2)),
                     Bytes(std::begin(kEpochKey3), std::end(kEpochKey3)) });
}

auto AnyCompressedFabricId()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kCompressedFabricIdentifierSize * 2)
        .WithSeeds({ Bytes(std::begin(kCompressedFabricId1), std::end(kCompressedFabricId1)),
                     Bytes(std::begin(kCompressedFabricId2), std::end(kCompressedFabricId2)) });
}

// The group key schedule: an epoch key plus the compressed fabric id yield the
// encryption key, the privacy key and the session id used for group messaging.
void GroupKeyDerivation(const Bytes & epochKey, const Bytes & compressedFabricId)
{
    EnsureInitialized();

    const ByteSpan epochKeySpan(epochKey.data(), epochKey.size());
    const ByteSpan fabricIdSpan(compressedFabricId.data(), compressedFabricId.size());

    uint8_t operationalKeyBuf[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    MutableByteSpan operationalKey(operationalKeyBuf);
    if (DeriveGroupOperationalKey(epochKeySpan, fabricIdSpan, operationalKey) == CHIP_NO_ERROR)
    {
        // The session id is derived from the operational key, so it is only
        // meaningful once that derivation has succeeded.
        uint16_t sessionId = 0;
        RETURN_SAFELY_IGNORED DeriveGroupSessionId(ByteSpan(operationalKey.data(), operationalKey.size()), sessionId);
    }

    // Fed the raw fuzzer bytes rather than the derived operational key that
    // production uses, so that the length-rejection path is exercised too.
    uint8_t privacyKeyBuf[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    MutableByteSpan privacyKey(privacyKeyBuf);
    RETURN_SAFELY_IGNORED DeriveGroupPrivacyKey(epochKeySpan, privacyKey);

    GroupOperationalCredentials credentials;
    RETURN_SAFELY_IGNORED DeriveGroupOperationalCredentials(epochKeySpan, fabricIdSpan, credentials);
}

FUZZ_TEST(CryptoPrimitives, GroupKeyDerivation).WithDomains(AnyEpochKey(), AnyCompressedFabricId());

// The combined credentials call must agree with running the same schedule by
// hand. Note the chaining: the privacy key is derived from the *operational*
// key, not from the epoch key -- DeriveGroupPrivacyKey names its parameter
// encryption_key, and both keys are 16 bytes, so feeding it the epoch key is
// accepted and silently yields a different key.
void GroupKeyDerivationAgrees(const Bytes & epochKey, const Bytes & compressedFabricId)
{
    EnsureInitialized();

    const ByteSpan epochKeySpan(epochKey.data(), epochKey.size());
    const ByteSpan fabricIdSpan(compressedFabricId.data(), compressedFabricId.size());

    GroupOperationalCredentials credentials;
    if (DeriveGroupOperationalCredentials(epochKeySpan, fabricIdSpan, credentials) != CHIP_NO_ERROR)
    {
        return;
    }

    uint8_t operationalKeyBuf[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    MutableByteSpan operationalKey(operationalKeyBuf);
    ASSERT_EQ(DeriveGroupOperationalKey(epochKeySpan, fabricIdSpan, operationalKey), CHIP_NO_ERROR);
    ASSERT_EQ(memcmp(credentials.encryption_key, operationalKey.data(), operationalKey.size()), 0);

    uint8_t privacyKeyBuf[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    MutableByteSpan privacyKey(privacyKeyBuf);
    ASSERT_EQ(DeriveGroupPrivacyKey(ByteSpan(operationalKey.data(), operationalKey.size()), privacyKey), CHIP_NO_ERROR);
    ASSERT_EQ(memcmp(credentials.privacy_key, privacyKey.data(), privacyKey.size()), 0);

    uint16_t sessionId = 0;
    ASSERT_EQ(DeriveGroupSessionId(ByteSpan(operationalKey.data(), operationalKey.size()), sessionId), CHIP_NO_ERROR);
    ASSERT_EQ(sessionId, credentials.hash);
}

FUZZ_TEST(CryptoPrimitives, GroupKeyDerivationAgrees).WithDomains(AnyEpochKey(), AnyCompressedFabricId());

// ECDSA signatures cross this boundary in both directions: raw fixed-width r||s
// on the Matter wire, X9.62 DER inside certificates.
std::vector<Bytes> DerSigSeeds(bool wantDer)
{
    std::vector<Bytes> seeds;
    for (const auto & vector : kDerSigConvTestVectors)
    {
        seeds.push_back(wantDer ? ToBytes(vector.der_version, vector.der_version_length)
                                : ToBytes(vector.raw_version, vector.raw_version_length));
    }
    return seeds;
}

// The vectors use 32- and 64-byte field elements, so the buffers here are sized
// past P-256's kMax_ECDSA_Signature_Length rather than from it. Fuzzing the
// width a little beyond the largest vector exercises the length validation.
constexpr size_t kMaxFeLen     = 80;
constexpr size_t kMaxRawSigLen = 2 * kMaxFeLen;
constexpr size_t kMaxDerSigLen = kMaxRawSigLen + kMax_ECDSA_X9Dot62_Asn1_Overhead;

auto AnyFeLength()
{
    return InRange<size_t>(0, kMaxFeLen);
}

void EcdsaSignatureRawToDer(size_t feLengthBytes, const Bytes & rawSig)
{
    EnsureInitialized();

    uint8_t derBuf[kMaxDerSigLen];
    MutableByteSpan derSig(derBuf);
    RETURN_SAFELY_IGNORED EcdsaRawSignatureToAsn1(feLengthBytes, ByteSpan(rawSig.data(), rawSig.size()), derSig);
}

FUZZ_TEST(CryptoPrimitives, EcdsaSignatureRawToDer)
    .WithDomains(AnyFeLength(), Arbitrary<Bytes>().WithMaxSize(kMaxRawSigLen).WithSeeds(DerSigSeeds(false)));

void EcdsaSignatureDerToRaw(size_t feLengthBytes, const Bytes & derSig)
{
    EnsureInitialized();

    uint8_t rawBuf[kMaxRawSigLen];
    MutableByteSpan rawSig(rawBuf);
    RETURN_SAFELY_IGNORED EcdsaAsn1SignatureToRaw(feLengthBytes, ByteSpan(derSig.data(), derSig.size()), rawSig);
}

FUZZ_TEST(CryptoPrimitives, EcdsaSignatureDerToRaw)
    .WithDomains(AnyFeLength(), Arbitrary<Bytes>().WithMaxSize(kMaxDerSigLen).WithSeeds(DerSigSeeds(true)));

// The two converters are inverses for a well-formed raw signature, so a
// successful round-trip must reproduce the input exactly.
void EcdsaSignatureConversionRoundtrip(const Bytes & rawSig)
{
    EnsureInitialized();

    // Both directions take the field-element width, which is half of r||s.
    if (rawSig.empty() || (rawSig.size() % 2) != 0)
    {
        return;
    }
    const size_t feLengthBytes = rawSig.size() / 2;

    uint8_t derBuf[kMaxDerSigLen];
    MutableByteSpan derSig(derBuf);
    if (EcdsaRawSignatureToAsn1(feLengthBytes, ByteSpan(rawSig.data(), rawSig.size()), derSig) != CHIP_NO_ERROR)
    {
        return;
    }

    uint8_t rawBuf[kMaxRawSigLen];
    MutableByteSpan recovered(rawBuf);
    ASSERT_EQ(EcdsaAsn1SignatureToRaw(feLengthBytes, ByteSpan(derSig.data(), derSig.size()), recovered), CHIP_NO_ERROR);
    ASSERT_EQ(recovered.size(), rawSig.size());
    ASSERT_EQ(memcmp(recovered.data(), rawSig.data(), rawSig.size()), 0);
}

FUZZ_TEST(CryptoPrimitives, EcdsaSignatureConversionRoundtrip)
    .WithDomains(Arbitrary<Bytes>().WithMaxSize(kMaxRawSigLen).WithSeeds(DerSigSeeds(false)));

template <typename Selector>
std::vector<Bytes> HkdfSeeds(Selector select, size_t maxLen)
{
    std::vector<Bytes> seeds;
    for (const auto & vector : hkdf_sha256_test_vectors)
    {
        Bytes value = select(vector);
        if (value.size() <= maxLen)
        {
            seeds.push_back(std::move(value));
        }
    }
    return seeds;
}

constexpr size_t kMaxHkdfInputLen = 512;

auto AnyHkdfSecret()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kMaxHkdfInputLen)
        .WithSeeds(
            HkdfSeeds([](const hkdf_sha256_vector & v) { return ToBytes(v.initial_key_material, v.initial_key_material_length); },
                      kMaxHkdfInputLen));
}

auto AnyHkdfSalt()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kMaxHkdfInputLen)
        .WithSeeds(HkdfSeeds([](const hkdf_sha256_vector & v) { return ToBytes(v.salt, v.salt_length); }, kMaxHkdfInputLen));
}

auto AnyHkdfInfo()
{
    return Arbitrary<Bytes>()
        .WithMaxSize(kMaxHkdfInputLen)
        .WithSeeds(HkdfSeeds([](const hkdf_sha256_vector & v) { return ToBytes(v.info, v.info_length); }, kMaxHkdfInputLen));
}

void HkdfSha256NoCrash(const Bytes & secret, const Bytes & salt, const Bytes & info, size_t outLen)
{
    EnsureInitialized();

    Bytes out(outLen);
    HKDF_sha hkdf;
    RETURN_SAFELY_IGNORED hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(),
                                           out.data(), outLen);
}

FUZZ_TEST(CryptoPrimitives, HkdfSha256NoCrash).WithDomains(AnyHkdfSecret(), AnyHkdfSalt(), AnyHkdfInfo(), InRange<size_t>(1, 4096));

// HKDF is a pure function: the same inputs must always produce the same output.
void HkdfSha256Deterministic(const Bytes & secret, const Bytes & salt, const Bytes & info, size_t outLen)
{
    EnsureInitialized();

    HKDF_sha hkdf;
    Bytes first(outLen);
    Bytes second(outLen);
    CHIP_ERROR firstError =
        hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(), first.data(), outLen);
    CHIP_ERROR secondError =
        hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(), second.data(), outLen);
    ASSERT_EQ(firstError, secondError);
    if (firstError == CHIP_NO_ERROR)
    {
        ASSERT_EQ(first, second);
    }
}

FUZZ_TEST(CryptoPrimitives, HkdfSha256Deterministic)
    .WithDomains(AnyHkdfSecret(), AnyHkdfSalt(), AnyHkdfInfo(), InRange<size_t>(1, 1024));

} // namespace
