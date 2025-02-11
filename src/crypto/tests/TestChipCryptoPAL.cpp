/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#include "TestCryptoLayer.h"

#include "AES_CCM_128_test_vectors.h"
#include "DerSigConversion_test_vectors.h"
#include "ECDH_P256_test_vectors.h"
#include "HKDF_SHA256_test_vectors.h"
#include "HMAC_SHA256_test_vectors.h"
#include "Hash_SHA256_test_vectors.h"
#include "PBKDF2_SHA256_test_vectors.h"

#include "RawIntegerToDer_test_vectors.h"
#include "SPAKE2P_FE_MUL_test_vectors.h"
#include "SPAKE2P_FE_RW_test_vectors.h"
#include "SPAKE2P_HMAC_test_vectors.h"
#include "SPAKE2P_POINT_MUL_ADD_test_vectors.h"
#include "SPAKE2P_POINT_MUL_test_vectors.h"
#include "SPAKE2P_POINT_RW_test_vectors.h"
#include "SPAKE2P_POINT_VALID_test_vectors.h"
#include "SPAKE2P_RFC_test_vectors.h"

#include <pw_unit_test/framework.h>

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/support/BytesToHex.h>

#include <credentials/CHIPCert.h>
#include <credentials/attestation_verifier/TestPAAStore.h>
#include <credentials/tests/CHIPAttCert_test_vectors.h>
#include <credentials/tests/CHIPCert_test_vectors.h>

#include <lib/asn1/ASN1.h>
#include <lib/asn1/ASN1Macros.h>
#include <lib/core/TLV.h>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if CHIP_CRYPTO_MBEDTLS || CHIP_CRYPTO_PSA
#include <mbedtls/memory_buffer_alloc.h>
#endif

#if CHIP_CRYPTO_PSA
#include <psa/crypto.h>
extern "C" {
psa_status_t psa_initialize_key_slots(void);
void psa_wipe_all_key_slots(void);
}
#endif

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;
using namespace chip::TLV;

namespace {

using Test_P256Keypair                  = P256Keypair;
using TestSpake2p_P256_SHA256_HKDF_HMAC = Spake2p_P256_SHA256_HKDF_HMAC;
using TestPBKDF2_sha256                 = PBKDF2_sha256;
using TestHKDF_sha                      = HKDF_sha;
using TestHMAC_sha                      = HMAC_sha;

// Helper class to verify that all mbedTLS heap objects are released at the end of a test.
#if defined(MBEDTLS_MEMORY_DEBUG)
class HeapChecker
{
public:
    explicit HeapChecker()
    {
        size_t numBlocks;
        mbedtls_memory_buffer_alloc_cur_get(&mHeapBytesUsed, &numBlocks);
    }

    ~HeapChecker()
    {
        size_t bytesUsed;
        size_t numBlocks;
        mbedtls_memory_buffer_alloc_cur_get(&bytesUsed, &numBlocks);

        if (bytesUsed != mHeapBytesUsed)
        {
            mbedtls_memory_buffer_alloc_status();
            EXPECT_EQ(bytesUsed, mHeapBytesUsed);
        }
    }

private:
    size_t mHeapBytesUsed;
};
#else
class HeapChecker
{
public:
    explicit HeapChecker() {}
};
#endif

#include "DacValidationExplicitVectors.h"

// Verify that two HKDF keys are equal by checking if they generate the same attestation challenge.
// Note that the keys cannot be compared directly because they are given as key handles.
void AssertKeysEqual(SessionKeystore & keystore, HkdfKeyHandle & left, const HkdfKeyHandle & right)
{
    auto generateChallenge = [&](const HkdfKeyHandle & key, AttestationChallenge & challenge) -> void {
        constexpr uint8_t kTestSalt[] = { 'T', 'E', 'S', 'T', 'S', 'A', 'L', 'T' };
        constexpr uint8_t kTestInfo[] = { 'T', 'E', 'S', 'T', 'I', 'N', 'F', 'O' };

        Aes128KeyHandle i2rKey;
        Aes128KeyHandle r2iKey;

        CHIP_ERROR error = keystore.DeriveSessionKeys(key, ByteSpan(kTestSalt), ByteSpan(kTestInfo), i2rKey, r2iKey, challenge);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // Ignore the keys, just return the attestation challenge
        keystore.DestroyKey(i2rKey);
        keystore.DestroyKey(r2iKey);
    };

    AttestationChallenge leftChallenge;
    AttestationChallenge rightChallenge;

    generateChallenge(left, leftChallenge);
    generateChallenge(right, rightChallenge);

    EXPECT_EQ(memcmp(leftChallenge.ConstBytes(), rightChallenge.ConstBytes(), AttestationChallenge::Capacity()), 0);
}

} // namespace
  //

#if CHIP_CRYPTO_OPENSSL || CHIP_CRYPTO_MBEDTLS

static uint32_t gs_test_entropy_source_called = 0;
static int test_entropy_source(void * data, uint8_t * output, size_t len, size_t * olen)
{
    *olen = len;
    gs_test_entropy_source_called++;
    return 0;
}

#endif // CHIP_CRYPTO_OPENSSL || CHIP_CRYPTO_MBEDTLS

constexpr size_t KEY_LENGTH   = Crypto::kAES_CCM128_Key_Length;
constexpr size_t NONCE_LENGTH = Crypto::kAES_CCM128_Nonce_Length;

struct AesCtrTestEntry
{
    const uint8_t * key;   ///< Key to use for AES-CTR-128 encryption/decryption -- 16 byte length
    const uint8_t * nonce; ///< Nonce to use for AES-CTR-128 encryption/decryption -- 13 byte length
    const uint8_t * plaintext;
    size_t plaintextLen;
    const uint8_t * ciphertext;
    size_t ciphertextLen;
};

/**
 * Test vectors for AES-CTR-128 encryption/decryption.
 *
 * Sourced from: https://www.ietf.org/rfc/rfc3686.txt (Section 6)
 * Modified to use `IV = flags byte | 13 byte nonce | u16 counter` as defined in NIST SP 800-38A.
 *
 * All AES-CCM test vectors can be used as well, but those are already called to validate underlying AES-CCM functionality.
 */
const AesCtrTestEntry theAesCtrTestVector[] = {
    {
        .key           = (const uint8_t *) "\xae\x68\x52\xf8\x12\x10\x67\xcc\x4b\xf7\xa5\x76\x55\x77\xf3\x9e",
        .nonce         = (const uint8_t *) "\x00\x00\x00\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        .plaintext     = (const uint8_t *) "\x53\x69\x6e\x67\x6c\x65\x20\x62\x6c\x6f\x63\x6b\x20\x6d\x73\x67",
        .plaintextLen  = 16,
        .ciphertext    = (const uint8_t *) "\x0d\x0a\x6b\x6d\xc1\xf6\x9b\x4d\x14\xca\x4c\x15\x42\x22\x42\xc4",
        .ciphertextLen = 16,
    },
    {
        .key           = (const uint8_t *) "\x7e\x24\x06\x78\x17\xfa\xe0\xd7\x43\xd6\xce\x1f\x32\x53\x91\x63",
        .nonce         = (const uint8_t *) "\x00\x6c\xb6\xdb\xc0\x54\x3b\x59\xda\x48\xd9\x0b\x00",
        .plaintext     = (const uint8_t *) "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
                                           "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f",
        .plaintextLen  = 32,
        .ciphertext    = (const uint8_t *) "\x4f\x3d\xf9\x49\x15\x88\x4d\xe0\xdc\x0e\x30\x95\x0d\xe7\xa6\xe9"
                                           "\x5a\x91\x7e\x1d\x06\x42\x22\xdb\x2f\x6e\xc7\x3d\x99\x4a\xd9\x5f",
        .ciphertextLen = 32,
    }
};

#if !(CHIP_CRYPTO_KEYSTORE_APP)
struct TestAesKey
{
public:
    TestAesKey(const uint8_t * keyBytes, size_t keyLength)
    {
        Crypto::Symmetric128BitsKeyByteArray keyMaterial;
        memcpy(&keyMaterial, keyBytes, keyLength);

        CHIP_ERROR err = keystore.CreateKey(keyMaterial, key);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    ~TestAesKey() { keystore.DestroyKey(key); }

    DefaultSessionKeystore keystore;
    Aes128KeyHandle key;
};

struct TestHmacKey
{
public:
    TestHmacKey(const uint8_t * keyBytes, size_t keyLength)
    {
        Crypto::Symmetric128BitsKeyByteArray keyMaterial;
        memcpy(&keyMaterial, keyBytes, keyLength);

        CHIP_ERROR err = keystore.CreateKey(keyMaterial, key);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    ~TestHmacKey() { keystore.DestroyKey(key); }

    DefaultSessionKeystore keystore;
    Hmac128KeyHandle key;
};
#endif

static void TestAES_CTR_128_Encrypt(const AesCtrTestEntry * vector)
{
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuffer;
    outBuffer.Alloc(vector->ciphertextLen);
    EXPECT_TRUE(outBuffer);

    TestAesKey key(vector->key, KEY_LENGTH);

    CHIP_ERROR err = AES_CTR_crypt(vector->plaintext, vector->plaintextLen, key.key, vector->nonce, NONCE_LENGTH, outBuffer.Get());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    bool outputMatches = memcmp(outBuffer.Get(), vector->ciphertext, vector->ciphertextLen) == 0;
    EXPECT_TRUE(outputMatches);
    if (!outputMatches)
    {
        printf("\n Test failed due to mismatching ciphertext\n");
    }
}

static void TestAES_CTR_128_Decrypt(const AesCtrTestEntry * vector)
{
    chip::Platform::ScopedMemoryBuffer<uint8_t> outBuffer;
    outBuffer.Alloc(vector->plaintextLen);
    EXPECT_TRUE(outBuffer);

    TestAesKey key(vector->key, KEY_LENGTH);

    CHIP_ERROR err =
        AES_CTR_crypt(vector->ciphertext, vector->ciphertextLen, key.key, vector->nonce, NONCE_LENGTH, outBuffer.Get());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    bool outputMatches = memcmp(outBuffer.Get(), vector->plaintext, vector->plaintextLen) == 0;
    EXPECT_TRUE(outputMatches);
    if (!outputMatches)
    {
        printf("\n Test failed due to mismatching plaintext\n");
    }
}

struct TestChipCryptoPAL : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
#if CHIP_CRYPTO_PSA
        psa_crypto_init();
        psa_wipe_all_key_slots();
        psa_initialize_key_slots();
#endif
    }
};

TEST_F(TestChipCryptoPAL, TestAES_CTR_128CryptTestVectors)
{
    HeapChecker heapChecker;
    int numOfTestsRan = 0;
    for (const auto & vector : theAesCtrTestVector)
    {
        if (vector.plaintextLen > 0)
        {
            numOfTestsRan++;
            TestAES_CTR_128_Encrypt(&vector);
            TestAES_CTR_128_Decrypt(&vector);
        }
    }
    EXPECT_GT(numOfTestsRan, 0);
}

TEST_F(TestChipCryptoPAL, TestAES_CCM_128EncryptTestVectors)
{
    HeapChecker heapChecker;
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_ct;
            out_ct.Alloc(vector->ct_len);
            EXPECT_TRUE(out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            EXPECT_TRUE(out_tag);

            TestAesKey key(vector->key, vector->key_len);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, key.key, vector->nonce,
                                             vector->nonce_len, out_ct.Get(), out_tag.Get(), vector->tag_len);
            EXPECT_EQ(err, vector->result);

            if (vector->result == CHIP_NO_ERROR)
            {
                bool areCTsEqual  = memcmp(out_ct.Get(), vector->ct, vector->ct_len) == 0;
                bool areTagsEqual = memcmp(out_tag.Get(), vector->tag, vector->tag_len) == 0;
                EXPECT_TRUE(areCTsEqual);
                EXPECT_TRUE(areTagsEqual);
                if (!areCTsEqual)
                {
                    printf("\n Test %d failed due to mismatching ciphertext\n", vector->tcId);
                }
                if (!areTagsEqual)
                {
                    printf("\n Test %d failed due to mismatching tags\n", vector->tcId);
                }
            }
        }
    }
    EXPECT_GT(numOfTestsRan, 0);
}

TEST_F(TestChipCryptoPAL, TestAES_CCM_128DecryptTestVectors)
{
    HeapChecker heapChecker;
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_pt;
            out_pt.Alloc(vector->pt_len);
            EXPECT_TRUE(out_pt);

            TestAesKey key(vector->key, vector->key_len);

            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             key.key, vector->nonce, vector->nonce_len, out_pt.Get());

            EXPECT_EQ(err, vector->result);
            if (vector->result == CHIP_NO_ERROR)
            {
                bool arePTsEqual = memcmp(vector->pt, out_pt.Get(), vector->pt_len) == 0;
                EXPECT_TRUE(arePTsEqual);
                if (!arePTsEqual)
                {
                    printf("\n Test %d failed due to mismatching plaintext\n", vector->tcId);
                }
            }
        }
    }
    EXPECT_GT(numOfTestsRan, 0);
}

TEST_F(TestChipCryptoPAL, TestAES_CCM_128EncryptInvalidNonceLen)
{
    HeapChecker heapChecker;
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_ct;
            out_ct.Alloc(vector->ct_len);
            EXPECT_TRUE(out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            EXPECT_TRUE(out_tag);

            TestAesKey key(vector->key, vector->key_len);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, key.key, vector->nonce, 0,
                                             out_ct.Get(), out_tag.Get(), vector->tag_len);
            EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    EXPECT_GT(numOfTestsRan, 0);
}

TEST_F(TestChipCryptoPAL, TestAES_CCM_128EncryptInvalidTagLen)
{
    HeapChecker heapChecker;
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_ct;
            out_ct.Alloc(vector->ct_len);
            EXPECT_TRUE(out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            EXPECT_TRUE(out_tag);

            TestAesKey key(vector->key, vector->key_len);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, key.key, vector->nonce,
                                             vector->nonce_len, out_ct.Get(), out_tag.Get(), 13);
            EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    EXPECT_GT(numOfTestsRan, 0);
}

TEST_F(TestChipCryptoPAL, TestAES_CCM_128DecryptInvalidNonceLen)
{
    HeapChecker heapChecker;
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            Platform::ScopedMemoryBuffer<uint8_t> out_pt;
            out_pt.Alloc(vector->pt_len);
            EXPECT_TRUE(out_pt);

            TestAesKey key(vector->key, vector->key_len);

            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             key.key, vector->nonce, 0, out_pt.Get());
            EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    EXPECT_GT(numOfTestsRan, 0);
}

TEST_F(TestChipCryptoPAL, TestSensitiveDataBuffer)
{
    HeapChecker heapChecker;

    constexpr size_t kCapacity         = 32;
    constexpr size_t kLength           = 16;
    using Buffer                       = SensitiveDataBuffer<kCapacity>;
    const uint8_t kAllZeros[kCapacity] = { 0 };
    uint8_t testVector[kCapacity];

    // Give us some data.
    CHIP_ERROR err = DRBG_get_bytes(testVector, sizeof(testVector));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test initial value
    Buffer buffer;
    EXPECT_EQ(buffer.ConstBytes(), (const uint8_t *) buffer.Bytes());
    EXPECT_EQ(buffer.Length(), 0u);

    // Put data in the buffer and test all accessors
    memcpy(buffer.Bytes(), testVector, kCapacity);
    buffer.SetLength(kLength);

    EXPECT_EQ(buffer.ConstBytes(), (const uint8_t *) buffer.Bytes());
    EXPECT_EQ(buffer.ConstBytes(), buffer.Span().data());
    EXPECT_EQ(buffer.Length(), kLength);
    EXPECT_EQ(buffer.Length(), buffer.Span().size());

    // Test sanitization of entire buffer (even though length < capacity)
    const void * bufferStorage = buffer.ConstBytes();
    buffer.~Buffer();
    EXPECT_EQ(memcmp(bufferStorage, kAllZeros, kCapacity), 0);
    EXPECT_TRUE(memcmp(bufferStorage, testVector, kCapacity));
}

TEST_F(TestChipCryptoPAL, TestSensitiveDataFixedBuffer)
{
    HeapChecker heapChecker;

    constexpr size_t kCapacity         = 32;
    using Buffer                       = SensitiveDataFixedBuffer<kCapacity>;
    using BufferSpan                   = FixedByteSpan<kCapacity>;
    const uint8_t kAllZeros[kCapacity] = { 0 };
    uint8_t testVector[kCapacity];

    // Give us some data.
    CHIP_ERROR err = DRBG_get_bytes(testVector, sizeof(testVector));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test construction from array and all accessors
    Buffer buffer(testVector);
    EXPECT_EQ(buffer.ConstBytes(), (const uint8_t *) buffer.Bytes());
    EXPECT_EQ(buffer.ConstBytes(), buffer.Span().data());
    EXPECT_EQ(memcmp(buffer.ConstBytes(), testVector, kCapacity), 0);

    // Test sanitization
    const void * bufferStorage = buffer.ConstBytes();
    buffer.~Buffer();
    EXPECT_EQ(memcmp(bufferStorage, kAllZeros, kCapacity), 0);
    EXPECT_TRUE(memcmp(bufferStorage, testVector, kCapacity));

    // Give us different data
    err = DRBG_get_bytes(testVector, sizeof(testVector));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test construction from span and all accessors
    new (&buffer) Buffer(BufferSpan(testVector));
    EXPECT_EQ(buffer.ConstBytes(), (const uint8_t *) buffer.Bytes());
    EXPECT_EQ(buffer.ConstBytes(), buffer.Span().data());
    EXPECT_EQ(memcmp(buffer.ConstBytes(), testVector, kCapacity), 0);
}

TEST_F(TestChipCryptoPAL, TestAsn1Conversions)
{
    HeapChecker heapChecker;
    static_assert(sizeof(kDerSigConvDerCase4) == (sizeof(kDerSigConvRawCase4) + chip::Crypto::kMax_ECDSA_X9Dot62_Asn1_Overhead),
                  "kDerSigConvDerCase4 must have worst case overhead");

    int numOfTestVectors = ArraySize(kDerSigConvTestVectors);
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const der_sig_conv_vector * vector = &kDerSigConvTestVectors[vectorIndex];

        chip::Platform::ScopedMemoryBuffer<uint8_t> out_raw_sig;
        size_t out_raw_sig_allocated_size = vector->fe_length_bytes * 2;
        out_raw_sig.Calloc(out_raw_sig_allocated_size);
        EXPECT_TRUE(out_raw_sig);

        chip::Platform::ScopedMemoryBuffer<uint8_t> out_der_sig;
        size_t out_der_sig_allocated_size = (vector->fe_length_bytes * 2) + kMax_ECDSA_X9Dot62_Asn1_Overhead;
        out_der_sig.Calloc(out_der_sig_allocated_size);
        EXPECT_TRUE(out_der_sig);

        // Test conversion from ASN.1 ER to raw
        MutableByteSpan out_raw_sig_span(out_raw_sig.Get(), out_raw_sig_allocated_size);

        CHIP_ERROR status = EcdsaAsn1SignatureToRaw(vector->fe_length_bytes,
                                                    ByteSpan{ vector->der_version, vector->der_version_length }, out_raw_sig_span);
        EXPECT_EQ(status, CHIP_NO_ERROR);
        EXPECT_EQ(out_raw_sig_span.size(), vector->raw_version_length);
        EXPECT_EQ(memcmp(out_raw_sig_span.data(), vector->raw_version, vector->raw_version_length), 0);

        // Test conversion from raw to ASN.1 DER
        MutableByteSpan out_der_sig_span(out_der_sig.Get(), out_der_sig_allocated_size);
        status = EcdsaRawSignatureToAsn1(vector->fe_length_bytes, ByteSpan{ vector->raw_version, vector->raw_version_length },
                                         out_der_sig_span);
        EXPECT_EQ(status, CHIP_NO_ERROR);
        EXPECT_LE(out_der_sig_span.size(), out_der_sig_allocated_size);
        EXPECT_EQ(out_der_sig_span.size(), vector->der_version_length);
        EXPECT_EQ(memcmp(out_der_sig_span.data(), vector->der_version, vector->der_version_length), 0);
    }
}

TEST_F(TestChipCryptoPAL, TestRawIntegerToDerValidCases)
{
    HeapChecker heapChecker;
    int numOfTestCases = ArraySize(kRawIntegerToDerVectors);

    for (int testIdx = 0; testIdx < numOfTestCases; testIdx++)
    {
        RawIntegerToDerVector v = kRawIntegerToDerVectors[testIdx];

        // Cover case with tag/length
        {
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_der_buffer;
            out_der_buffer.Alloc(v.expected_size);
            EXPECT_TRUE(out_der_buffer);

            MutableByteSpan out_der_integer(out_der_buffer.Get(), v.expected_size);
            CHIP_ERROR status = ConvertIntegerRawToDer(ByteSpan{ v.candidate, v.candidate_size }, out_der_integer);
            EXPECT_EQ(status, CHIP_NO_ERROR);
            EXPECT_EQ(out_der_integer.size(), v.expected_size);
            EXPECT_TRUE(out_der_integer.data_equal(ByteSpan(v.expected, v.expected_size)));

            // Cover case of buffer too small
            MutableByteSpan out_der_integer_too_small(out_der_buffer.Get(), v.expected_size - 1);
            status = ConvertIntegerRawToDer(ByteSpan{ v.candidate, v.candidate_size }, out_der_integer_too_small);
            EXPECT_EQ(status, CHIP_ERROR_BUFFER_TOO_SMALL);
        }

        // Cover case without tag/length
        {
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_der_buffer;
            out_der_buffer.Alloc(v.expected_without_tag_size);
            EXPECT_TRUE(out_der_buffer);

            MutableByteSpan out_der_integer(out_der_buffer.Get(), v.expected_without_tag_size);
            CHIP_ERROR status = ConvertIntegerRawToDerWithoutTag(ByteSpan{ v.candidate, v.candidate_size }, out_der_integer);

            EXPECT_EQ(status, CHIP_NO_ERROR);
            EXPECT_EQ(out_der_integer.size(), v.expected_without_tag_size);
            EXPECT_TRUE(out_der_integer.data_equal(ByteSpan(v.expected_without_tag, v.expected_without_tag_size)));
        }
    }
}

TEST_F(TestChipCryptoPAL, TestRawIntegerToDerInvalidCases)
{
    HeapChecker heapChecker;
    // Cover case of invalid buffers
    uint8_t placeholder[10] = { 0 };
    MutableByteSpan good_out_buffer(placeholder);
    ByteSpan good_buffer(placeholder);

    MutableByteSpan bad_out_buffer_empty;
    ByteSpan bad_buffer_empty;

    struct ErrorCase
    {
        const ByteSpan & input;
        MutableByteSpan & output;
        CHIP_ERROR expected_status;
    };

    const ErrorCase error_cases[] = {
        { .input = good_buffer, .output = bad_out_buffer_empty, .expected_status = CHIP_ERROR_INVALID_ARGUMENT },
        { .input = bad_buffer_empty, .output = good_out_buffer, .expected_status = CHIP_ERROR_INVALID_ARGUMENT }
    };

    [[maybe_unused]] int case_idx = 0;
    for (const ErrorCase & v : error_cases)
    {
        CHIP_ERROR status = ConvertIntegerRawToDerWithoutTag(v.input, v.output);
        if (status != v.expected_status)
        {
            ChipLogError(Crypto, "Failed TestRawIntegerToDerInvalidCases sub-case %d", case_idx);
            EXPECT_EQ(v.expected_status, status);
        }
        ++case_idx;
    }
}

TEST_F(TestChipCryptoPAL, TestReadDerLengthValidCases)
{
    const uint8_t short_zero_length[] = { 0x00 };
    ByteSpan short_zero_length_buf(short_zero_length);

    const uint8_t short_length[] = { 0x15 };
    ByteSpan short_length_buf(short_length);

    const uint8_t single_byte_length[] = { 0x81, 0x80 };
    ByteSpan single_byte_length_buf(single_byte_length);

    const uint8_t single_byte_length_large[] = { 0x81, 0xFF };
    ByteSpan single_byte_length_large_buf(single_byte_length_large);

    const uint8_t two_byte_length[] = { 0x82, 0xFF, 0x01 };
    ByteSpan two_byte_length_buf(two_byte_length);

    const uint8_t three_byte_length[] = { 0x83, 0xFF, 0x00, 0xAA };
    ByteSpan three_byte_length_buf(three_byte_length);

    const uint8_t four_byte_length[] = { 0x84, 0x01, 0x02, 0x03, 0x04 };
    ByteSpan four_byte_length_buf(four_byte_length);

    const uint8_t four_byte_length_large[] = { 0x84, 0xFF, 0xFF, 0xFF, 0xFF };
    ByteSpan four_byte_length_large_buf(four_byte_length_large);

    uint8_t max_byte_length_large[1 + sizeof(size_t)];
    ByteSpan max_byte_length_large_buf(max_byte_length_large);

    // We build a DER length value of SIZE_MAX programmatically.
    max_byte_length_large[0] = 0x80 | sizeof(size_t);
    memset(&max_byte_length_large[1], 0xFF, sizeof(size_t));

    struct SuccessCase
    {
        const ByteSpan & input_buf;
        const size_t expected_length;
    };

    const SuccessCase cases[] = {
        { .input_buf = short_zero_length_buf, .expected_length = static_cast<size_t>(0x00) },
        { .input_buf = short_length_buf, .expected_length = static_cast<size_t>(0x15) },
        { .input_buf = single_byte_length_buf, .expected_length = static_cast<size_t>(0x80) },
        { .input_buf = single_byte_length_large_buf, .expected_length = static_cast<size_t>(0xFF) },
        { .input_buf = two_byte_length_buf, .expected_length = static_cast<size_t>(0xFF01) },
        { .input_buf = three_byte_length_buf, .expected_length = static_cast<size_t>(0xFF00AAUL) },
        { .input_buf = four_byte_length_buf, .expected_length = static_cast<size_t>(0x01020304UL) },
        { .input_buf = four_byte_length_large_buf, .expected_length = static_cast<size_t>(0xFFFFFFFFUL) },
        { .input_buf = max_byte_length_large_buf, .expected_length = SIZE_MAX },
    };

    [[maybe_unused]] int case_idx = 0;
    for (const SuccessCase & v : cases)
    {
        size_t output_length = SIZE_MAX - 1;
        chip::Encoding::LittleEndian::Reader input_reader{ v.input_buf };
        CHIP_ERROR status = ReadDerLength(input_reader, output_length);
        if ((status != CHIP_NO_ERROR) || (v.expected_length != output_length))
        {
            ChipLogError(Crypto, "Failed TestReadDerLengthValidCases sub-case %d", case_idx);
            EXPECT_EQ(output_length, v.expected_length);
            EXPECT_EQ(status, CHIP_NO_ERROR);
        }
        ++case_idx;
    }
}

TEST_F(TestChipCryptoPAL, TestReadDerLengthInvalidCases)
{
    uint8_t placeholder[1];

    ByteSpan bad_buffer_empty(placeholder, 0);

    const uint8_t zero_multi_byte_length[] = { 0x80 };
    ByteSpan zero_multi_byte_length_buf(zero_multi_byte_length);

    const uint8_t single_byte_length_zero[] = { 0x81, 0x00 };
    ByteSpan single_byte_length_zero_buf(single_byte_length_zero);

    const uint8_t single_byte_length_too_small[] = { 0x81, 0x7F };
    ByteSpan single_byte_length_too_small_buf(single_byte_length_too_small);

    const uint8_t multiple_byte_length_zero_padded[] = { 0x82, 0x00, 0xFF };
    ByteSpan multiple_byte_length_zero_padded_buf(multiple_byte_length_zero_padded);

    const uint8_t multiple_byte_length_insufficient_bytes[] = { 0x84, 0xFF, 0xAA, 0x01 };
    ByteSpan multiple_byte_length_insufficient_bytes_buf(multiple_byte_length_insufficient_bytes);

    const uint8_t multiple_byte_length_insufficient_bytes2[] = { 0x83 };
    ByteSpan multiple_byte_length_insufficient_bytes2_buf(multiple_byte_length_insufficient_bytes2);

    uint8_t max_byte_length_large_insufficient_bytes[1 + sizeof(size_t) - 1];
    ByteSpan max_byte_length_large_insufficient_bytes_buf(max_byte_length_large_insufficient_bytes);

    // We build a DER length value of SIZE_MAX programmatically, with one byte too few.
    max_byte_length_large_insufficient_bytes[0] = 0x80 | sizeof(size_t);
    memset(&max_byte_length_large_insufficient_bytes[1], 0xFF, sizeof(max_byte_length_large_insufficient_bytes) - 1);

    struct ErrorCase
    {
        const ByteSpan & input_buf;
        CHIP_ERROR expected_status;
    };

    const ErrorCase error_cases[] = {
        { .input_buf = bad_buffer_empty, .expected_status = CHIP_ERROR_BUFFER_TOO_SMALL },
        { .input_buf = zero_multi_byte_length_buf, .expected_status = CHIP_ERROR_INVALID_ARGUMENT },
        { .input_buf = single_byte_length_zero_buf, .expected_status = CHIP_ERROR_INVALID_ARGUMENT },
        { .input_buf = single_byte_length_too_small_buf, .expected_status = CHIP_ERROR_INVALID_ARGUMENT },
        { .input_buf = multiple_byte_length_zero_padded_buf, .expected_status = CHIP_ERROR_INVALID_ARGUMENT },
        { .input_buf = multiple_byte_length_insufficient_bytes_buf, .expected_status = CHIP_ERROR_BUFFER_TOO_SMALL },
        { .input_buf = multiple_byte_length_insufficient_bytes2_buf, .expected_status = CHIP_ERROR_BUFFER_TOO_SMALL },
        { .input_buf = max_byte_length_large_insufficient_bytes_buf, .expected_status = CHIP_ERROR_BUFFER_TOO_SMALL },
    };

    [[maybe_unused]] int case_idx = 0;
    for (const ErrorCase & v : error_cases)
    {
        size_t output_length = SIZE_MAX;
        chip::Encoding::LittleEndian::Reader input_reader{ v.input_buf };
        CHIP_ERROR status = ReadDerLength(input_reader, output_length);
        if (status != v.expected_status)
        {
            ChipLogError(Crypto, "Failed TestReadDerLengthInvalidCases sub-case %d", case_idx);
            EXPECT_EQ(v.expected_status, status);
        }
        ++case_idx;
    }
}

TEST_F(TestChipCryptoPAL, TestHash_SHA256)
{
    HeapChecker heapChecker;
    unsigned int numOfTestCases     = ArraySize(hash_sha256_test_vectors);
    unsigned int numOfTestsExecuted = 0;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hash_sha256_vector v = hash_sha256_test_vectors[numOfTestsExecuted];
        uint8_t out_buffer[kSHA256_Hash_Length];
        Hash_SHA256(v.data, v.data_length, out_buffer);
        bool success = memcmp(v.hash, out_buffer, sizeof(out_buffer)) == 0;
        EXPECT_TRUE(success);
    }
    EXPECT_EQ(numOfTestsExecuted, ArraySize(hash_sha256_test_vectors));
}

TEST_F(TestChipCryptoPAL, TestHash_SHA256_Stream)
{
    HeapChecker heapChecker;
    unsigned int numOfTestCases     = ArraySize(hash_sha256_test_vectors);
    unsigned int numOfTestsExecuted = 0;
    CHIP_ERROR error                = CHIP_NO_ERROR;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hash_sha256_vector v = hash_sha256_test_vectors[numOfTestsExecuted];
        const uint8_t * data = v.data;
        size_t data_length   = v.data_length;
        uint8_t out_buffer[kSHA256_Hash_Length];

        Hash_SHA256_stream sha256;

        error = sha256.Begin();
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // Split data into 3 random streams.
        for (int i = 0; i < 2; ++i)
        {
            size_t rand_data_length = static_cast<unsigned int>(rand()) % (data_length + 1);

            error = sha256.AddData(ByteSpan{ data, rand_data_length });
            EXPECT_EQ(error, CHIP_NO_ERROR);

            data += rand_data_length;
            data_length -= rand_data_length;
        }

        error = sha256.AddData(ByteSpan{ data, data_length });
        EXPECT_EQ(error, CHIP_NO_ERROR);

        MutableByteSpan out_span(out_buffer);
        error = sha256.Finish(out_span);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(out_span.size(), kSHA256_Hash_Length);

        bool success = memcmp(v.hash, out_span.data(), out_span.size()) == 0;
        EXPECT_TRUE(success);
    }

    EXPECT_EQ(numOfTestsExecuted, ArraySize(hash_sha256_test_vectors));

    // Test partial digests
    uint8_t source_buf[2 * kSHA256_Hash_Length];

    // Use a basic counter for all data
    for (size_t idx = 0; idx < sizeof(source_buf); idx++)
    {
        source_buf[idx] = static_cast<uint8_t>(idx & 0xFFu);
    }

    // Use split blocks of every length including digest length, to cover
    // all padding cases.
    for (size_t block1_size = 1; block1_size <= kSHA256_Hash_Length; block1_size++)
    {
        for (size_t block2_size = 1; block2_size <= kSHA256_Hash_Length; block2_size++)
        {
            uint8_t partial_digest1[kSHA256_Hash_Length];
            uint8_t partial_digest2[kSHA256_Hash_Length];
            uint8_t partial_digest_ref[kSHA256_Hash_Length];
            uint8_t total_digest[kSHA256_Hash_Length];
            uint8_t total_digest_ref[kSHA256_Hash_Length];
            MutableByteSpan partial_digest_span1(partial_digest1);
            MutableByteSpan partial_digest_span2(partial_digest2);
            MutableByteSpan total_digest_span(total_digest);

            Hash_SHA256_stream sha256;
            EXPECT_EQ(sha256.Begin(), CHIP_NO_ERROR);

            // Compute partial digest after first block
            EXPECT_EQ(sha256.AddData(ByteSpan{ &source_buf[0], block1_size }), CHIP_NO_ERROR);
            EXPECT_EQ(sha256.GetDigest(partial_digest_span1), CHIP_NO_ERROR);
            EXPECT_EQ(partial_digest_span1.size(), kSHA256_Hash_Length);

            // Validate partial digest matches expectations
            Hash_SHA256(&source_buf[0], block1_size, &partial_digest_ref[0]);
            EXPECT_EQ(0, memcmp(partial_digest_span1.data(), partial_digest_ref, partial_digest_span1.size()));

            // Compute partial digest and total digest after second block
            EXPECT_EQ(sha256.AddData(ByteSpan{ &source_buf[block1_size], block2_size }), CHIP_NO_ERROR);

            EXPECT_EQ(sha256.GetDigest(partial_digest_span2), CHIP_NO_ERROR);
            EXPECT_EQ(partial_digest_span2.size(), kSHA256_Hash_Length);

            EXPECT_EQ(sha256.Finish(total_digest_span), CHIP_NO_ERROR);
            EXPECT_EQ(total_digest_span.size(), kSHA256_Hash_Length);

            // Validate second partial digest matches final digest
            Hash_SHA256(&source_buf[0], block1_size + block2_size, &total_digest_ref[0]);
            EXPECT_EQ(0, memcmp(partial_digest_span2.data(), total_digest_ref, partial_digest_span2.size()));
            EXPECT_EQ(0, memcmp(total_digest_span.data(), total_digest_ref, total_digest_span.size()));
        }
    }

    // Validate error cases
    {
        uint8_t source_buf2[5] = { 1, 2, 3, 4, 5 };
        uint8_t digest_buf_too_small[kSHA256_Hash_Length - 1];
        uint8_t digest_buf_ok[kSHA256_Hash_Length];
        uint8_t digest_buf_ref[kSHA256_Hash_Length];
        MutableByteSpan digest_span_too_small(digest_buf_too_small);
        MutableByteSpan digest_span_ok(digest_buf_ok);

        Hash_SHA256(&source_buf2[0], sizeof(source_buf2), &digest_buf_ref[0]);

        Hash_SHA256_stream sha256;
        EXPECT_EQ(sha256.Begin(), CHIP_NO_ERROR);

        EXPECT_EQ(sha256.AddData(ByteSpan{ source_buf2 }), CHIP_NO_ERROR);

        // Check that error behavior works on buffer too small
        EXPECT_EQ(sha256.GetDigest(digest_span_too_small), CHIP_ERROR_BUFFER_TOO_SMALL);
        EXPECT_EQ(sha256.Finish(digest_span_too_small), CHIP_ERROR_BUFFER_TOO_SMALL);

        // Check that both GetDigest/Finish can still work after error.
        EXPECT_EQ(sha256.GetDigest(digest_span_ok), CHIP_NO_ERROR);
        EXPECT_EQ(0, memcmp(digest_span_ok.data(), digest_buf_ref, digest_span_ok.size()));

        memset(digest_buf_ok, 0, sizeof(digest_buf_ok));

        EXPECT_EQ(sha256.Finish(digest_span_ok), CHIP_NO_ERROR);
        EXPECT_EQ(0, memcmp(digest_span_ok.data(), digest_buf_ref, digest_span_ok.size()));
    }
}

TEST_F(TestChipCryptoPAL, TestHMAC_SHA256_RawKey)
{
    HeapChecker heapChecker;
    int numOfTestCases     = ArraySize(hmac_sha256_test_vectors_raw_key);
    int numOfTestsExecuted = 0;
    TestHMAC_sha mHMAC;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hmac_sha256_vector v = hmac_sha256_test_vectors_raw_key[numOfTestsExecuted];
        size_t out_length    = v.output_hash_length;
        chip::Platform::ScopedMemoryBuffer<uint8_t> out_buffer;
        out_buffer.Alloc(out_length);
        EXPECT_TRUE(out_buffer);
        mHMAC.HMAC_SHA256(v.key, v.key_length, v.message, v.message_length, out_buffer.Get(), v.output_hash_length);
        bool success = memcmp(v.output_hash, out_buffer.Get(), out_length) == 0;
        EXPECT_TRUE(success);
    }
    EXPECT_EQ(numOfTestsExecuted, numOfTestCases);
}

#if !(CHIP_CRYPTO_KEYSTORE_APP)
TEST_F(TestChipCryptoPAL, TestHMAC_SHA256_KeyHandle)
{
    HeapChecker heapChecker;
    int numOfTestCases     = ArraySize(hmac_sha256_test_vectors_key_handle);
    int numOfTestsExecuted = 0;
    TestHMAC_sha mHMAC;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hmac_sha256_vector v = hmac_sha256_test_vectors_key_handle[numOfTestsExecuted];
        size_t out_length    = v.output_hash_length;
        chip::Platform::ScopedMemoryBuffer<uint8_t> out_buffer;
        out_buffer.Alloc(out_length);
        EXPECT_TRUE(out_buffer);
        Crypto::DefaultSessionKeystore keystore;

        Symmetric128BitsKeyByteArray keyMaterial;
        memcpy(keyMaterial, v.key, v.key_length);

        Hmac128KeyHandle keyHandle;
        EXPECT_EQ(keystore.CreateKey(keyMaterial, keyHandle), CHIP_NO_ERROR);

        mHMAC.HMAC_SHA256(keyHandle, v.message, v.message_length, out_buffer.Get(), v.output_hash_length);
        bool success = memcmp(v.output_hash, out_buffer.Get(), out_length) == 0;
        EXPECT_TRUE(success);

        keystore.DestroyKey(keyHandle);
    }
    EXPECT_EQ(numOfTestsExecuted, numOfTestCases);
}
#endif

TEST_F(TestChipCryptoPAL, TestHKDF_SHA256)
{
    HeapChecker heapChecker;
    int numOfTestCases     = ArraySize(hkdf_sha256_test_vectors);
    int numOfTestsExecuted = 0;
    TestHKDF_sha mHKDF;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hkdf_sha256_vector v = hkdf_sha256_test_vectors[numOfTestsExecuted];
        size_t out_length    = v.output_key_material_length;
        chip::Platform::ScopedMemoryBuffer<uint8_t> out_buffer;
        out_buffer.Alloc(out_length);
        EXPECT_TRUE(out_buffer);
        mHKDF.HKDF_SHA256(v.initial_key_material, v.initial_key_material_length, v.salt, v.salt_length, v.info, v.info_length,
                          out_buffer.Get(), v.output_key_material_length);
        bool success = memcmp(v.output_key_material, out_buffer.Get(), out_length) == 0;
        EXPECT_TRUE(success);
    }
    EXPECT_EQ(numOfTestsExecuted, 3);
}

TEST_F(TestChipCryptoPAL, TestDRBG_InvalidInputs)
{
    HeapChecker heapChecker;
    CHIP_ERROR error = CHIP_NO_ERROR;
    error            = DRBG_get_bytes(nullptr, 10);
    EXPECT_EQ(error, CHIP_ERROR_INVALID_ARGUMENT);
    error = CHIP_NO_ERROR;
    uint8_t buffer[5];
    error = DRBG_get_bytes(buffer, 0);
    EXPECT_EQ(error, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestChipCryptoPAL, TestDRBG_Output)
{
    // No good way to unit test a DRBG. Just validate that we get out something
    CHIP_ERROR error     = CHIP_ERROR_INVALID_ARGUMENT;
    uint8_t out_buf[10]  = { 0 };
    uint8_t orig_buf[10] = { 0 };

    error = DRBG_get_bytes(out_buf, sizeof(out_buf));
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_NE(memcmp(out_buf, orig_buf, sizeof(out_buf)), 0);
}

TEST_F(TestChipCryptoPAL, TestECDSA_Signing_SHA256_Msg)
{
    HeapChecker heapChecker;
    const char * msg  = "Hello World!";
    size_t msg_length = strlen(msg);

    Test_P256Keypair keypair;

    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    EXPECT_EQ(signing_error, CHIP_NO_ERROR);

    CHIP_ERROR validation_error =
        keypair.Pubkey().ECDSA_validate_msg_signature(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    EXPECT_EQ(validation_error, CHIP_NO_ERROR);
}

TEST_F(TestChipCryptoPAL, TestECDSA_Signing_SHA256_Hash)
{
    HeapChecker heapChecker;
    const uint8_t msg[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    size_t msg_length   = sizeof(msg);

    Test_P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    // TODO: Need to make this large number (1k+) to catch some signature serialization corner cases
    //       but this is too slow on QEMU/embedded, so we need to parametrize. Signing with ECDSA
    //       is non-deterministic by design (since knowledge of the value `k` used allows recovery
    //       of the private key).
    constexpr int kNumSigningIterations = 3;

    for (int i = 0; i < kNumSigningIterations; ++i)
    {
        P256ECDSASignature signature;

        uint8_t hash[Crypto::kSHA256_Hash_Length];
        EXPECT_EQ(Hash_SHA256(&msg[0], msg_length, &hash[0]), CHIP_NO_ERROR);

        CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(msg, msg_length, signature);
        EXPECT_EQ(signing_error, CHIP_NO_ERROR);

        CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(hash, sizeof(hash), signature);
        EXPECT_EQ(validation_error, CHIP_NO_ERROR);

        if ((signing_error != CHIP_NO_ERROR) || (validation_error != CHIP_NO_ERROR))
        {
            ChipLogError(Crypto, "TestECDSA_Signing_SHA256_Hash failed after %d/%d iterations", i + 1, kNumSigningIterations);
            break;
        }
    }
}

TEST_F(TestChipCryptoPAL, TestECDSA_ValidationFailsDifferentMessage)
{
    HeapChecker heapChecker;
    const char * msg  = "Hello World!";
    size_t msg_length = strlen(msg);

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    EXPECT_EQ(signing_error, CHIP_NO_ERROR);

    const char * diff_msg  = "NOT Hello World!";
    size_t diff_msg_length = strlen(msg);
    CHIP_ERROR validation_error =
        keypair.Pubkey().ECDSA_validate_msg_signature(reinterpret_cast<const uint8_t *>(diff_msg), diff_msg_length, signature);
    EXPECT_EQ(validation_error, CHIP_ERROR_INVALID_SIGNATURE);
}

TEST_F(TestChipCryptoPAL, TestECDSA_ValidationFailIncorrectMsgSignature)
{
    HeapChecker heapChecker;
    const char * msg  = "Hello World!";
    size_t msg_length = strlen(msg);

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    EXPECT_EQ(signing_error, CHIP_NO_ERROR);
    signature.Bytes()[0] = static_cast<uint8_t>(~signature.ConstBytes()[0]); // Flipping bits should invalidate the signature.

    CHIP_ERROR validation_error =
        keypair.Pubkey().ECDSA_validate_msg_signature(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    EXPECT_EQ(validation_error, CHIP_ERROR_INVALID_SIGNATURE);
}

TEST_F(TestChipCryptoPAL, TestECDSA_ValidationFailIncorrectHashSignature)
{
    HeapChecker heapChecker;
    const uint8_t msg[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    size_t msg_length   = sizeof(msg);

    uint8_t hash[Crypto::kSHA256_Hash_Length];
    EXPECT_EQ(Hash_SHA256(&msg[0], msg_length, &hash[0]), CHIP_NO_ERROR);

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(msg, msg_length, signature);
    EXPECT_EQ(signing_error, CHIP_NO_ERROR);
    signature.Bytes()[0] = static_cast<uint8_t>(~signature.ConstBytes()[0]); // Flipping bits should invalidate the signature.

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(hash, sizeof(hash), signature);
    EXPECT_EQ(validation_error, CHIP_ERROR_INVALID_SIGNATURE);
}

TEST_F(TestChipCryptoPAL, TestECDSA_SigningMsgInvalidParams)
{
    HeapChecker heapChecker;
    const uint8_t * msg = reinterpret_cast<const uint8_t *>("Hello World!");
    size_t msg_length   = strlen(reinterpret_cast<const char *>(msg));

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(nullptr, msg_length, signature);
    EXPECT_EQ(signing_error, CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    signing_error = keypair.ECDSA_sign_msg(msg, 0, signature);
    EXPECT_EQ(signing_error, CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;
}

TEST_F(TestChipCryptoPAL, TestECDSA_ValidationMsgInvalidParam)
{
    HeapChecker heapChecker;
    const char * msg  = "Hello World!";
    size_t msg_length = strlen(msg);

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    EXPECT_EQ(signing_error, CHIP_NO_ERROR);

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_msg_signature(nullptr, msg_length, signature);
    EXPECT_EQ(validation_error, CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;

    validation_error = keypair.Pubkey().ECDSA_validate_msg_signature(reinterpret_cast<const uint8_t *>(msg), 0, signature);
    EXPECT_EQ(validation_error, CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;
}

TEST_F(TestChipCryptoPAL, TestECDSA_ValidationHashInvalidParam)
{
    HeapChecker heapChecker;
    const uint8_t msg[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    size_t msg_length   = sizeof(msg);

    uint8_t hash[Crypto::kSHA256_Hash_Length];
    EXPECT_EQ(Hash_SHA256(&msg[0], msg_length, &hash[0]), CHIP_NO_ERROR);

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(msg, msg_length, signature);
    EXPECT_EQ(signing_error, CHIP_NO_ERROR);

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(nullptr, sizeof(hash), signature);
    EXPECT_EQ(validation_error, CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(hash, sizeof(hash) - 5, signature);
    EXPECT_EQ(validation_error, CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;
}

TEST_F(TestChipCryptoPAL, TestECDH_EstablishSecret)
{
    HeapChecker heapChecker;
    Test_P256Keypair keypair1;
    EXPECT_EQ(keypair1.Initialize(ECPKeyTarget::ECDH), CHIP_NO_ERROR);

    Test_P256Keypair keypair2;
    EXPECT_EQ(keypair2.Initialize(ECPKeyTarget::ECDH), CHIP_NO_ERROR);

    P256ECDHDerivedSecret out_secret1;
    out_secret1.Bytes()[0] = 0;

    P256ECDHDerivedSecret out_secret2;
    out_secret2.Bytes()[0] = 1;

    CHIP_ERROR error = CHIP_NO_ERROR;
    EXPECT_NE(memcmp(out_secret1.ConstBytes(), out_secret2.ConstBytes(), out_secret1.Capacity()),
              0); // Validate that buffers are indeed different.

    error = keypair2.ECDH_derive_secret(keypair1.Pubkey(), out_secret1);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    error = keypair1.ECDH_derive_secret(keypair2.Pubkey(), out_secret2);
    EXPECT_EQ(error, CHIP_NO_ERROR);

    bool signature_lengths_match = out_secret1.Length() == out_secret2.Length();
    EXPECT_TRUE(signature_lengths_match);

    bool signatures_match = (memcmp(out_secret1.ConstBytes(), out_secret2.ConstBytes(), out_secret1.Length()) == 0);
    EXPECT_TRUE(signatures_match);
}

#if CHIP_CRYPTO_OPENSSL
TEST_F(TestChipCryptoPAL, TestAddEntropySources)
{
    HeapChecker heapChecker;
    CHIP_ERROR error = add_entropy_source(test_entropy_source, nullptr, 10);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    uint8_t buffer[5];
    EXPECT_EQ(DRBG_get_bytes(buffer, sizeof(buffer)), CHIP_NO_ERROR);
}
#endif

#if CHIP_CRYPTO_MBEDTLS
TEST_F(TestChipCryptoPAL, TestAddEntropySources)
{
    HeapChecker heapChecker;
    CHIP_ERROR error = add_entropy_source(test_entropy_source, nullptr, 10);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    uint8_t buffer[5];
    uint32_t test_entropy_source_call_count = gs_test_entropy_source_called;
    EXPECT_EQ(DRBG_get_bytes(buffer, sizeof(buffer)), CHIP_NO_ERROR);
    for (int i = 0; i < 5000 * 2; i++)
    {
        (void) DRBG_get_bytes(buffer, sizeof(buffer));
    }
    EXPECT_GT(gs_test_entropy_source_called, test_entropy_source_call_count);
}
#endif

TEST_F(TestChipCryptoPAL, TestPBKDF2_SHA256_TestVectors)
{
    HeapChecker heapChecker;
    int numOfTestVectors = ArraySize(pbkdf2_sha256_test_vectors);
    int numOfTestsRan    = 0;
    TestPBKDF2_sha256 pbkdf1;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const pbkdf2_test_vector * vector = pbkdf2_sha256_test_vectors[vectorIndex];
        if (vector->plen > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_key;
            out_key.Alloc(vector->key_len);
            EXPECT_TRUE(out_key);

            CHIP_ERROR err = pbkdf1.pbkdf2_sha256(vector->password, vector->plen, vector->salt, vector->slen, vector->iter,
                                                  vector->key_len, out_key.Get());
            EXPECT_EQ(err, vector->result);

            if (vector->result == CHIP_NO_ERROR)
            {
                EXPECT_EQ(memcmp(out_key.Get(), vector->key, vector->key_len), 0);
            }
        }
    }
    EXPECT_GT(numOfTestsRan, 0);
}

TEST_F(TestChipCryptoPAL, TestP256_Keygen)
{
    HeapChecker heapChecker;
    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    const char * msg         = "Test Message for Keygen";
    const uint8_t * test_msg = Uint8::from_const_char(msg);
    size_t msglen            = strlen(msg);

    P256ECDSASignature test_sig;
    EXPECT_EQ(keypair.ECDSA_sign_msg(test_msg, msglen, test_sig), CHIP_NO_ERROR);
    EXPECT_EQ(keypair.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig), CHIP_NO_ERROR);
}

TEST_F(TestChipCryptoPAL, TestCSR_Verify)
{
    Crypto::P256PublicKey pubKey;
    CHIP_ERROR err;

    // First case: there is trailing garbage in the CSR
    {
        const uint8_t kBadTrailingGarbageCsr[255] = {
            0x30, 0x81, 0xda, 0x30, 0x81, 0x81, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0b,
            0x0c, 0x03, 0x43, 0x53, 0x41, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08,
            0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x72, 0x48, 0xc0, 0x36, 0xf0, 0x12, 0x5f, 0xd1,
            0x68, 0x92, 0x2d, 0xee, 0x57, 0x2b, 0x8e, 0x20, 0x9d, 0x97, 0xfa, 0x73, 0x92, 0xf1, 0xa0, 0x91, 0x0e, 0xfd, 0x04, 0x93,
            0x66, 0x47, 0x3c, 0xa3, 0xf0, 0xa8, 0x47, 0xa1, 0xa3, 0x1e, 0x13, 0x3b, 0x67, 0x3b, 0x18, 0xca, 0x77, 0xd1, 0xea, 0xe3,
            0x74, 0x93, 0x49, 0x8b, 0x9d, 0xdc, 0xef, 0xf9, 0xd5, 0x9b, 0x27, 0x19, 0xad, 0x6e, 0x90, 0xd2, 0xa0, 0x11, 0x30, 0x0f,
            0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x0e, 0x31, 0x02, 0x30, 0x00, 0x30, 0x0a, 0x06, 0x08, 0x2a,
            0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x6a, 0x2e, 0x15, 0x34, 0x1b, 0xde,
            0xcb, 0x8f, 0xd2, 0xfd, 0x35, 0x03, 0x89, 0x0e, 0xed, 0x23, 0x54, 0xff, 0xcb, 0x79, 0xf9, 0xcb, 0x40, 0x33, 0x59, 0xb4,
            0x27, 0x69, 0xeb, 0x07, 0x3b, 0xd5, 0x02, 0x21, 0x00, 0xb0, 0x25, 0xc9, 0xc2, 0x21, 0xe8, 0x54, 0xcc, 0x08, 0x12, 0xf5,
            0x10, 0x3a, 0x0b, 0x25, 0x20, 0x0a, 0x61, 0x38, 0xc8, 0x6f, 0x82, 0xa7, 0x51, 0x84, 0x61, 0xae, 0x93, 0x69, 0xe4, 0x74,
            0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        };

        Crypto::ClearSecretData(pubKey.Bytes(), pubKey.Length());

        err = VerifyCertificateSigningRequest(&kBadTrailingGarbageCsr[0], sizeof(kBadTrailingGarbageCsr), pubKey);

        // On first test case, check if CSRs are supported at all, and skip test if they are not.
        if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            ChipLogError(Crypto, "The current platform does not support CSR parsing.");
            return;
        }

        EXPECT_NE(err, CHIP_NO_ERROR);

        err = VerifyCertificateSigningRequestFormat(&kBadTrailingGarbageCsr[0], sizeof(kBadTrailingGarbageCsr));
        EXPECT_EQ(err, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);
    }

    // Second case: correct CSR
    {
        const uint8_t kGoodCsr[] = {
            0x30, 0x81, 0xca, 0x30, 0x70, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0a,
            0x0c, 0x03, 0x43, 0x53, 0x52, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06,
            0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xa3, 0xbe, 0xa1, 0xf5, 0x42, 0x01,
            0x07, 0x3c, 0x4b, 0x75, 0x85, 0xd8, 0xe2, 0x98, 0xac, 0x2f, 0xf6, 0x98, 0xdb, 0xd9, 0x5b, 0xe0, 0x7e, 0xc1, 0x04,
            0xd5, 0x73, 0xc5, 0xb0, 0x90, 0x77, 0x27, 0x00, 0x1e, 0x22, 0xc7, 0x89, 0x5e, 0x4d, 0x75, 0x07, 0x89, 0x82, 0x0f,
            0x49, 0xb6, 0x59, 0xd5, 0xc5, 0x15, 0x7d, 0x93, 0xe6, 0x80, 0x5c, 0x70, 0x89, 0x0a, 0x43, 0x10, 0x3d, 0xeb, 0x3d,
            0x4a, 0xa0, 0x00, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x05, 0x00, 0x03, 0x48,
            0x00, 0x30, 0x45, 0x02, 0x20, 0x1d, 0x86, 0x21, 0xb4, 0xc2, 0xe1, 0xa9, 0xf3, 0xbc, 0xc8, 0x7c, 0xda, 0xb4, 0xb9,
            0xc6, 0x8c, 0xd0, 0xe4, 0x9a, 0x9c, 0xef, 0x02, 0x93, 0x98, 0x27, 0x7e, 0x81, 0x21, 0x5d, 0x20, 0x9d, 0x32, 0x02,
            0x21, 0x00, 0x8b, 0x6b, 0x49, 0xb6, 0x7d, 0x3e, 0x67, 0x9e, 0xb1, 0x22, 0xd3, 0x63, 0x82, 0x40, 0x4f, 0x49, 0xa4,
            0xdc, 0x17, 0x35, 0xac, 0x4b, 0x7a, 0xbf, 0x52, 0x05, 0x58, 0x68, 0xe0, 0xaa, 0xd2, 0x8e,
        };
        const uint8_t kGoodCsrSubjectPublicKey[] = {
            0x04, 0xa3, 0xbe, 0xa1, 0xf5, 0x42, 0x01, 0x07, 0x3c, 0x4b, 0x75, 0x85, 0xd8, 0xe2, 0x98, 0xac, 0x2f,
            0xf6, 0x98, 0xdb, 0xd9, 0x5b, 0xe0, 0x7e, 0xc1, 0x04, 0xd5, 0x73, 0xc5, 0xb0, 0x90, 0x77, 0x27, 0x00,
            0x1e, 0x22, 0xc7, 0x89, 0x5e, 0x4d, 0x75, 0x07, 0x89, 0x82, 0x0f, 0x49, 0xb6, 0x59, 0xd5, 0xc5, 0x15,
            0x7d, 0x93, 0xe6, 0x80, 0x5c, 0x70, 0x89, 0x0a, 0x43, 0x10, 0x3d, 0xeb, 0x3d, 0x4a,
        };

        Crypto::ClearSecretData(pubKey.Bytes(), pubKey.Length());

        err = VerifyCertificateSigningRequestFormat(&kGoodCsr[0], sizeof(kGoodCsr));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = VerifyCertificateSigningRequest(&kGoodCsr[0], sizeof(kGoodCsr), pubKey);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        Crypto::P256PublicKey expected(kGoodCsrSubjectPublicKey);
        EXPECT_TRUE(pubKey.Matches(expected));
    }

    // Third case: bad signature
    {
        const uint8_t kBadSignatureSignatureCsr[] = {
            0x30, 0x81, 0xca, 0x30, 0x70, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0a,
            0x0c, 0x03, 0x43, 0x53, 0x52, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06,
            0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xa3, 0xbe, 0xa1, 0xf5, 0x42, 0x01,
            0x07, 0x3c, 0x4b, 0x75, 0x85, 0xd8, 0xe2, 0x98, 0xac, 0x2f, 0xf6, 0x98, 0xdb, 0xd9, 0x5b, 0xe0, 0x7e, 0xc1, 0x04,
            0xd5, 0x73, 0xc5, 0xb0, 0x90, 0x77, 0x27, 0x00, 0x1e, 0x22, 0xc7, 0x89, 0x5e, 0x4d, 0x75, 0x07, 0x89, 0x82, 0x0f,
            0x49, 0xb6, 0x59, 0xd5, 0xc5, 0x15, 0x7d, 0x93, 0xe6, 0x80, 0x5c, 0x70, 0x89, 0x0a, 0x43, 0x10, 0x3d, 0xeb, 0x3d,
            0x4a, 0xa0, 0x00, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x05, 0x00, 0x03, 0x48,
            0x00, 0x30, 0x45, 0x02, 0x20, 0x1d, 0x86, 0x21, 0xb4, 0xc2, 0xe1, 0xa9, 0xf3, 0xbc, 0xc8, 0x7c, 0xda, 0xb4, 0xb9,
            0xc6, 0x8c, 0xd0, 0xe4, 0x9a, 0x9c, 0xef, 0x02, 0x93, 0x98, 0x27, 0x7e, 0x81, 0x21, 0x5d, 0x20, 0x9d, 0x32, 0x02,
            0x21, 0x00, 0x8b, 0x6b, 0x49, 0xb6, 0x7d, 0x3e, 0x67, 0x9e, 0xb1, 0x21, 0xd3, 0x63, 0x82, 0x40, 0x4f, 0x49, 0xa4,
            0xdc, 0x17, 0x35, 0xac, 0x4b, 0x7a, 0xbf, 0x52, 0x05, 0x58, 0x68, 0xe0, 0xaa, 0xd2, 0x8e,
        };

        Crypto::ClearSecretData(pubKey.Bytes(), pubKey.Length());

        err = VerifyCertificateSigningRequestFormat(&kBadSignatureSignatureCsr[0], sizeof(kBadSignatureSignatureCsr));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = VerifyCertificateSigningRequest(&kBadSignatureSignatureCsr[0], sizeof(kBadSignatureSignatureCsr), pubKey);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }

    // Fourth case: CSR too big
    {
        const uint8_t kBadTooBigCsr[] = {
            0x30, 0x81, 0xda, 0x30, 0x81, 0x81, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0b,
            0x0c, 0x03, 0x43, 0x53, 0x41, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08,
            0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x72, 0x48, 0xc0, 0x36, 0xf0, 0x12, 0x5f, 0xd1,
            0x68, 0x92, 0x2d, 0xee, 0x57, 0x2b, 0x8e, 0x20, 0x9d, 0x97, 0xfa, 0x73, 0x92, 0xf1, 0xa0, 0x91, 0x0e, 0xfd, 0x04, 0x93,
            0x66, 0x47, 0x3c, 0xa3, 0xf0, 0xa8, 0x47, 0xa1, 0xa3, 0x1e, 0x13, 0x3b, 0x67, 0x3b, 0x18, 0xca, 0x77, 0xd1, 0xea, 0xe3,
            0x74, 0x93, 0x49, 0x8b, 0x9d, 0xdc, 0xef, 0xf9, 0xd5, 0x9b, 0x27, 0x19, 0xad, 0x6e, 0x90, 0xd2, 0xa0, 0x11, 0x30, 0x0f,
            0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x0e, 0x31, 0x02, 0x30, 0x00, 0x30, 0x0a, 0x06, 0x08, 0x2a,
            0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x6a, 0x2e, 0x15, 0x34, 0x1b, 0xde,
            0xcb, 0x8f, 0xd2, 0xfd, 0x35, 0x03, 0x89, 0x0e, 0xed, 0x23, 0x54, 0xff, 0xcb, 0x79, 0xf9, 0xcb, 0x40, 0x33, 0x59, 0xb4,
            0x27, 0x69, 0xeb, 0x07, 0x3b, 0xd5, 0x02, 0x21, 0x00, 0xb0, 0x25, 0xc9, 0xc2, 0x21, 0xe8, 0x54, 0xcc, 0x08, 0x12, 0xf5,
            0x10, 0x3a, 0x0b, 0x25, 0x20, 0x0a, 0x61, 0x38, 0xc8, 0x6f, 0x82, 0xa7, 0x51, 0x84, 0x61, 0xae, 0x93, 0x69, 0xe4, 0x74,
            0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        };

        Crypto::ClearSecretData(pubKey.Bytes(), pubKey.Length());
        err = VerifyCertificateSigningRequestFormat(&kBadTooBigCsr[0], sizeof(kBadTooBigCsr));
        EXPECT_EQ(err, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        err = VerifyCertificateSigningRequest(&kBadTooBigCsr[0], sizeof(kBadTooBigCsr), pubKey);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }

    // Fifth case: obviously invalid CSR (1/2)
    {
        const uint8_t kTooSmallCsr[] = {
            0x30, 0x81, 0xda, 0x30, 0x81, 0x81, 0x02, 0x01, 0x00, 0x30,
        };

        Crypto::ClearSecretData(pubKey.Bytes(), pubKey.Length());

        err = VerifyCertificateSigningRequestFormat(&kTooSmallCsr[0], sizeof(kTooSmallCsr));
        EXPECT_EQ(err, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        err = VerifyCertificateSigningRequest(&kTooSmallCsr[0], sizeof(kTooSmallCsr), pubKey);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }

    // Sixth case: obviously invalid CSR (2/2)
    {
        const uint8_t kNotSequenceCsr[] = {
            0x31, 0x81, 0xca, 0x30, 0x70, 0x02, 0x01, 0x00, 0x30, 0x0e, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x0a,
            0x0c, 0x03, 0x43, 0x53, 0x52, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06,
            0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xa3, 0xbe, 0xa1, 0xf5, 0x42, 0x01,
            0x07, 0x3c, 0x4b, 0x75, 0x85, 0xd8, 0xe2, 0x98, 0xac, 0x2f, 0xf6, 0x98, 0xdb, 0xd9, 0x5b, 0xe0, 0x7e, 0xc1, 0x04,
            0xd5, 0x73, 0xc5, 0xb0, 0x90, 0x77, 0x27, 0x00, 0x1e, 0x22, 0xc7, 0x89, 0x5e, 0x4d, 0x75, 0x07, 0x89, 0x82, 0x0f,
            0x49, 0xb6, 0x59, 0xd5, 0xc5, 0x15, 0x7d, 0x93, 0xe6, 0x80, 0x5c, 0x70, 0x89, 0x0a, 0x43, 0x10, 0x3d, 0xeb, 0x3d,
            0x4a, 0xa0, 0x00, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x05, 0x00, 0x03, 0x48,
            0x00, 0x30, 0x45, 0x02, 0x20, 0x1d, 0x86, 0x21, 0xb4, 0xc2, 0xe1, 0xa9, 0xf3, 0xbc, 0xc8, 0x7c, 0xda, 0xb4, 0xb9,
            0xc6, 0x8c, 0xd0, 0xe4, 0x9a, 0x9c, 0xef, 0x02, 0x93, 0x98, 0x27, 0x7e, 0x81, 0x21, 0x5d, 0x20, 0x9d, 0x32, 0x02,
            0x21, 0x00, 0x8b, 0x6b, 0x49, 0xb6, 0x7d, 0x3e, 0x67, 0x9e, 0xb1, 0x22, 0xd3, 0x63, 0x82, 0x40, 0x4f, 0x49, 0xa4,
            0xdc, 0x17, 0x35, 0xac, 0x4b, 0x7a, 0xbf, 0x52, 0x05, 0x58, 0x68, 0xe0, 0xaa, 0xd2, 0x8e,
        };

        Crypto::ClearSecretData(pubKey.Bytes(), pubKey.Length());

        err = VerifyCertificateSigningRequestFormat(&kNotSequenceCsr[0], sizeof(kNotSequenceCsr));
        EXPECT_EQ(err, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

        err = VerifyCertificateSigningRequest(&kNotSequenceCsr[0], sizeof(kNotSequenceCsr), pubKey);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }
}

TEST_F(TestChipCryptoPAL, TestCSR_GenDirect)
{
    uint8_t csrBuf[kMIN_CSR_Buffer_Size];
    ClearSecretData(csrBuf);
    MutableByteSpan csrSpan(csrBuf);

    Test_P256Keypair keypair;

    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    // Validate case of buffer too small
    uint8_t csrBufTooSmall[kMIN_CSR_Buffer_Size - 1];
    MutableByteSpan csrSpanTooSmall(csrBufTooSmall);
    EXPECT_EQ(GenerateCertificateSigningRequest(&keypair, csrSpanTooSmall), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Validate case of null keypair
    EXPECT_EQ(GenerateCertificateSigningRequest(nullptr, csrSpan), CHIP_ERROR_INVALID_ARGUMENT);

    // Validate normal case
    ClearSecretData(csrBuf);
    EXPECT_EQ(GenerateCertificateSigningRequest(&keypair, csrSpan), CHIP_NO_ERROR);

    P256PublicKey pubkey;

    CHIP_ERROR err = VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), pubkey);
    if (err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(pubkey.Length(), kP256_PublicKey_Length);
        EXPECT_EQ(memcmp(pubkey.ConstBytes(), keypair.Pubkey().ConstBytes(), pubkey.Length()), 0);

        // Let's corrupt the CSR buffer and make sure it fails to verify
        size_t length      = csrSpan.size();
        csrBuf[length - 2] = (uint8_t) (csrBuf[length - 2] + 1);
        csrBuf[length - 1] = (uint8_t) (csrBuf[length - 1] + 1);

        EXPECT_NE(VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), pubkey), CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(Crypto, "The current platform does not support CSR parsing.");
    }
}

TEST_F(TestChipCryptoPAL, TestCSR_GenByKeypair)
{
    HeapChecker heapChecker;
    uint8_t csr[kMIN_CSR_Buffer_Size];
    size_t length = sizeof(csr);

    Test_P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);
    EXPECT_EQ(keypair.NewCertificateSigningRequest(csr, length), CHIP_NO_ERROR);
    EXPECT_GT(length, 0u);

    P256PublicKey pubkey;
    CHIP_ERROR err = VerifyCertificateSigningRequest(csr, length, pubkey);
    if (err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(pubkey.Length(), kP256_PublicKey_Length);
        EXPECT_EQ(memcmp(pubkey.ConstBytes(), keypair.Pubkey().ConstBytes(), pubkey.Length()), 0);

        // Let's corrupt the CSR buffer and make sure it fails to verify
        csr[length - 2] = (uint8_t) (csr[length - 2] + 1);
        csr[length - 1] = (uint8_t) (csr[length - 1] + 1);

        EXPECT_NE(VerifyCertificateSigningRequest(csr, length, pubkey), CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(Crypto, "The current platform does not support CSR parsing.");
    }
}

TEST_F(TestChipCryptoPAL, TestKeypair_Serialize)
{
    HeapChecker heapChecker;
    Test_P256Keypair keypair;

    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    P256SerializedKeypair serialized;
    EXPECT_EQ(keypair.Serialize(serialized), CHIP_NO_ERROR);

    Test_P256Keypair keypair_dup;
    EXPECT_EQ(keypair_dup.Deserialize(serialized), CHIP_NO_ERROR);

    const char * msg         = "Test Message for Keygen";
    const uint8_t * test_msg = Uint8::from_const_char(msg);
    size_t msglen            = strlen(msg);

    P256ECDSASignature test_sig;
    EXPECT_EQ(keypair.ECDSA_sign_msg(test_msg, msglen, test_sig), CHIP_NO_ERROR);
    EXPECT_EQ(keypair_dup.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig), CHIP_NO_ERROR);

    EXPECT_EQ(keypair_dup.ECDSA_sign_msg(test_msg, msglen, test_sig), CHIP_NO_ERROR);
    EXPECT_EQ(keypair.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig), CHIP_NO_ERROR);
}

TEST_F(TestChipCryptoPAL, TestSPAKE2P_spake2p_FEMul)
{
    HeapChecker heapChecker;
    uint8_t fe_out[kMAX_FE_Length];

    int numOfTestVectors = ArraySize(fe_mul_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_fe_mul_tv * vector = fe_mul_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe1, vector->fe1_len, spake2p.w0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe2, vector->fe2_len, spake2p.w1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FEMul(spake2p.xy, spake2p.w0, spake2p.w1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FEWrite(spake2p.xy, fe_out, sizeof(fe_out));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(memcmp(fe_out, vector->fe_out, vector->fe_out_len), 0);
        numOfTestsRan += 1;
    }
    EXPECT_GT(numOfTestsRan, 0);
    EXPECT_EQ(numOfTestsRan, numOfTestVectors);
}

TEST_F(TestChipCryptoPAL, TestSPAKE2P_spake2p_FELoadWrite)
{
    HeapChecker heapChecker;
    uint8_t fe_out[kMAX_FE_Length];

    int numOfTestVectors = ArraySize(fe_rw_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_fe_rw_tv * vector = fe_rw_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe_in, vector->fe_in_len, spake2p.w0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FEWrite(spake2p.w0, fe_out, sizeof(fe_out));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(memcmp(fe_out, vector->fe_out, vector->fe_out_len), 0);
        numOfTestsRan += 1;
    }
    EXPECT_GT(numOfTestsRan, 0);
    EXPECT_EQ(numOfTestsRan, numOfTestVectors);
}

TEST_F(TestChipCryptoPAL, TestSPAKE2P_spake2p_Mac)
{
    HeapChecker heapChecker;
    uint8_t mac[kMAX_Hash_Length];
    MutableByteSpan mac_span{ mac };

    int numOfTestVectors = ArraySize(hmac_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_hmac_tv * vector = hmac_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.Mac(vector->key, vector->key_len, vector->input, vector->input_len, mac_span);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(memcmp(mac_span.data(), vector->output, vector->output_len), 0);

        err = spake2p.MacVerify(vector->key, vector->key_len, vector->output, vector->output_len, vector->input, vector->input_len);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        numOfTestsRan += 1;
    }
    EXPECT_GT(numOfTestsRan, 0);
    EXPECT_EQ(numOfTestsRan, numOfTestVectors);
}

TEST_F(TestChipCryptoPAL, TestSPAKE2P_spake2p_PointMul)
{
    HeapChecker heapChecker;
    uint8_t output[kMAX_Point_Length];
    size_t out_len = sizeof(output);

    int numOfTestVectors = ArraySize(point_mul_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        out_len                                    = sizeof(output);
        const struct spake2p_point_mul_tv * vector = point_mul_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar, vector->scalar_len, spake2p.w0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointMul(spake2p.X, spake2p.L, spake2p.w0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.X, output, out_len);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(memcmp(output, vector->out_point, vector->out_point_len), 0);

        numOfTestsRan += 1;
    }
    EXPECT_GT(numOfTestsRan, 0);
    EXPECT_EQ(numOfTestsRan, numOfTestVectors);
}

TEST_F(TestChipCryptoPAL, TestSPAKE2P_spake2p_PointMulAdd)
{
    HeapChecker heapChecker;
    uint8_t output[kMAX_Point_Length];
    size_t out_len = sizeof(output);

    int numOfTestVectors = ArraySize(point_muladd_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        out_len                                       = sizeof(output);
        const struct spake2p_point_muladd_tv * vector = point_muladd_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point1, vector->point1_len, spake2p.X);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point2, vector->point2_len, spake2p.Y);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar1, vector->scalar1_len, spake2p.w0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar2, vector->scalar2_len, spake2p.w1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointAddMul(spake2p.L, spake2p.X, spake2p.w0, spake2p.Y, spake2p.w1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.L, output, out_len);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(memcmp(output, vector->out_point, vector->out_point_len), 0);

        numOfTestsRan += 1;
    }
    EXPECT_GT(numOfTestsRan, 0);
    EXPECT_EQ(numOfTestsRan, numOfTestVectors);
}

TEST_F(TestChipCryptoPAL, TestSPAKE2P_spake2p_PointLoadWrite)
{
    HeapChecker heapChecker;
    uint8_t output[kMAX_Point_Length];
    size_t out_len = sizeof(output);

    int numOfTestVectors = ArraySize(point_rw_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        out_len                                   = sizeof(output);
        const struct spake2p_point_rw_tv * vector = point_rw_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.L, output, out_len);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(memcmp(output, vector->point, vector->point_len), 0);

        numOfTestsRan += 1;
    }
    EXPECT_GT(numOfTestsRan, 0);
    EXPECT_EQ(numOfTestsRan, numOfTestVectors);
}

TEST_F(TestChipCryptoPAL, TestSPAKE2P_spake2p_PointIsValid)
{
    HeapChecker heapChecker;
    int numOfTestVectors = ArraySize(point_valid_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_point_valid_tv * vector = point_valid_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        // The underlying implementation may (i.e. should) check for validity when loading a point. Let's catch this case.
        EXPECT_TRUE(err == CHIP_NO_ERROR || vector->valid == 0);

        err = spake2p.PointIsValid(spake2p.L);
        EXPECT_TRUE(err == CHIP_NO_ERROR || vector->valid == 0);

        numOfTestsRan += 1;
    }
    EXPECT_GT(numOfTestsRan, 0);
    EXPECT_EQ(numOfTestsRan, numOfTestVectors);
}

// We need to "generate" specific field elements
// to do so we need to override the specific method
class Test_Spake2p_P256_SHA256_HKDF_HMAC : public Spake2p_P256_SHA256_HKDF_HMAC
{
public:
    CHIP_ERROR TestSetFE(const uint8_t * fe_in, size_t fe_in_len)
    {
        if (fe_in_len > kMAX_FE_Length)
        {
            return CHIP_ERROR_INTERNAL;
        }
        memcpy(fe, fe_in, fe_in_len);
        fe_len = fe_in_len;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FEGenerate(void * feout) override { return FELoad(fe, fe_len, feout); }

private:
    uint8_t fe[kMAX_FE_Length];
    size_t fe_len;
};

TEST_F(TestChipCryptoPAL, TestSPAKE2P_RFC)
{
    HeapChecker heapChecker;
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint8_t L[kMAX_Point_Length];
    size_t L_len = sizeof(L);
    uint8_t Z[kMAX_Point_Length];
    uint8_t V[kMAX_Point_Length];
    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);
    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);
    uint8_t Pverifier[kMAX_Hash_Length];
    size_t Pverifier_len = sizeof(Pverifier);
    uint8_t Vverifier[kMAX_Hash_Length];
    size_t Vverifier_len = sizeof(Vverifier);

    int numOfTestVectors = ArraySize(rfc_tvs);
    int numOfTestsRan    = 0;
    // static_assert(sizeof(Spake2p_Context) < 1024, "Allocate more bytes for Spake2p Context");
    // printf("Sizeof spake2pcontext %lu\n", sizeof(Spake2p_Context));
    // printf("Sizeof mbedtls_sha256_context %lu\n", sizeof(mbedtls_sha256_context));
    // printf("Sizeof SHA256_CTX %lu\n", sizeof(SHA256_CTX));
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_rfc_tv * vector = rfc_tvs[vectorIndex];

        Test_Spake2p_P256_SHA256_HKDF_HMAC Verifier;
        Test_Spake2p_P256_SHA256_HKDF_HMAC Prover;

        // First start the prover
        error = Prover.Init(vector->context, vector->context_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        error = Prover.BeginProver(vector->prover_identity, vector->prover_identity_len, vector->verifier_identity,
                                   vector->verifier_identity_len, vector->w0, vector->w0_len, vector->w1, vector->w1_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // Monkey patch the generated x coordinate
        error = Prover.TestSetFE(vector->x, vector->x_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // Compute the first round and send it to the verifier
        X_len = sizeof(X);
        error = Prover.ComputeRoundOne(nullptr, 0, X, &X_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(X_len, vector->X_len);
        EXPECT_EQ(memcmp(X, vector->X, vector->X_len), 0);

        // Start up the verifier
        error = Verifier.Init(vector->context, vector->context_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // First pre-compute L (accessories with dynamic setup codes will do this)
        L_len = sizeof(L);
        error = Verifier.ComputeL(L, &L_len, vector->w1, vector->w1_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(L_len, vector->L_len);
        EXPECT_EQ(memcmp(L, vector->L, vector->L_len), 0);

        // Start up the verifier
        error = Verifier.BeginVerifier(vector->verifier_identity, vector->verifier_identity_len, vector->prover_identity,
                                       vector->prover_identity_len, vector->w0, vector->w0_len, L, L_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // Monkey patch the generated y coordinate
        error = Verifier.TestSetFE(vector->y, vector->y_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // Compute the first round and send it to the prover
        Y_len = sizeof(Y);
        error = Verifier.ComputeRoundOne(X, X_len, Y, &Y_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(Y_len, vector->Y_len);
        EXPECT_EQ(memcmp(Y, vector->Y, vector->Y_len), 0);

        // Compute the second round to also send to the prover
        Vverifier_len = sizeof(Vverifier);
        error         = Verifier.ComputeRoundTwo(X, X_len, Vverifier, &Vverifier_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(Vverifier_len, vector->MAC_KcB_len);
        EXPECT_EQ(memcmp(Vverifier, vector->MAC_KcB, vector->MAC_KcB_len), 0);

        error = Verifier.PointWrite(Verifier.Z, Z, kP256_Point_Length);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(Z, vector->Z, vector->Z_len), 0);

        error = Verifier.PointWrite(Verifier.V, V, kP256_Point_Length);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(V, vector->V, vector->V_len), 0);

        // Now the prover computes round 2
        Pverifier_len = sizeof(Pverifier);
        error         = Prover.ComputeRoundTwo(Y, Y_len, Pverifier, &Pverifier_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(Pverifier_len, vector->MAC_KcA_len);
        EXPECT_EQ(memcmp(Pverifier, vector->MAC_KcA, vector->MAC_KcA_len), 0);

        error = Prover.PointWrite(Verifier.Z, Z, kP256_Point_Length);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(Z, vector->Z, vector->Z_len), 0);

        error = Prover.PointWrite(Verifier.V, V, kP256_Point_Length);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        EXPECT_EQ(memcmp(V, vector->V, vector->V_len), 0);

        // Both sides now confirm the keys they received
        error = Prover.KeyConfirm(Vverifier, Vverifier_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        error = Verifier.KeyConfirm(Pverifier, Pverifier_len);
        EXPECT_EQ(error, CHIP_NO_ERROR);

#if !(CHIP_CRYPTO_KEYSTORE_APP)
        // Import HKDF key from the test vector to the keystore
        DefaultSessionKeystore keystore;
        HkdfKeyHandle vectorKe;
        error = keystore.CreateKey(ByteSpan(vector->Ke, vector->Ke_len), vectorKe);
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // Verify that both sides generated the same HKDF key as in the test vector
        // Since the HKDF keys may not be availabe in the raw form, do not compare them directly,
        // but rather check if the same attestation challenge is derived from
        HkdfKeyHandle PKe;
        error = Prover.GetKeys(keystore, PKe);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        AssertKeysEqual(keystore, PKe, vectorKe);

        HkdfKeyHandle VKe;
        error = Verifier.GetKeys(keystore, VKe);
        EXPECT_EQ(error, CHIP_NO_ERROR);
        AssertKeysEqual(keystore, VKe, vectorKe);

        keystore.DestroyKey(vectorKe);
        keystore.DestroyKey(PKe);
        keystore.DestroyKey(VKe);
#endif
        numOfTestsRan += 1;
    }
    EXPECT_GT(numOfTestsRan, 0);
    EXPECT_EQ(numOfTestsRan, numOfTestVectors);
}

TEST_F(TestChipCryptoPAL, TestSPAKE2P_Reuse)
{
    HeapChecker heapChecker;
    Test_Spake2p_P256_SHA256_HKDF_HMAC spake2;

    // Veriy Init -> Clear -> Init sequence works and does not leak memory
    EXPECT_EQ(spake2.Init(nullptr, 0), CHIP_NO_ERROR);
    spake2.Clear();
    EXPECT_EQ(spake2.Init(nullptr, 0), CHIP_NO_ERROR);

    // Even without an explicit Clear, Init does not leak memory
    EXPECT_EQ(spake2.Init(nullptr, 0), CHIP_NO_ERROR);
}

TEST_F(TestChipCryptoPAL, TestCompressedFabricIdentifier)
{
    HeapChecker heapChecker;
    // Data from spec test vector (see Operational Discovery section)
    const uint8_t kRootPublicKey[] = {
        0x04, 0x4a, 0x9f, 0x42, 0xb1, 0xca, 0x48, 0x40, 0xd3, 0x72, 0x92, 0xbb, 0xc7, 0xf6, 0xa7, 0xe1, 0x1e,
        0x22, 0x20, 0x0c, 0x97, 0x6f, 0xc9, 0x00, 0xdb, 0xc9, 0x8a, 0x7a, 0x38, 0x3a, 0x64, 0x1c, 0xb8, 0x25,
        0x4a, 0x2e, 0x56, 0xd4, 0xe2, 0x95, 0xa8, 0x47, 0x94, 0x3b, 0x4e, 0x38, 0x97, 0xc4, 0xa7, 0x73, 0xe9,
        0x30, 0x27, 0x7b, 0x4d, 0x9f, 0xbe, 0xde, 0x8a, 0x05, 0x26, 0x86, 0xbf, 0xac, 0xfa,
    };
    P256PublicKey root_public_key(kRootPublicKey);

    constexpr uint64_t kFabricId = 0x2906C908D115D362;

    const uint8_t kExpectedCompressedFabricIdentifier[] = {
        0x87, 0xe1, 0xb0, 0x04, 0xe2, 0x35, 0xa1, 0x30,
    };
    static_assert(sizeof(kExpectedCompressedFabricIdentifier) == kCompressedFabricIdentifierSize,
                  "Expected compressed fabric identifier must the correct size");
    const uint64_t kExpectedCompressedFabricIdentifierInt = 0x87e1b004e235a130;

    uint8_t compressed_fabric_id[kCompressedFabricIdentifierSize];
    MutableByteSpan compressed_fabric_id_span(compressed_fabric_id);
    ClearSecretData(compressed_fabric_id, sizeof(compressed_fabric_id));
    uint64_t compressed_fabric_id_int;

    CHIP_ERROR error = GenerateCompressedFabricId(root_public_key, kFabricId, compressed_fabric_id_span);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(compressed_fabric_id_span.size(), kCompressedFabricIdentifierSize);
    EXPECT_EQ(
        0,
        memcmp(compressed_fabric_id_span.data(), kExpectedCompressedFabricIdentifier, sizeof(kExpectedCompressedFabricIdentifier)));

    // Test bigger input buffer than needed
    uint8_t compressed_fabric_id_large[3 * kCompressedFabricIdentifierSize];
    MutableByteSpan compressed_fabric_id_large_span(compressed_fabric_id_large);
    ClearSecretData(compressed_fabric_id_large, sizeof(compressed_fabric_id_large));

    error = GenerateCompressedFabricId(root_public_key, kFabricId, compressed_fabric_id_large_span);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(compressed_fabric_id_large_span.size(), kCompressedFabricIdentifierSize);
    EXPECT_EQ(0,
              memcmp(compressed_fabric_id_large_span.data(), kExpectedCompressedFabricIdentifier,
                     sizeof(kExpectedCompressedFabricIdentifier)));

    // Test smaller buffer than needed
    MutableByteSpan compressed_fabric_id_small_span(compressed_fabric_id, kCompressedFabricIdentifierSize - 1);
    error = GenerateCompressedFabricId(root_public_key, kFabricId, compressed_fabric_id_small_span);
    EXPECT_EQ(error, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test overload that writes to an integer output type.
    error = GenerateCompressedFabricId(root_public_key, kFabricId, compressed_fabric_id_int);
    EXPECT_EQ(error, CHIP_NO_ERROR);
    EXPECT_EQ(compressed_fabric_id_int, kExpectedCompressedFabricIdentifierInt);

    // Test invalid public key
    const uint8_t kInvalidRootPublicKey[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    P256PublicKey invalid_root_public_key(kInvalidRootPublicKey);

    error = GenerateCompressedFabricId(invalid_root_public_key, kFabricId, compressed_fabric_id_span);
    EXPECT_EQ(error, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestChipCryptoPAL, TestPubkey_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;
    P256PublicKey publicKey;

    ByteSpan cert;
    ByteSpan pubkeySpan;

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        TestCert certType = TestCerts::gTestCerts[i];

        err = GetTestCert(certType, TestCertLoadFlags::kDERForm, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = GetTestCertPubkey(certType, pubkeySpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = ExtractPubkeyFromX509Cert(cert, publicKey);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(publicKey.Length(), pubkeySpan.size());
        EXPECT_EQ(memcmp(publicKey.ConstBytes(), pubkeySpan.data(), pubkeySpan.size()), 0);
    }
}

TEST_F(TestChipCryptoPAL, TestX509_VerifyAttestationCertificateFormat)
{
    using namespace TestCerts;

    HeapChecker heapChecker;

    struct ValidationTestCase
    {
        ByteSpan cert;
        AttestationCertType type;
        CHIP_ERROR expectedError;
    };

    // clang-format off
    static ValidationTestCase sValidationTestCases[] = {
        // cert                                           type                               Expected Error
        // ===============================================================================================================
        {  sTestCert_PAA_FFF1_Cert,                       Crypto::AttestationCertType::kPAA, CHIP_NO_ERROR               },
        {  sTestCert_PAI_FFF1_8000_Cert,                  Crypto::AttestationCertType::kPAI, CHIP_NO_ERROR               },
        {  sTestCert_DAC_FFF2_8002_0017_Cert,             Crypto::AttestationCertType::kDAC, CHIP_NO_ERROR               },
        {  ByteSpan(),                                    Crypto::AttestationCertType::kDAC, CHIP_ERROR_INVALID_ARGUMENT },
        {  sTestCert_PAI_FFF2_NoPID_FB_Cert,              Crypto::AttestationCertType::kDAC, CHIP_ERROR_INTERNAL         },
        {  sTestCert_DAC_FFF2_8006_0025_ValInFuture_Cert, Crypto::AttestationCertType::kPAA, CHIP_ERROR_INTERNAL         },
        {  ByteSpan{kPaaWithNoPathlen},                   Crypto::AttestationCertType::kPAA, CHIP_NO_ERROR               },
        {  ByteSpan{kPaiPathLenMissing},                  Crypto::AttestationCertType::kPAI, CHIP_ERROR_INTERNAL         },
        {  ByteSpan{kPaiPathLen1},                        Crypto::AttestationCertType::kPAI, CHIP_ERROR_INTERNAL         },
        {  ByteSpan{kPaaPathLen2},                        Crypto::AttestationCertType::kPAA, CHIP_ERROR_INTERNAL         },
        {  ByteSpan{kWrongPathLenFormat},                 Crypto::AttestationCertType::kPAI, CHIP_ERROR_INTERNAL         },
    };
    // clang-format on

    [[maybe_unused]] int case_idx = 0;
    for (auto & testCase : sValidationTestCases)
    {
        ByteSpan cert  = testCase.cert;
        CHIP_ERROR err = VerifyAttestationCertificateFormat(cert, testCase.type);
        if (err != testCase.expectedError)
        {
            ChipLogError(Crypto, "Failed TestX509_VerifyAttestationCertificateFormat sub-case %d, err: %" CHIP_ERROR_FORMAT,
                         case_idx, err.Format());
        }
        EXPECT_EQ(err, testCase.expectedError);
        ++case_idx;
    }
}

TEST_F(TestChipCryptoPAL, TestX509_CertChainValidation)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;

    struct ValidationTestCase
    {
        ByteSpan root;
        ByteSpan ica;
        ByteSpan leaf;
        CHIP_ERROR expectedError;
        CertificateChainValidationResult expectedValResult;
    };

    // clang-format off
    static ValidationTestCase sValidationTestCases[] = {
        // root                                 ica                                        leaf                                             Expected Error              Expected Validation Result
        // ======================================================================================================================================================================================================================
        {  sTestCert_PAA_FFF1_Cert,             sTestCert_PAI_FFF1_8000_Cert,              sTestCert_DAC_FFF1_8000_0000_Cert,               CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_FFF1_Cert,             sTestCert_PAI_FFF1_8000_Cert,              sTestCert_DAC_FFF1_8000_0000_Cert,               CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_NoPID_Cert,             sTestCert_DAC_FFF2_8002_0017_Cert,               CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_NoPID_FB_Cert,          sTestCert_DAC_FFF2_8003_0018_FB_Cert,            CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_8004_FB_Cert,           sTestCert_DAC_FFF2_8004_001C_FB_Cert,            CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        // Valid cases with PAA, PAI, DAC time validity in the past or future:
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_8004_FB_Cert,           sTestCert_DAC_FFF2_8004_0020_ValInPast_Cert,     CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_8004_FB_Cert,           sTestCert_DAC_FFF2_8004_0021_ValInFuture_Cert,   CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_8005_ValInPast_Cert,    sTestCert_DAC_FFF2_8005_0022_ValInPast_Cert,     CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_8005_ValInFuture_Cert,  sTestCert_DAC_FFF2_8005_0023_ValInFuture_Cert,   CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_FFF2_ValInPast_Cert,   sTestCert_PAI_FFF2_8006_ValInPast_Cert,    sTestCert_DAC_FFF2_8006_0024_ValInPast_Cert,     CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        {  sTestCert_PAA_FFF2_ValInFuture_Cert, sTestCert_PAI_FFF2_8006_ValInFuture_Cert,  sTestCert_DAC_FFF2_8006_0025_ValInFuture_Cert,   CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        // Valid cases without intermediate:
        {  sTestCert_Root01_DER,                ByteSpan(),                                sTestCert_Node01_02_DER,                         CHIP_NO_ERROR,               CertificateChainValidationResult::kSuccess             },
        // Error cases with invalid (empty Span) inputs:
        {  ByteSpan(),                          sTestCert_PAI_FFF1_8000_Cert,              sTestCert_DAC_FFF1_8000_0000_Cert,               CHIP_ERROR_INVALID_ARGUMENT, CertificateChainValidationResult::kRootArgumentInvalid },
        {  sTestCert_PAA_FFF1_Cert,             sTestCert_PAI_FFF1_8000_Cert,              ByteSpan(),                                      CHIP_ERROR_INVALID_ARGUMENT, CertificateChainValidationResult::kLeafArgumentInvalid },
        // Error case with empty intermediate but the leaf doesn't chain up to the root in this case:
        {  sTestCert_PAA_FFF1_Cert,             ByteSpan(),                                sTestCert_DAC_FFF1_8000_0000_Cert,               CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
        // Error cases with wrong certificate chaining:
        {  sTestCert_PAA_FFF1_Cert,             sTestCert_PAI_FFF2_NoPID_Cert,             sTestCert_DAC_FFF1_8000_0000_Cert,               CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF1_8000_Cert,              sTestCert_DAC_FFF1_8000_0000_Cert,               CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAA_FFF1_Cert,                   sTestCert_DAC_FFF1_8000_0000_Cert,               CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
        // Error cases with PAA, PAI, DAC time validity in the past or future.
        // In all cases either PAA or PAI was invalid with respect to DAC's notBefore time:
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_8004_FB_Cert,           sTestCert_DAC_FFF2_8004_0030_Val1SecBefore_Cert, CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_8005_Val1SecBefore_Cert,sTestCert_DAC_FFF2_8005_0032_Val1SecBefore_Cert, CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
        {  sTestCert_PAA_NoVID_Cert,            sTestCert_PAI_FFF2_8005_ValInFuture_Cert,  sTestCert_DAC_FFF2_8005_0033_Val1SecBefore_Cert, CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
        {  sTestCert_PAA_FFF2_ValInPast_Cert,   sTestCert_PAI_FFF2_8006_ValInPast_Cert,    sTestCert_DAC_FFF2_8006_0034_ValInFuture_Cert,   CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
        {  sTestCert_PAA_FFF2_ValInFuture_Cert, sTestCert_PAI_FFF2_8006_ValInFuture_Cert,  sTestCert_DAC_FFF2_8006_0035_Val1SecBefore_Cert, CHIP_ERROR_CERT_NOT_TRUSTED, CertificateChainValidationResult::kChainInvalid        },
    };
    // clang-format on

    for (auto & testCase : sValidationTestCases)
    {
        CertificateChainValidationResult chainValidationResult;
        err = ValidateCertificateChain(testCase.root.data(), testCase.root.size(), testCase.ica.data(), testCase.ica.size(),
                                       testCase.leaf.data(), testCase.leaf.size(), chainValidationResult);
        EXPECT_EQ(err, testCase.expectedError);
        EXPECT_EQ(chainValidationResult, testCase.expectedValResult);
    }
}

TEST_F(TestChipCryptoPAL, TestX509_IssuingTimestampValidation)
{
    using namespace TestCerts;
    using namespace ASN1;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;

    struct ValidationTestCase
    {
        ByteSpan refCert;
        ByteSpan evaluatedCert;
        CHIP_ERROR expectedError;
    };

    // clang-format off
    static ValidationTestCase sValidationTestCases[] = {
        // Reference Certificate                            Evaluated Certificate                          Expected Error
        // ================================================================================================================================
        {  sTestCert_DAC_FFF1_8000_0000_Cert,               sTestCert_PAA_FFF1_Cert,                       CHIP_NO_ERROR                 },
        {  sTestCert_DAC_FFF1_8000_0000_Cert,               sTestCert_PAI_FFF1_8000_Cert,                  CHIP_NO_ERROR                 },
        {  sTestCert_DAC_FFF2_8004_0020_ValInPast_Cert,     sTestCert_PAA_NoVID_Cert,                      CHIP_NO_ERROR                 },
        {  sTestCert_DAC_FFF2_8004_0021_ValInFuture_Cert,   sTestCert_PAI_FFF2_8004_FB_Cert,               CHIP_NO_ERROR                 },
        {  sTestCert_DAC_FFF2_8005_0023_ValInFuture_Cert,   sTestCert_PAI_FFF2_8005_ValInFuture_Cert,      CHIP_NO_ERROR                 },
        {  sTestCert_DAC_FFF2_8006_0025_ValInFuture_Cert,   sTestCert_PAA_FFF2_ValInFuture_Cert,           CHIP_NO_ERROR                 },
        {  sTestCert_DAC_FFF2_8005_0032_Val1SecBefore_Cert, sTestCert_PAI_FFF2_8005_Val1SecBefore_Cert,    CHIP_NO_ERROR                 },
        // Error cases with invalid (empty Span) inputs:
        {  sTestCert_DAC_FFF1_8000_0000_Cert,               ByteSpan(),                                    CHIP_ERROR_INVALID_ARGUMENT   },
        {  ByteSpan(),                                      sTestCert_PAA_FFF1_Cert,                       CHIP_ERROR_INVALID_ARGUMENT   },
        // Error cases with not yet valid certificate:
        {  sTestCert_DAC_FFF2_8004_0030_Val1SecBefore_Cert, sTestCert_PAI_FFF2_8004_FB_Cert,               CHIP_ERROR_CERT_EXPIRED       },
        {  sTestCert_DAC_FFF2_8004_0030_Val1SecBefore_Cert, sTestCert_PAI_FFF2_8004_FB_Cert,               CHIP_ERROR_CERT_EXPIRED       },
        {  sTestCert_DAC_FFF2_8005_0032_Val1SecBefore_Cert, sTestCert_PAA_NoVID_Cert,                      CHIP_ERROR_CERT_EXPIRED       },
        {  sTestCert_PAI_FFF2_8004_FB_Cert,                 sTestCert_DAC_FFF2_8004_0021_ValInFuture_Cert, CHIP_ERROR_CERT_EXPIRED       },
        {  sTestCert_DAC_FFF2_8006_0034_ValInFuture_Cert,   sTestCert_PAI_FFF2_8006_ValInPast_Cert,        CHIP_ERROR_CERT_EXPIRED       },
    };
    // clang-format on

    for (auto & testCase : sValidationTestCases)
    {
        err = IsCertificateValidAtIssuance(testCase.refCert, testCase.evaluatedCert);
        EXPECT_EQ(err, testCase.expectedError);
    }

#if !defined(CURRENT_TIME_NOT_IMPLEMENTED)
    // test certificate validity (this one contains validity until year 9999 so it will not fail soon)
    err = IsCertificateValidAtCurrentTime(sTestCert_DAC_FFF2_8001_0008_Cert);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#endif
}

TEST_F(TestChipCryptoPAL, TestSKID_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t skidBuf[kSubjectKeyIdentifierLength];
    MutableByteSpan skidOut(skidBuf);

    ByteSpan cert;
    ByteSpan skidSpan;

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        TestCert certType = gTestCerts[i];

        err = GetTestCert(certType, TestCertLoadFlags::kDERForm, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = GetTestCertSKID(certType, skidSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = ExtractSKIDFromX509Cert(cert, skidOut);
        if (!skidSpan.empty())
        {
            EXPECT_EQ(err, CHIP_NO_ERROR);
            EXPECT_TRUE(skidSpan.data_equal(skidOut));
        }
        else
        {
            EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);
        }
    }
}

TEST_F(TestChipCryptoPAL, TestAKID_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t akidBuf[kAuthorityKeyIdentifierLength];
    MutableByteSpan akidOut(akidBuf);

    ByteSpan cert;
    ByteSpan akidSpan;

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        TestCert certType = gTestCerts[i];

        err = GetTestCert(certType, TestCertLoadFlags::kDERForm, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = GetTestCertAKID(certType, akidSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = ExtractAKIDFromX509Cert(cert, akidOut);
        if (!akidSpan.empty())
        {
            EXPECT_EQ(err, CHIP_NO_ERROR);
            EXPECT_TRUE(akidSpan.data_equal(akidOut));
        }
        else
        {
            EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);
        }
    }
}

TEST_F(TestChipCryptoPAL, TestCDPExtension_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;

    struct CDPTestCase
    {
        ByteSpan Cert;
        CHIP_ERROR mExpectedError;
        CharSpan mExpectedResult;
    };

    constexpr const char * exampleHttpURI  = "http://example.com/crl.pem";
    constexpr const char * exampleHttpsURI = "https://example.com/crl.pem";
    CharSpan httpSpan                      = CharSpan::fromCharString(exampleHttpURI);
    CharSpan httpsSpan                     = CharSpan::fromCharString(exampleHttpsURI);

    // clang-format off
    static CDPTestCase sCDPTestCases[] = {
        // Cert                                                             Expected Error               Expected Output
        // ==============================================================================================================
        {  ByteSpan(),                                                      CHIP_ERROR_INVALID_ARGUMENT, CharSpan() },
        {  sTestCert_PAA_FFF1_Cert,                                         CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_PAI_FFF2_8001_Cert,                                    CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF2_8003_0019_FB_Cert,                            CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Cert,                           CHIP_NO_ERROR,               httpSpan   },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_HTTPS_Cert,                     CHIP_NO_ERROR,               httpsSpan  },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_2URIs_Cert,                     CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_2DPs_Cert,                      CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_2CDPs_Cert,                         CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Wrong_Prefix_Cert,              CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Long_Cert,                      CHIP_ERROR_BUFFER_TOO_SMALL, CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_2CDPs_Issuer_PAA_FFF1_Cert,         CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_2CDPs_Issuer_PAI_FFF2_8004_Cert,    CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_CRL_Issuer_PAA_FFF1_2DPs_Cert,  CHIP_ERROR_NOT_FOUND,        CharSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_2CRLIssuers_PAA_FFF1_Cert,      CHIP_NO_ERROR,               httpsSpan  },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAA_FFF1_Cert,           CHIP_NO_ERROR,               httpsSpan  },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAA_NoVID_Cert,          CHIP_NO_ERROR,               httpsSpan  },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAI_FFF2_8004_Cert,      CHIP_NO_ERROR,               httpsSpan  },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAI_FFF2_8004_Long_Cert, CHIP_ERROR_BUFFER_TOO_SMALL, CharSpan() },
    };
    // clang-format on

    for (auto & testCase : sCDPTestCases)
    {
        char cdpBuf[kMaxCRLDistributionPointURLLength] = { '\0' };
        MutableCharSpan cdp(cdpBuf);
        err = ExtractCRLDistributionPointURIFromX509Cert(testCase.Cert, cdp);
        EXPECT_EQ(err, testCase.mExpectedError);
        if (testCase.mExpectedError == CHIP_NO_ERROR)
        {
            EXPECT_EQ(cdp.size(), testCase.mExpectedResult.size());
            EXPECT_TRUE(cdp.data_equal(testCase.mExpectedResult));
        }
    }
}

TEST_F(TestChipCryptoPAL, TestCDPCRLIssuerExtension_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;

    struct CDPTestCase
    {
        ByteSpan Cert;
        CHIP_ERROR mExpectedError;
        ByteSpan mCRLIssuerCert;
    };

    // clang-format off
    static CDPTestCase sCDPTestCases[] = {
        // Cert                                                             Expected Error               Expected CRL Issuer Cert
        // =======================================================================================================================
        {  ByteSpan(),                                                      CHIP_ERROR_INVALID_ARGUMENT, ByteSpan() },
        {  sTestCert_PAA_FFF1_Cert,                                         CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_PAI_FFF2_8001_Cert,                                    CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF2_8003_0019_FB_Cert,                            CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Cert,                           CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_HTTPS_Cert,                     CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_2URIs_Cert,                     CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_2DPs_Cert,                      CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_2CDPs_Cert,                         CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Wrong_Prefix_Cert,              CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Long_Cert,                      CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_2CDPs_Issuer_PAA_FFF1_Cert,         CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_2CDPs_Issuer_PAI_FFF2_8004_Cert,    CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_CRL_Issuer_PAA_FFF1_2DPs_Cert,  CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_2CRLIssuers_PAA_FFF1_Cert,      CHIP_ERROR_NOT_FOUND,        ByteSpan() },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAA_FFF1_Cert,           CHIP_NO_ERROR,               sTestCert_PAA_FFF1_Cert },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAA_NoVID_Cert,          CHIP_NO_ERROR,               sTestCert_PAA_NoVID_Cert },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAI_FFF2_8004_Cert,      CHIP_NO_ERROR,               sTestCert_PAI_FFF2_8004_FB_Cert },
        {  sTestCert_DAC_FFF1_8000_0000_CDP_Issuer_PAI_FFF2_8004_Long_Cert, CHIP_NO_ERROR,               sTestCert_PAI_FFF2_8004_FB_Cert },
    };
    // clang-format on

    for (auto & testCase : sCDPTestCases)
    {
        uint8_t crlIssuerBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
        MutableByteSpan crlIssuer(crlIssuerBuf);
        err = ExtractCDPExtensionCRLIssuerFromX509Cert(testCase.Cert, crlIssuer);
        EXPECT_EQ(err, testCase.mExpectedError);
        if (testCase.mExpectedError == CHIP_NO_ERROR)
        {
            uint8_t crlIssuerSubjectBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
            MutableByteSpan crlIssuerSubject(crlIssuerSubjectBuf);

            err = ExtractSubjectFromX509Cert(testCase.mCRLIssuerCert, crlIssuerSubject);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            EXPECT_TRUE(crlIssuer.data_equal(crlIssuerSubject));
        }
    }
}

TEST_F(TestChipCryptoPAL, TestSerialNumber_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;

    struct SerialNumberTestCase
    {
        TestCert Cert;
        ByteSpan mExpectedResult;
    };

    const uint8_t serialNumberRoot01[]    = { 0x53, 0x4c, 0x45, 0x82, 0x73, 0x62, 0x35, 0x14 };
    const uint8_t serialNumberICA01[]     = { 0x69, 0xd8, 0x6a, 0x8d, 0x80, 0xfc, 0x8f, 0x5d };
    const uint8_t serialNumberNode02_08[] = { 0x3e, 0x67, 0x94, 0x70, 0x7a, 0xec, 0xb8, 0x15 };

    // clang-format off
    static SerialNumberTestCase sSerialNumberTestCases[] = {
        // Cert                    Expected Output
        // ====================================================
        {  TestCert::kRoot01,      ByteSpan(serialNumberRoot01) },
        {  TestCert::kICA01,       ByteSpan(serialNumberICA01) },
        {  TestCert::kNode02_08,   ByteSpan(serialNumberNode02_08) },
    };
    // clang-format on

    for (auto & testCase : sSerialNumberTestCases)
    {
        ByteSpan cert;
        err = GetTestCert(testCase.Cert, TestCertLoadFlags::kDERForm, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        uint8_t serialNumberBuf[kMaxCertificateSerialNumberLength] = { 0 };
        MutableByteSpan serialNumber(serialNumberBuf);
        err = ExtractSerialNumberFromX509Cert(cert, serialNumber);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(serialNumber.data_equal(testCase.mExpectedResult));
    }
}

TEST_F(TestChipCryptoPAL, TestSubject_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;

    struct TestCase
    {
        TestCert Cert;
        ChipDN mExpectedDN;
    };

    ChipDN subjectDN_Root01;
    EXPECT_EQ(subjectDN_Root01.AddAttribute_MatterRCACId(0xCACACACA00000001), CHIP_NO_ERROR);
    ChipDN subjectDN_ICA01;
    EXPECT_EQ(subjectDN_ICA01.AddAttribute_MatterICACId(0xCACACACA00000003), CHIP_NO_ERROR);
    ChipDN subjectDN_Node02_02;
    EXPECT_EQ(subjectDN_Node02_02.AddAttribute_MatterNodeId(0xDEDEDEDE00020002), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_02.AddAttribute_MatterFabricId(0xFAB000000000001D), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_02.AddAttribute_CommonName("TEST CERT COMMON NAME Attr for Node02_02"_span, false), CHIP_NO_ERROR);
    ChipDN subjectDN_Node02_04;
    EXPECT_EQ(subjectDN_Node02_04.AddAttribute_MatterCASEAuthTag(0xABCE1002), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_04.AddAttribute_CommonName("TestCert02_04"_span, false), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_04.AddAttribute_MatterFabricId(0xFAB000000000001D), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_04.AddAttribute_MatterCASEAuthTag(0xABCD0003), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_04.AddAttribute_MatterNodeId(0xDEDEDEDE00020004), CHIP_NO_ERROR);
    ChipDN subjectDN_Node02_08;
    EXPECT_EQ(subjectDN_Node02_08.AddAttribute_MatterCASEAuthTag(0xABCF00A0), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_08.AddAttribute_MatterNodeId(0xDEDEDEDE00020008), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_08.AddAttribute_MatterCASEAuthTag(0xABCD0020), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_08.AddAttribute_MatterFabricId(0xFAB000000000001D), CHIP_NO_ERROR);
    EXPECT_EQ(subjectDN_Node02_08.AddAttribute_MatterCASEAuthTag(0xABCE0100), CHIP_NO_ERROR);

    // clang-format off
    static TestCase sTestCases[] = {
        // Cert                    Expected Output
        // ==============================================
        {  TestCert::kRoot01,      subjectDN_Root01    },
        {  TestCert::kICA01,       subjectDN_ICA01     },
        {  TestCert::kNode02_02,   subjectDN_Node02_02 },
        {  TestCert::kNode02_04,   subjectDN_Node02_04 },
        {  TestCert::kNode02_08,   subjectDN_Node02_08 },
    };
    // clang-format on

    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        err = GetTestCert(testCase.Cert, TestCertLoadFlags::kDERForm, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        uint8_t subjectBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
        MutableByteSpan subject(subjectBuf);
        err = ExtractSubjectFromX509Cert(cert, subject);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        static uint8_t expectedSubjectBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
        ASN1::ASN1Writer writer;
        writer.Init(expectedSubjectBuf);
        err = testCase.mExpectedDN.EncodeToASN1(writer);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        size_t expectedSubjectLen = writer.GetLengthWritten();
        EXPECT_EQ(expectedSubjectLen, subject.size());
        EXPECT_EQ(memcmp(subject.data(), expectedSubjectBuf, expectedSubjectLen), 0);
    }
}

TEST_F(TestChipCryptoPAL, TestIssuer_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;
    CHIP_ERROR err = CHIP_NO_ERROR;

    struct TestCase
    {
        TestCert Cert;
        ChipDN mExpectedDN;
    };

    ChipDN issuerDN_Root01;
    EXPECT_EQ(CHIP_NO_ERROR, issuerDN_Root01.AddAttribute_MatterRCACId(0xCACACACA00000001));
    ChipDN issuerDN_ICA02;
    EXPECT_EQ(CHIP_NO_ERROR, issuerDN_ICA02.AddAttribute_MatterRCACId(0xCACACACA00000002));
    EXPECT_EQ(CHIP_NO_ERROR, issuerDN_ICA02.AddAttribute_MatterFabricId(0xFAB000000000001D));
    ChipDN issuerDN_Node02_02;
    EXPECT_EQ(CHIP_NO_ERROR, issuerDN_Node02_02.AddAttribute_MatterICACId(0xCACACACA00000004));
    EXPECT_EQ(CHIP_NO_ERROR, issuerDN_Node02_02.AddAttribute_MatterFabricId(0xFAB000000000001D));

    // clang-format off
    static TestCase sTestCases[] = {
        // Cert                    Expected Output
        // ==============================================
        {  TestCert::kRoot01,      issuerDN_Root01    },
        {  TestCert::kICA02,       issuerDN_ICA02     },
        {  TestCert::kNode02_02,   issuerDN_Node02_02 },
    };
    // clang-format on

    for (auto & testCase : sTestCases)
    {
        ByteSpan cert;
        err = GetTestCert(testCase.Cert, TestCertLoadFlags::kDERForm, cert);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        uint8_t issuerBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
        MutableByteSpan issuer(issuerBuf);
        err = ExtractIssuerFromX509Cert(cert, issuer);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        static uint8_t expectedIssuerBuf[kMaxCertificateDistinguishedNameLength] = { 0 };
        ASN1::ASN1Writer writer;
        writer.Init(expectedIssuerBuf);
        err = testCase.mExpectedDN.EncodeToASN1(writer);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        size_t expectedIssuerLen = writer.GetLengthWritten();
        EXPECT_EQ(expectedIssuerLen, issuer.size());
        EXPECT_EQ(memcmp(issuer.data(), expectedIssuerBuf, expectedIssuerLen), 0);
    }
}

TEST_F(TestChipCryptoPAL, TestVIDPID_StringExtraction)
{
    HeapChecker heapChecker;

    // Matter VID/PID Attribute examples (from the spec):
    const char * sTestMatterAttribute01 = "FFF1";
    const char * sTestMatterAttribute02 = "0000";
    const char * sTestMatterAttribute03 = "ABCD";
    const char * sTestMatterAttribute04 = "D90F";

    // Matter VID/PID Attribute error cases (from the spec):
    const char * sTestMatterAttribute05 = "12eF";
    const char * sTestMatterAttribute06 = "12345";
    const char * sTestMatterAttribute07 = "AB5";
    const char * sTestMatterAttribute08 = "abct";
    const char * sTestMatterAttribute09 = "10FH";
    const char * sTestMatterAttribute10 = "0x1234";
    const char * sTestMatterAttribute11 = "0x45";
    const char * sTestMatterAttribute12 = "Mvip:1234";
    const char * sTestMatterAttribute13 = "HELLO";
    const char * sTestMatterAttribute14 = "12";

    // Common Name (CN) VID/PID encoding examples (from the spec):
    const char * sTestCNAttribute01 = "Mvid:FFF1";
    const char * sTestCNAttribute02 = "Mvid:002A";
    const char * sTestCNAttribute03 = "Mpid:C20A";
    const char * sTestCNAttribute04 = "Mpid:03A5";
    const char * sTestCNAttribute05 = "ACME Matter Devel DAC 5CDA9899 Mvid:FFF1 Mpid:00B1";
    const char * sTestCNAttribute06 = "Mpid:00B1,ACME Matter Devel DAC 5CDA9899,Mvid:FFF1";
    const char * sTestCNAttribute07 = "ACME Matter Devel DAC 5CDA9899 Mvid:FFF1Mpid:00B1";
    const char * sTestCNAttribute08 = "Mvid:FFF1ACME Matter Devel DAC 5CDAMpid:00B19899";

    // Common Name (CN) VID/PID encoding error cases (from the spec):
    const char * sTestCNAttribute09 = "ACME Matter Devel DAC 5CDA9899 Mvid:FF1 Mpid:00B1";
    const char * sTestCNAttribute10 = "ACME Matter Devel DAC 5CDA9899 Mvid:fff1 Mpid:00B1";
    const char * sTestCNAttribute11 = "ACME Matter Devel DAC 5CDA9899 Mvid:FFF1 Mpid:B1";
    const char * sTestCNAttribute12 = "ACME Matter Devel DAC 5CDA9899 Mpid: Mvid:FFF1";

    // Common Name (CN) VID/PID encoding more cases (more examples):
    const char * sTestCNAttribute13 = "Mpid:987Mvid:FFF10x";
    const char * sTestCNAttribute14 = "MpidMvid:FFF10 Matter Test Mpid:FE67"; // Valid, even if there is run-in.
    const char * sTestCNAttribute15 = "Matter Devel Mpid:Mvid:Fff1";
    // Even though "Mpid:" appears thrice, only the value with correct hex afterwards is taken
    const char * sTestCNAttribute16 = "Mpid:Mvid:FFF1 Mpid:12cd Matter Test Mpid:FE67";

    struct TestCase
    {
        DNAttrType attrType;
        ByteSpan attr;
        bool expectedVidPresent;
        bool expectedPidPresent;
        VendorId expectedVid;
        uint16_t expectedPid;
        CHIP_ERROR expectedResult;
    };

    // clang-format off
    const TestCase kTestCases[] = {
        // Matter VID/PID Attribute examples:
        { DNAttrType::kMatterVID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute01), strlen(sTestMatterAttribute01)), true, false, chip::VendorId::TestVendor1, 0x0000, CHIP_NO_ERROR },
        { DNAttrType::kMatterVID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute02), strlen(sTestMatterAttribute02)), true, false, chip::VendorId::Common, 0x0000, CHIP_NO_ERROR },
        { DNAttrType::kMatterPID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute03), strlen(sTestMatterAttribute03)), false, true, chip::VendorId::NotSpecified, 0xABCD, CHIP_NO_ERROR },
        { DNAttrType::kMatterPID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute04), strlen(sTestMatterAttribute04)), false, true, chip::VendorId::NotSpecified, 0xD90F, CHIP_NO_ERROR },
        // Matter VID/PID Attribute error cases:
        { DNAttrType::kMatterVID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute05), strlen(sTestMatterAttribute05)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterPID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute06), strlen(sTestMatterAttribute06)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterVID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute07), strlen(sTestMatterAttribute07)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterPID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute08), strlen(sTestMatterAttribute08)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterVID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute09), strlen(sTestMatterAttribute09)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterPID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute10), strlen(sTestMatterAttribute10)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterVID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute11), strlen(sTestMatterAttribute11)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterPID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute12), strlen(sTestMatterAttribute12)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterVID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute13), strlen(sTestMatterAttribute13)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kMatterPID, ByteSpan(reinterpret_cast<const uint8_t *>(sTestMatterAttribute14), strlen(sTestMatterAttribute14)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        // Common Name (CN) VID/PID encoding examples:
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute01), strlen(sTestCNAttribute01)), true, false, chip::VendorId::TestVendor1, 0, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute02), strlen(sTestCNAttribute02)), true, false, static_cast<chip::VendorId>(0x002A), 0, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute03), strlen(sTestCNAttribute03)), false, true, chip::VendorId::NotSpecified, 0xC20A, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute04), strlen(sTestCNAttribute04)), false, true, chip::VendorId::NotSpecified, 0x03A5, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute05), strlen(sTestCNAttribute05)), true, true, chip::VendorId::TestVendor1, 0x00B1, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute06), strlen(sTestCNAttribute06)), true, true, chip::VendorId::TestVendor1, 0x00B1, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute07), strlen(sTestCNAttribute07)), true, true, chip::VendorId::TestVendor1, 0x00B1, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute08), strlen(sTestCNAttribute08)), true, true, chip::VendorId::TestVendor1, 0x00B1, CHIP_NO_ERROR },
        // Common Name (CN) VID/PID encoding error cases:
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute09), strlen(sTestCNAttribute09)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute10), strlen(sTestCNAttribute10)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute11), strlen(sTestCNAttribute11)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute12), strlen(sTestCNAttribute12)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        // Common Name (CN) VID/PID encoding additional cases:
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute13), strlen(sTestCNAttribute13)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute14), strlen(sTestCNAttribute14)), true, true, chip::VendorId::TestVendor1, 0xFE67, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute15), strlen(sTestCNAttribute15)), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_WRONG_CERT_DN },
        { DNAttrType::kCommonName, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute16), strlen(sTestCNAttribute16)), true, true, chip::VendorId::TestVendor1, 0xFE67, CHIP_NO_ERROR },
        // Other input combinations:
        { DNAttrType::kUnspecified, ByteSpan(reinterpret_cast<const uint8_t *>(sTestCNAttribute15), strlen(sTestCNAttribute15)), false, false, chip::VendorId::NotSpecified, 0, CHIP_NO_ERROR },
        { DNAttrType::kCommonName, ByteSpan(), false, false, chip::VendorId::NotSpecified, 0, CHIP_ERROR_INVALID_ARGUMENT },
    };
    // clang-format on

    [[maybe_unused]] int caseIdx = 0;
    for (const auto & testCase : kTestCases)
    {
        AttestationCertVidPid vidpid;
        AttestationCertVidPid vidpidFromCN;
        AttestationCertVidPid vidpidToCheck;
        CHIP_ERROR result = ExtractVIDPIDFromAttributeString(testCase.attrType, testCase.attr, vidpid, vidpidFromCN);
        ChipLogProgress(Crypto, "Checking VID/PID DN case %d. Expected: %" CHIP_ERROR_FORMAT, caseIdx,
                        testCase.expectedResult.Format());

        if (result != testCase.expectedResult)
        {
            ChipLogError(Crypto, "Actual result: %" CHIP_ERROR_FORMAT, result.Format());
        }
        EXPECT_EQ(result, testCase.expectedResult);

        // Only do assertions on output params in case of success since otherwise
        // many of the output params are intermediate outputs.
        if (result == CHIP_NO_ERROR)
        {
            if (testCase.attrType == DNAttrType::kMatterVID || testCase.attrType == DNAttrType::kMatterPID)
            {
                EXPECT_FALSE(vidpidFromCN.Initialized());
                vidpidToCheck = vidpid;
            }
            else if (testCase.attrType == DNAttrType::kCommonName)
            {
                EXPECT_FALSE(vidpid.Initialized());
                vidpidToCheck = vidpidFromCN;
            }

            EXPECT_EQ(vidpidToCheck.mVendorId.HasValue(), testCase.expectedVidPresent);
            EXPECT_EQ(vidpidToCheck.mProductId.HasValue(), testCase.expectedPidPresent);

            if (testCase.expectedVidPresent)
            {
                EXPECT_EQ(vidpidToCheck.mVendorId.Value(), testCase.expectedVid);
            }

            if (testCase.expectedPidPresent)
            {
                EXPECT_EQ(vidpidToCheck.mProductId.Value(), testCase.expectedPid);
            }
        }
        ++caseIdx;
    }
}

TEST_F(TestChipCryptoPAL, TestVIDPID_x509Extraction)
{
    using namespace TestCerts;

    HeapChecker heapChecker;

    // Test scenario where Certificate does not contain a Vendor ID field
    ByteSpan kOpCertNoVID;
    EXPECT_EQ(GetTestCert(TestCert::kNode01_01, TestCertLoadFlags::kDERForm, kOpCertNoVID), CHIP_NO_ERROR);

    struct TestCase
    {
        ByteSpan cert;
        bool expectedVidPresent;
        bool expectedPidPresent;
        VendorId expectedVid;
        uint16_t expectedPid;
        CHIP_ERROR expectedResult;
    };

    const TestCase kTestCases[] = {
        // VID and PID preset cases:
        { sTestCert_PAI_FFF1_8000_Cert, true, true, chip::VendorId::TestVendor1, 0x8000, CHIP_NO_ERROR },
        { sTestCert_DAC_FFF1_8000_0004_Cert, true, true, chip::VendorId::TestVendor1, 0x8000, CHIP_NO_ERROR },
        { sTestCert_PAI_FFF2_8001_Cert, true, true, chip::VendorId::TestVendor2, 0x8001, CHIP_NO_ERROR },
        { sTestCert_DAC_FFF2_8001_0009_Cert, true, true, chip::VendorId::TestVendor2, 0x8001, CHIP_NO_ERROR },
        { sTestCert_DAC_FFF2_8002_0016_Cert, true, true, chip::VendorId::TestVendor2, 0x8002, CHIP_NO_ERROR },
        { sTestCert_DAC_FFF2_8003_0019_FB_Cert, true, true, chip::VendorId::TestVendor2, 0x8003, CHIP_NO_ERROR },
        { sTestCert_DAC_FFF2_8004_001E_FB_Cert, true, true, chip::VendorId::TestVendor2, 0x8004, CHIP_NO_ERROR },
        { sTestCert_PAI_FFF2_8004_FB_Cert, true, true, chip::VendorId::TestVendor2, 0x8004, CHIP_NO_ERROR },
        // VID present and PID not present cases:
        { sTestCert_PAA_FFF1_Cert, true, false, chip::VendorId::TestVendor1, 0x0000, CHIP_NO_ERROR },
        { sTestCert_PAI_FFF2_NoPID_Cert, true, false, chip::VendorId::TestVendor2, 0x0000, CHIP_NO_ERROR },
        { sTestCert_PAI_FFF2_NoPID_FB_Cert, true, false, chip::VendorId::TestVendor2, 0x0000, CHIP_NO_ERROR },
        // VID and PID not present cases:
        { sTestCert_PAA_NoVID_Cert, false, false, chip::VendorId::NotSpecified, 0x0000, CHIP_NO_ERROR },
        { kOpCertNoVID, false, false, chip::VendorId::NotSpecified, 0x0000, CHIP_NO_ERROR },
    };

    for (const auto & testCase : kTestCases)
    {
        AttestationCertVidPid vidpid;
        CHIP_ERROR result = ExtractVIDPIDFromX509Cert(testCase.cert, vidpid);
        EXPECT_EQ(result, testCase.expectedResult);
        ASSERT_EQ(vidpid.mVendorId.HasValue(), testCase.expectedVidPresent);
        ASSERT_EQ(vidpid.mProductId.HasValue(), testCase.expectedPidPresent);

        // If present, make sure the VID matches expectation.
        if (testCase.expectedVidPresent)
        {
            EXPECT_EQ(vidpid.mVendorId.Value(), testCase.expectedVid);
        }

        // If present, make sure the VID matches expectation.
        if (testCase.expectedPidPresent)
        {
            EXPECT_EQ(vidpid.mProductId.Value(), testCase.expectedPid);
        }
    }
}

TEST_F(TestChipCryptoPAL, TestX509_ReplaceCertIfResignedCertFound)
{
    using namespace TestCerts;

    HeapChecker heapChecker;

    struct TestCase
    {
        ByteSpan referenceCert;
        ByteSpan * candidateCertsList;
        size_t candidateCertsCount;
        ByteSpan expectedOutCert;
    };

    // clang-format off
    ByteSpan TestCandidateCertsList1[] = { sTestCert_DAC_FFF1_8000_0004_Cert,
                                           sTestCert_PAI_FFF2_8004_FB_Cert,
                                           sTestCert_PAA_FFF1_Cert };
    ByteSpan TestCandidateCertsList2[] = { sTestCert_DAC_FFF1_8000_0004_Cert,
                                           sTestCert_PAI_FFF2_NoPID_Resigned_Cert };
    ByteSpan TestCandidateCertsList3[] = { sTestCert_DAC_FFF1_8000_0004_Cert,
                                           sTestCert_PAI_FFF2_8001_Resigned_Cert };
    ByteSpan TestCandidateCertsList4[] = { sTestCert_PAI_FFF2_8001_Resigned_Cert,
                                           sTestCert_PAI_FFF1_8000_Cert };
    ByteSpan TestCandidateCertsList5[] = { sTestCert_PAI_FFF2_NoPID_Resigned_Cert,
                                           sTestCert_PAI_FFF2_8001_ResignedSKIDDiff_Cert };
    ByteSpan TestCandidateCertsList6[] = { sTestCert_PAI_FFF2_8001_ResignedSubjectDiff_Cert,
                                           sTestCert_DAC_FFF1_8000_0004_Cert };
    ByteSpan TestCandidateCertsList7[] = { sTestCert_PAA_NoVID_ToResignPAIs_Cert,
                                           sTestCert_PAI_FFF2_8001_ResignedSKIDDiff_Cert,
                                           sTestCert_PAI_FFF2_8001_ResignedSubjectDiff_Cert,
                                           sTestCert_PAI_FFF2_8001_Resigned_Cert,
                                           sTestCert_PAI_FFF2_NoPID_Resigned_Cert };
    ByteSpan TestCandidateCertsList8[] = { sTestCert_PAA_NoVID_ToResignPAIs_Cert,
                                           sTestCert_PAI_FFF2_8001_Resigned_Cert,
                                           ByteSpan(),
                                           sTestCert_PAI_FFF2_8001_ResignedSKIDDiff_Cert,
                                           sTestCert_PAI_FFF2_8001_ResignedSubjectDiff_Cert,
                                           sTestCert_PAI_FFF2_NoPID_Resigned_Cert };

    const TestCase kTestCases[] = {
        { sTestCert_PAI_FFF2_8001_Cert, nullptr, 5, sTestCert_PAI_FFF2_8001_Cert },
        { sTestCert_PAI_FFF2_8001_Cert, TestCandidateCertsList3, 0, sTestCert_PAI_FFF2_8001_Cert },
        { sTestCert_PAI_FFF1_8000_Cert, TestCandidateCertsList1, ArraySize(TestCandidateCertsList1), sTestCert_PAI_FFF1_8000_Cert },
        { sTestCert_PAI_FFF2_8001_Cert, TestCandidateCertsList2, ArraySize(TestCandidateCertsList2), sTestCert_PAI_FFF2_8001_Cert },
        { sTestCert_PAI_FFF2_8001_Cert, TestCandidateCertsList3, ArraySize(TestCandidateCertsList3), sTestCert_PAI_FFF2_8001_Resigned_Cert },
        { sTestCert_PAI_FFF2_8001_Cert, TestCandidateCertsList4, ArraySize(TestCandidateCertsList4), sTestCert_PAI_FFF2_8001_Resigned_Cert },
        { sTestCert_PAI_FFF2_8001_Cert, TestCandidateCertsList5, ArraySize(TestCandidateCertsList5), sTestCert_PAI_FFF2_8001_Cert },
        { sTestCert_PAI_FFF2_8001_Cert, TestCandidateCertsList6, ArraySize(TestCandidateCertsList6), sTestCert_PAI_FFF2_8001_Cert },
        { sTestCert_PAI_FFF2_8001_Cert, TestCandidateCertsList7, ArraySize(TestCandidateCertsList7), sTestCert_PAI_FFF2_8001_Resigned_Cert },
        { sTestCert_PAI_FFF2_NoPID_Cert, TestCandidateCertsList7, ArraySize(TestCandidateCertsList7), sTestCert_PAI_FFF2_NoPID_Resigned_Cert },
    };
    // clang-format on

    for (const auto & testCase : kTestCases)
    {
        ByteSpan outCert;
        CHIP_ERROR result = ReplaceCertIfResignedCertFound(testCase.referenceCert, testCase.candidateCertsList,
                                                           testCase.candidateCertsCount, outCert);

        EXPECT_EQ(result, CHIP_NO_ERROR);
        EXPECT_TRUE(outCert.data_equal(testCase.expectedOutCert));
    }

    // Error case: invalid input argument for referenceCertificate
    {
        ByteSpan outCert;
        CHIP_ERROR result =
            ReplaceCertIfResignedCertFound(ByteSpan(), TestCandidateCertsList7, ArraySize(TestCandidateCertsList7), outCert);
        EXPECT_EQ(result, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Error case: invalid input argument for one of the certificates in the candidateCertificates list
    {
        ByteSpan outCert;
        CHIP_ERROR result =
            ReplaceCertIfResignedCertFound(ByteSpan(), TestCandidateCertsList8, ArraySize(TestCandidateCertsList8), outCert);
        EXPECT_EQ(result, CHIP_ERROR_INVALID_ARGUMENT);
    }
}

static const uint8_t kCompressedFabricId[] = { 0x29, 0x06, 0xC9, 0x08, 0xD1, 0x15, 0xD3, 0x62 };

const uint8_t kEpochKeyBuffer1[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
                                                                                   0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf };
const uint8_t kEpochKeyBuffer2[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
                                                                                   0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf };
const uint8_t kGroupOperationalKey1[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0x1f, 0x19, 0xed, 0x3c, 0xef, 0x8a,
                                                                                        0x21, 0x1b, 0xaf, 0x30, 0x6f, 0xae,
                                                                                        0xee, 0xe7, 0xaa, 0xc6 };
const uint8_t kGroupOperationalKey2[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0xaa, 0x97, 0x9a, 0x48, 0xbd, 0x8c,
                                                                                        0xdf, 0x29, 0x3a, 0x07, 0x09, 0xb9,
                                                                                        0xc1, 0xeb, 0x19, 0x30 };

static const uint8_t kCompressedFabricId2[]                                    = { 0x87, 0xe1, 0xb0, 0x04, 0xe2, 0x35, 0xa1, 0x30 };
const uint8_t kEpochKeyBuffer3[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0x23, 0x5b, 0xf7, 0xe6, 0x28, 0x23, 0xd3, 0x58,
                                                                                   0xdc, 0xa4, 0xba, 0x50, 0xb1, 0x53, 0x5f, 0x4b };
const uint8_t kGroupOperationalKey3[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0xa6, 0xf5, 0x30, 0x6b, 0xaf, 0x6d,
                                                                                        0x05, 0x0a, 0xf2, 0x3b, 0xa4, 0xbd,
                                                                                        0x6b, 0x9d, 0xd9, 0x60 };

const uint16_t kGroupSessionId1 = 0x6c80;
const uint16_t kGroupSessionId2 = 0x0c48;

static const uint8_t kGroupPrivacyKey1[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0xb8, 0x27, 0x9f, 0x89, 0x62, 0x1e,
                                                                                           0xd3, 0x27, 0xa9, 0xc3, 0x9f, 0x6a,
                                                                                           0x27, 0x24, 0x73, 0x58 };
static const uint8_t kGroupPrivacyKey2[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0xf7, 0x25, 0x70, 0xc3, 0xc0, 0x89,
                                                                                           0xa0, 0xfe, 0x28, 0x75, 0x83, 0x57,
                                                                                           0xaf, 0xff, 0xb8, 0xd2 };
static const uint8_t kGroupPrivacyKey3[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0x01, 0xf8, 0xd1, 0x92, 0x71, 0x26,
                                                                                           0xf1, 0x94, 0x08, 0x25, 0x72, 0xd4,
                                                                                           0x9b, 0x1f, 0xdc, 0x73 };

TEST_F(TestChipCryptoPAL, TestGroup_OperationalKeyDerivation)
{
    uint8_t key_buffer[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    ByteSpan epoch_key(kEpochKeyBuffer1, sizeof(kEpochKeyBuffer1));
    MutableByteSpan operational_key(key_buffer, sizeof(key_buffer));
    ByteSpan compressed_fabric_id(kCompressedFabricId);

    // Invalid Epoch Key
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, DeriveGroupOperationalKey(ByteSpan(), compressed_fabric_id, operational_key));

    // Epoch Key 1
    EXPECT_EQ(CHIP_NO_ERROR, DeriveGroupOperationalKey(epoch_key, compressed_fabric_id, operational_key));
    EXPECT_EQ(0, memcmp(operational_key.data(), kGroupOperationalKey1, sizeof(kGroupOperationalKey1)));

    // Epoch Key 2
    epoch_key = ByteSpan(kEpochKeyBuffer2, sizeof(kEpochKeyBuffer2));
    EXPECT_EQ(CHIP_NO_ERROR, DeriveGroupOperationalKey(epoch_key, compressed_fabric_id, operational_key));
    EXPECT_EQ(0, memcmp(operational_key.data(), kGroupOperationalKey2, sizeof(kGroupOperationalKey2)));

    // Epoch Key 3 (example from spec)
    epoch_key            = ByteSpan(kEpochKeyBuffer3, sizeof(kEpochKeyBuffer3));
    compressed_fabric_id = ByteSpan(kCompressedFabricId2);
    EXPECT_EQ(CHIP_NO_ERROR, DeriveGroupOperationalKey(epoch_key, compressed_fabric_id, operational_key));
    EXPECT_EQ(0, memcmp(operational_key.data(), kGroupOperationalKey3, sizeof(kGroupOperationalKey3)));
}

TEST_F(TestChipCryptoPAL, TestGroup_SessionIdDerivation)
{
    ByteSpan operational_key1(kGroupOperationalKey1, sizeof(kGroupOperationalKey1));
    ByteSpan operational_key2(kGroupOperationalKey2, sizeof(kGroupOperationalKey2));
    uint16_t session_id = 0;

    // Bad Key
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, DeriveGroupSessionId(ByteSpan(), session_id));

    // Session ID 1
    EXPECT_EQ(CHIP_NO_ERROR, DeriveGroupSessionId(operational_key1, session_id));
    EXPECT_EQ(kGroupSessionId1, session_id);

    // Session ID 2
    EXPECT_EQ(CHIP_NO_ERROR, DeriveGroupSessionId(operational_key2, session_id));
    EXPECT_EQ(kGroupSessionId2, session_id);
}

TEST_F(TestChipCryptoPAL, TestGroup_PrivacyKeyDerivation)
{
    uint8_t key_buffer[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    ByteSpan encryption_key;
    MutableByteSpan privacy_key(key_buffer, sizeof(key_buffer));

    // Invalid Epoch Key
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, DeriveGroupPrivacyKey(ByteSpan(), privacy_key));

    // Epoch Key 1
    encryption_key = ByteSpan(kGroupOperationalKey1, sizeof(kGroupOperationalKey1));
    EXPECT_EQ(CHIP_NO_ERROR, DeriveGroupPrivacyKey(encryption_key, privacy_key));
    EXPECT_EQ(0, memcmp(privacy_key.data(), kGroupPrivacyKey1, sizeof(kGroupPrivacyKey1)));

    // Epoch Key 2
    encryption_key = ByteSpan(kGroupOperationalKey2, sizeof(kGroupOperationalKey2));
    EXPECT_EQ(CHIP_NO_ERROR, DeriveGroupPrivacyKey(encryption_key, privacy_key));
    EXPECT_EQ(0, memcmp(privacy_key.data(), kGroupPrivacyKey2, sizeof(kGroupPrivacyKey2)));

    // Epoch Key 3 (example from spec)
    encryption_key = ByteSpan(kGroupOperationalKey3, sizeof(kGroupOperationalKey3));
    EXPECT_EQ(CHIP_NO_ERROR, DeriveGroupPrivacyKey(encryption_key, privacy_key));
    EXPECT_EQ(0, memcmp(privacy_key.data(), kGroupPrivacyKey3, sizeof(kGroupPrivacyKey3)));
}
