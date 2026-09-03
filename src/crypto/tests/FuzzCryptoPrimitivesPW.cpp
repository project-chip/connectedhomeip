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
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/RawKeySessionKeystore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include "AES_CCM_128_test_vectors.h"
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
    return Arbitrary<Bytes>().WithMaxSize(kMaxNonceLen).WithSeeds(
        CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->nonce, v->nonce_len); }, kMaxNonceLen));
}

auto AnyAad()
{
    return Arbitrary<Bytes>().WithMaxSize(kMaxAadLen).WithSeeds(
        CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->aad, v->aad_len); }, kMaxAadLen));
}

auto AnyPlaintext()
{
    return Arbitrary<Bytes>().WithMaxSize(kMaxTextLen).WithSeeds(
        CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->pt, v->pt_len); }, kMaxTextLen));
}

auto AnyCiphertext()
{
    return Arbitrary<Bytes>().WithMaxSize(kMaxTextLen).WithSeeds(
        CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->ct, v->ct_len); }, kMaxTextLen));
}

auto AnyTag()
{
    return Arbitrary<Bytes>().WithMaxSize(kAES_CCM128_Tag_Length).WithSeeds(
        CcmSeeds([](const ccm_128_test_vector * v) { return ToBytes(v->tag, v->tag_len); }, kAES_CCM128_Tag_Length));
}

// AES-CCM permits these MIC/tag lengths.
auto AnyTagLen()
{
    return ElementOf<size_t>({ 4, 6, 8, 10, 12, 14, 16 });
}

bool MakeKey(const KeyArray & raw, RawKeySessionKeystore & keystore, Aes128KeyHandle & out)
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

    RawKeySessionKeystore keystore;
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
    (void) AES_CCM_decrypt(ciphertext.data(), ciphertext.size(), aad.data(), aad.size(), tag.data(), tagLen, handle, nonce.data(),
                           nonce.size(), plaintext.data());
    keystore.DestroyKey(handle);
}

FUZZ_TEST(CryptoPrimitives, AesCcmDecryptNoCrash)
    .WithDomains(AnyKey(), AnyNonce(), AnyAad(), AnyCiphertext(), AnyTag(), AnyTagLen());

// Round-trip plus authenticity: an honest decrypt must recover the plaintext,
// and any single-bit change to the tag or the ciphertext must be rejected.
void AesCcmRoundtrip(const KeyArray & key, const Bytes & nonce, const Bytes & aad, const Bytes & plaintext, size_t tagLen)
{
    EnsureInitialized();

    RawKeySessionKeystore keystore;
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
        ASSERT_NE(AES_CCM_decrypt(tamperedCiphertext.data(), tamperedCiphertext.size(), aad.data(), aad.size(), tag.data(),
                                  tagLen, handle, nonce.data(), nonce.size(), sink.data()),
                  CHIP_NO_ERROR);
    }

    keystore.DestroyKey(handle);
}

FUZZ_TEST(CryptoPrimitives, AesCcmRoundtrip).WithDomains(AnyKey(), AnyNonce(), AnyAad(), AnyPlaintext(), AnyTagLen());

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
    return Arbitrary<Bytes>().WithMaxSize(kMaxHkdfInputLen).WithSeeds(HkdfSeeds(
        [](const hkdf_sha256_vector & v) { return ToBytes(v.initial_key_material, v.initial_key_material_length); },
        kMaxHkdfInputLen));
}

auto AnyHkdfSalt()
{
    return Arbitrary<Bytes>().WithMaxSize(kMaxHkdfInputLen).WithSeeds(
        HkdfSeeds([](const hkdf_sha256_vector & v) { return ToBytes(v.salt, v.salt_length); }, kMaxHkdfInputLen));
}

auto AnyHkdfInfo()
{
    return Arbitrary<Bytes>().WithMaxSize(kMaxHkdfInputLen).WithSeeds(
        HkdfSeeds([](const hkdf_sha256_vector & v) { return ToBytes(v.info, v.info_length); }, kMaxHkdfInputLen));
}

void HkdfSha256NoCrash(const Bytes & secret, const Bytes & salt, const Bytes & info, size_t outLen)
{
    EnsureInitialized();

    Bytes out(outLen);
    HKDF_sha hkdf;
    (void) hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(), out.data(), outLen);
}

FUZZ_TEST(CryptoPrimitives, HkdfSha256NoCrash)
    .WithDomains(AnyHkdfSecret(), AnyHkdfSalt(), AnyHkdfInfo(), InRange<size_t>(1, 4096));

// HKDF is a pure function: the same inputs must always produce the same output.
void HkdfSha256Deterministic(const Bytes & secret, const Bytes & salt, const Bytes & info, size_t outLen)
{
    EnsureInitialized();

    HKDF_sha hkdf;
    Bytes first(outLen);
    Bytes second(outLen);
    CHIP_ERROR firstError = hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(),
                                             first.data(), outLen);
    CHIP_ERROR secondError = hkdf.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info.data(), info.size(),
                                              second.data(), outLen);
    ASSERT_EQ(firstError, secondError);
    if (firstError == CHIP_NO_ERROR)
    {
        ASSERT_EQ(first, second);
    }
}

FUZZ_TEST(CryptoPrimitives, HkdfSha256Deterministic)
    .WithDomains(AnyHkdfSecret(), AnyHkdfSalt(), AnyHkdfInfo(), InRange<size_t>(1, 1024));

} // namespace
