/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "AES_CCM_256_test_vectors.h"
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

#include <crypto/CHIPCryptoPAL.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/support/BytesToHex.h>

#if CHIP_CRYPTO_OPENSSL
#include "X509_PKCS7Extraction_test_vectors.h"
#endif

#include <credentials/CHIPCert.h>
#include <credentials/tests/CHIPCert_test_vectors.h>

#define HSM_ECC_KEYID 0x11223344

using namespace chip;
using namespace chip::Crypto;

#ifdef ENABLE_HSM_EC_KEY
class Test_P256Keypair : public P256KeypairHSM
{
public:
    Test_P256Keypair() { SetKeyId(HSM_ECC_KEYID); }
    Test_P256Keypair(uint32_t keyId) { SetKeyId(keyId); }
};
#else
using Test_P256Keypair                  = P256Keypair;
#endif

#ifdef ENABLE_HSM_SPAKE
using TestSpake2p_P256_SHA256_HKDF_HMAC = Spake2pHSM_P256_SHA256_HKDF_HMAC;
#else
using TestSpake2p_P256_SHA256_HKDF_HMAC = Spake2p_P256_SHA256_HKDF_HMAC;
#endif

#ifdef ENABLE_HSM_PBKDF2
using TestPBKDF2_sha256 = PBKDF2_sha256HSM;
#else
using TestPBKDF2_sha256                 = PBKDF2_sha256;
#endif

#ifdef ENABLE_HSM_HKDF
using TestHKDF_sha = HKDF_shaHSM;
#else
using TestHKDF_sha                      = HKDF_sha;
#endif

#ifdef ENABLE_HSM_HMAC
using TestHMAC_sha = HMAC_shaHSM;
#else
using TestHMAC_sha                      = HMAC_sha;
#endif

static uint32_t gs_test_entropy_source_called = 0;
static int test_entropy_source(void * data, uint8_t * output, size_t len, size_t * olen)
{
    *olen = len;
    gs_test_entropy_source_called++;
    return 0;
}

static void TestAES_CCM_256EncryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_ct;
            out_ct.Alloc(vector->ct_len);
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, vector->iv_len, out_ct.Get(), out_tag.Get(), vector->tag_len);
            bool areCTsEqual  = memcmp(out_ct.Get(), vector->ct, vector->ct_len) == 0;
            bool areTagsEqual = memcmp(out_tag.Get(), vector->tag, vector->tag_len) == 0;
            NL_TEST_ASSERT(inSuite, areCTsEqual);
            NL_TEST_ASSERT(inSuite, areTagsEqual);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            if (!areCTsEqual)
            {
                printf("\n Test %d failed due to mismatching ciphertext", vector->tcId);
            }
            if (!areTagsEqual)
            {
                printf("\n Test %d failed due to mismatching tags", vector->tcId);
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_pt;
            out_pt.Alloc(vector->pt_len);
            NL_TEST_ASSERT(inSuite, out_pt);
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, vector->iv_len, out_pt.Get());

            bool arePTsEqual = memcmp(vector->pt, out_pt.Get(), vector->pt_len) == 0;
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, arePTsEqual);
            if (!arePTsEqual)
            {
                printf("\n Test %d failed due to mismatching plaintext", vector->tcId);
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptInvalidPlainText(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_ct;
            out_ct.Alloc(vector->ct_len);
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, 0, vector->aad, vector->aad_len, vector->key, vector->key_len, vector->iv,
                                             vector->iv_len, out_ct.Get(), out_tag.Get(), vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptNilKey(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_ct;
            out_ct.Alloc(vector->ct_len);
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, nullptr, 32, vector->iv,
                                             vector->iv_len, out_ct.Get(), out_tag.Get(), vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptInvalidIVLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_ct;
            out_ct.Alloc(vector->ct_len);
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, 0, out_ct.Get(), out_tag.Get(), vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptInvalidTagLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_ct;
            out_ct.Alloc(vector->ct_len);
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, vector->iv_len, out_ct.Get(), out_tag.Get(), 13);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptInvalidCipherText(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_pt;
            out_pt.Alloc(vector->pt_len);
            NL_TEST_ASSERT(inSuite, out_pt);
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, 0, vector->aad, vector->aad_len, vector->tag, vector->tag_len, vector->key,
                                             vector->key_len, vector->iv, vector->iv_len, out_pt.Get());
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptInvalidKey(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_pt;
            out_pt.Alloc(vector->pt_len);
            NL_TEST_ASSERT(inSuite, out_pt);
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             nullptr, 32, vector->iv, vector->iv_len, out_pt.Get());
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptInvalidIVLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_pt;
            out_pt.Alloc(vector->pt_len);
            NL_TEST_ASSERT(inSuite, out_pt);
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, 0, out_pt.Get());
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptInvalidTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_invalid_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_invalid_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_pt;
            out_pt.Alloc(vector->pt_len);
            NL_TEST_ASSERT(inSuite, out_pt);
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, vector->iv_len, out_pt.Get());

            bool arePTsEqual = memcmp(vector->pt, out_pt.Get(), vector->pt_len) == 0;
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INTERNAL);
            NL_TEST_ASSERT(inSuite, arePTsEqual == false);
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
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
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, vector->iv_len, out_ct.Get(), out_tag.Get(), vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == vector->result);

            if (vector->result == CHIP_NO_ERROR)
            {
                bool areCTsEqual  = memcmp(out_ct.Get(), vector->ct, vector->ct_len) == 0;
                bool areTagsEqual = memcmp(out_tag.Get(), vector->tag, vector->tag_len) == 0;
                NL_TEST_ASSERT(inSuite, areCTsEqual);
                NL_TEST_ASSERT(inSuite, areTagsEqual);
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
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128DecryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
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
            NL_TEST_ASSERT(inSuite, out_pt);
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, vector->iv_len, out_pt.Get());

            NL_TEST_ASSERT(inSuite, err == vector->result);
            if (vector->result == CHIP_NO_ERROR)
            {
                bool arePTsEqual = memcmp(vector->pt, out_pt.Get(), vector->pt_len) == 0;
                NL_TEST_ASSERT(inSuite, arePTsEqual);
                if (!arePTsEqual)
                {
                    printf("\n Test %d failed due to mismatching plaintext\n", vector->tcId);
                }
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptInvalidPlainText(nlTestSuite * inSuite, void * inContext)
{
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
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, 0, vector->aad, vector->aad_len, vector->key, vector->key_len, vector->iv,
                                             vector->iv_len, out_ct.Get(), out_tag.Get(), vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptNilKey(nlTestSuite * inSuite, void * inContext)
{
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
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, nullptr, 0, vector->iv,
                                             vector->iv_len, out_ct.Get(), out_tag.Get(), vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptInvalidIVLen(nlTestSuite * inSuite, void * inContext)
{
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
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, 0, out_ct.Get(), out_tag.Get(), vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptInvalidTagLen(nlTestSuite * inSuite, void * inContext)
{
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
            NL_TEST_ASSERT(inSuite, out_ct);
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_tag;
            out_tag.Alloc(vector->tag_len);
            NL_TEST_ASSERT(inSuite, out_tag);

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, vector->iv_len, out_ct.Get(), out_tag.Get(), 13);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128DecryptInvalidCipherText(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, 0, vector->aad, vector->aad_len, vector->tag, vector->tag_len, vector->key,
                                             vector->key_len, vector->iv, vector->iv_len, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128DecryptInvalidKey(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             nullptr, 0, vector->iv, vector->iv_len, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128DecryptInvalidIVLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, 0, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAsn1Conversions(nlTestSuite * inSuite, void * inContext)
{
    static_assert(sizeof(kDerSigConvDerCase4) == (sizeof(kDerSigConvRawCase4) + chip::Crypto::kMax_ECDSA_X9Dot62_Asn1_Overhead),
                  "kDerSigConvDerCase4 must have worst case overhead");

    int numOfTestVectors = ArraySize(kDerSigConvTestVectors);
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const der_sig_conv_vector * vector = &kDerSigConvTestVectors[vectorIndex];

        chip::Platform::ScopedMemoryBuffer<uint8_t> out_raw_sig;
        size_t out_raw_sig_allocated_size = vector->fe_length_bytes * 2;
        out_raw_sig.Calloc(out_raw_sig_allocated_size);
        NL_TEST_ASSERT(inSuite, out_raw_sig);

        chip::Platform::ScopedMemoryBuffer<uint8_t> out_der_sig;
        size_t out_der_sig_allocated_size = (vector->fe_length_bytes * 2) + kMax_ECDSA_X9Dot62_Asn1_Overhead;
        out_der_sig.Calloc(out_der_sig_allocated_size);
        NL_TEST_ASSERT(inSuite, out_der_sig);

        // Test converstion from ASN.1 ER to raw
        MutableByteSpan out_raw_sig_span(out_raw_sig.Get(), out_raw_sig_allocated_size);

        CHIP_ERROR status = EcdsaAsn1SignatureToRaw(vector->fe_length_bytes,
                                                    ByteSpan{ vector->der_version, vector->der_version_length }, out_raw_sig_span);
        NL_TEST_ASSERT(inSuite, status == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, out_raw_sig_span.size() == vector->raw_version_length);
        NL_TEST_ASSERT(inSuite, (memcmp(out_raw_sig_span.data(), vector->raw_version, vector->raw_version_length) == 0));

        // Test conversion from raw to ASN.1 DER
        MutableByteSpan out_der_sig_span(out_der_sig.Get(), out_der_sig_allocated_size);
        status = EcdsaRawSignatureToAsn1(vector->fe_length_bytes, ByteSpan{ vector->raw_version, vector->raw_version_length },
                                         out_der_sig_span);
        NL_TEST_ASSERT(inSuite, status == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, out_der_sig_span.size() <= out_der_sig_allocated_size);
        NL_TEST_ASSERT(inSuite, out_der_sig_span.size() == vector->der_version_length);
        NL_TEST_ASSERT(inSuite, (memcmp(out_der_sig_span.data(), vector->der_version, vector->der_version_length) == 0));
    }
}

static void TestRawIntegerToDerValidCases(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases = ArraySize(kRawIntegerToDerVectors);

    for (int testIdx = 0; testIdx < numOfTestCases; testIdx++)
    {
        RawIntegerToDerVector v = kRawIntegerToDerVectors[testIdx];

        // Cover case with tag/length
        {
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_der_buffer;
            out_der_buffer.Alloc(v.expected_size);
            NL_TEST_ASSERT(inSuite, out_der_buffer);

            MutableByteSpan out_der_integer(out_der_buffer.Get(), v.expected_size);
            CHIP_ERROR status = ConvertIntegerRawToDer(ByteSpan{ v.candidate, v.candidate_size }, out_der_integer);
            NL_TEST_ASSERT(inSuite, status == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, out_der_integer.size() == v.expected_size);
            NL_TEST_ASSERT(inSuite, out_der_integer.data_equal(ByteSpan(v.expected, v.expected_size)));

            // Cover case of buffer too small
            MutableByteSpan out_der_integer_too_small(out_der_buffer.Get(), v.expected_size - 1);
            status = ConvertIntegerRawToDer(ByteSpan{ v.candidate, v.candidate_size }, out_der_integer_too_small);
            NL_TEST_ASSERT(inSuite, status == CHIP_ERROR_BUFFER_TOO_SMALL);
        }

        // Cover case without tag/length
        {
            chip::Platform::ScopedMemoryBuffer<uint8_t> out_der_buffer;
            out_der_buffer.Alloc(v.expected_without_tag_size);
            NL_TEST_ASSERT(inSuite, out_der_buffer);

            MutableByteSpan out_der_integer(out_der_buffer.Get(), v.expected_without_tag_size);
            CHIP_ERROR status = ConvertIntegerRawToDerWithoutTag(ByteSpan{ v.candidate, v.candidate_size }, out_der_integer);

            NL_TEST_ASSERT(inSuite, status == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, out_der_integer.size() == v.expected_without_tag_size);
            NL_TEST_ASSERT(inSuite, out_der_integer.data_equal(ByteSpan(v.expected_without_tag, v.expected_without_tag_size)));
        }
    }
}

static void TestRawIntegerToDerInvalidCases(nlTestSuite * inSuite, void * inContext)
{
    // Cover case of invalid buffers
    uint8_t placeholder[10] = { 0 };
    MutableByteSpan good_out_buffer(placeholder, sizeof(placeholder));
    ByteSpan good_buffer(placeholder, sizeof(placeholder));

    MutableByteSpan bad_out_buffer_nullptr(nullptr, sizeof(placeholder));
    MutableByteSpan bad_out_buffer_empty(placeholder, 0);

    ByteSpan bad_buffer_nullptr(nullptr, sizeof(placeholder));
    ByteSpan bad_buffer_empty(placeholder, 0);

    struct ErrorCase
    {
        const ByteSpan & input;
        MutableByteSpan & output;
        CHIP_ERROR expected_status;
    };

    const ErrorCase error_cases[] = {
        { .input = good_buffer, .output = bad_out_buffer_nullptr, .expected_status = CHIP_ERROR_INVALID_ARGUMENT },
        { .input = good_buffer, .output = bad_out_buffer_empty, .expected_status = CHIP_ERROR_INVALID_ARGUMENT },
        { .input = bad_buffer_nullptr, .output = good_out_buffer, .expected_status = CHIP_ERROR_INVALID_ARGUMENT },
        { .input = bad_buffer_empty, .output = good_out_buffer, .expected_status = CHIP_ERROR_INVALID_ARGUMENT }
    };

    int case_idx = 0;
    for (const ErrorCase & v : error_cases)
    {
        CHIP_ERROR status = ConvertIntegerRawToDerWithoutTag(v.input, v.output);
        if (status != v.expected_status)
        {
            ChipLogError(Crypto, "Failed TestRawIntegerToDerInvalidCases sub-case %d", case_idx);
            NL_TEST_ASSERT(inSuite, v.expected_status == status);
        }
        ++case_idx;
    }
}

static void TestHash_SHA256(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases     = ArraySize(hash_sha256_test_vectors);
    int numOfTestsExecuted = 0;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hash_sha256_vector v = hash_sha256_test_vectors[numOfTestsExecuted];
        uint8_t out_buffer[kSHA256_Hash_Length];
        Hash_SHA256(v.data, v.data_length, out_buffer);
        bool success = memcmp(v.hash, out_buffer, sizeof(out_buffer)) == 0;
        NL_TEST_ASSERT(inSuite, success);
    }
    NL_TEST_ASSERT(inSuite, numOfTestsExecuted == ArraySize(hash_sha256_test_vectors));
}

static void TestHash_SHA256_Stream(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases     = ArraySize(hash_sha256_test_vectors);
    int numOfTestsExecuted = 0;
    CHIP_ERROR error       = CHIP_NO_ERROR;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hash_sha256_vector v = hash_sha256_test_vectors[numOfTestsExecuted];
        const uint8_t * data = v.data;
        size_t data_length   = v.data_length;
        uint8_t out_buffer[kSHA256_Hash_Length];

        Hash_SHA256_stream sha256;

        error = sha256.Begin();
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Split data into 3 random streams.
        for (int i = 0; i < 2; ++i)
        {
            size_t rand_data_length = static_cast<unsigned int>(rand()) % (data_length + 1);

            error = sha256.AddData(ByteSpan{ data, rand_data_length });
            NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

            data += rand_data_length;
            data_length -= rand_data_length;
        }

        error = sha256.AddData(ByteSpan{ data, data_length });
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        MutableByteSpan out_span(out_buffer);
        error = sha256.Finish(out_span);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, out_span.size() == kSHA256_Hash_Length);

        bool success = memcmp(v.hash, out_span.data(), out_span.size()) == 0;
        NL_TEST_ASSERT(inSuite, success);
    }

    NL_TEST_ASSERT(inSuite, numOfTestsExecuted == ArraySize(hash_sha256_test_vectors));

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
            NL_TEST_ASSERT(inSuite, sha256.Begin() == CHIP_NO_ERROR);

            // Compute partial digest after first block
            NL_TEST_ASSERT(inSuite, sha256.AddData(ByteSpan{ &source_buf[0], block1_size }) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, sha256.GetDigest(partial_digest_span1) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, partial_digest_span1.size() == kSHA256_Hash_Length);

            // Validate partial digest matches expectations
            Hash_SHA256(&source_buf[0], block1_size, &partial_digest_ref[0]);
            NL_TEST_ASSERT(inSuite, 0 == memcmp(partial_digest_span1.data(), partial_digest_ref, partial_digest_span1.size()));

            // Compute partial digest and total digest after second block
            NL_TEST_ASSERT(inSuite, sha256.AddData(ByteSpan{ &source_buf[block1_size], block2_size }) == CHIP_NO_ERROR);

            NL_TEST_ASSERT(inSuite, sha256.GetDigest(partial_digest_span2) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, partial_digest_span2.size() == kSHA256_Hash_Length);

            NL_TEST_ASSERT(inSuite, sha256.Finish(total_digest_span) == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, total_digest_span.size() == kSHA256_Hash_Length);

            // Validate second partial digest matches final digest
            Hash_SHA256(&source_buf[0], block1_size + block2_size, &total_digest_ref[0]);
            NL_TEST_ASSERT(inSuite, 0 == memcmp(partial_digest_span2.data(), total_digest_ref, partial_digest_span2.size()));
            NL_TEST_ASSERT(inSuite, 0 == memcmp(total_digest_span.data(), total_digest_ref, total_digest_span.size()));
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
        NL_TEST_ASSERT(inSuite, sha256.Begin() == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, sha256.AddData(ByteSpan{ source_buf2 }) == CHIP_NO_ERROR);

        // Check that error behavior works on buffer too small
        NL_TEST_ASSERT(inSuite, sha256.GetDigest(digest_span_too_small) == CHIP_ERROR_BUFFER_TOO_SMALL);
        NL_TEST_ASSERT(inSuite, sha256.Finish(digest_span_too_small) == CHIP_ERROR_BUFFER_TOO_SMALL);

        // Check that both GetDigest/Finish can still work after error.
        NL_TEST_ASSERT(inSuite, sha256.GetDigest(digest_span_ok) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, 0 == memcmp(digest_span_ok.data(), digest_buf_ref, digest_span_ok.size()));

        memset(digest_buf_ok, 0, sizeof(digest_buf_ok));

        NL_TEST_ASSERT(inSuite, sha256.Finish(digest_span_ok) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, 0 == memcmp(digest_span_ok.data(), digest_buf_ref, digest_span_ok.size()));
    }
}

static void TestHMAC_SHA256(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases     = ArraySize(hmac_sha256_test_vectors);
    int numOfTestsExecuted = 0;
    TestHMAC_sha mHMAC;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hmac_sha256_vector v = hmac_sha256_test_vectors[numOfTestsExecuted];
        size_t out_length    = v.output_hash_length;
        chip::Platform::ScopedMemoryBuffer<uint8_t> out_buffer;
        out_buffer.Alloc(out_length);
        NL_TEST_ASSERT(inSuite, out_buffer);
        mHMAC.HMAC_SHA256(v.key, v.key_length, v.message, v.message_length, out_buffer.Get(), v.output_hash_length);
        bool success = memcmp(v.output_hash, out_buffer.Get(), out_length) == 0;
        NL_TEST_ASSERT(inSuite, success);
    }
    NL_TEST_ASSERT(inSuite, numOfTestsExecuted == numOfTestCases);
}

static void TestHKDF_SHA256(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases     = ArraySize(hkdf_sha256_test_vectors);
    int numOfTestsExecuted = 0;
    TestHKDF_sha mHKDF;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hkdf_sha256_vector v = hkdf_sha256_test_vectors[numOfTestsExecuted];
        size_t out_length    = v.output_key_material_length;
        chip::Platform::ScopedMemoryBuffer<uint8_t> out_buffer;
        out_buffer.Alloc(out_length);
        NL_TEST_ASSERT(inSuite, out_buffer);
        mHKDF.HKDF_SHA256(v.initial_key_material, v.initial_key_material_length, v.salt, v.salt_length, v.info, v.info_length,
                          out_buffer.Get(), v.output_key_material_length);
        bool success = memcmp(v.output_key_material, out_buffer.Get(), out_length) == 0;
        NL_TEST_ASSERT(inSuite, success);
    }
    NL_TEST_ASSERT(inSuite, numOfTestsExecuted == 3);
}

static void TestDRBG_InvalidInputs(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    error            = DRBG_get_bytes(nullptr, 10);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INVALID_ARGUMENT);
    error = CHIP_NO_ERROR;
    uint8_t buffer[5];
    error = DRBG_get_bytes(buffer, 0);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INVALID_ARGUMENT);
}

static void TestDRBG_Output(nlTestSuite * inSuite, void * inContext)
{
    // No good way to unit test a DRBG. Just validate that we get out something
    CHIP_ERROR error     = CHIP_ERROR_INVALID_ARGUMENT;
    uint8_t out_buf[10]  = { 0 };
    uint8_t orig_buf[10] = { 0 };

    error = DRBG_get_bytes(out_buf, sizeof(out_buf));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(out_buf, orig_buf, sizeof(out_buf)) != 0);
}

static void TestECDSA_Signing_SHA256_Msg(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen(msg);

    Test_P256Keypair keypair;

    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    CHIP_ERROR validation_error =
        keypair.Pubkey().ECDSA_validate_msg_signature(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_NO_ERROR);
}

static void TestECDSA_Signing_SHA256_Hash(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t hash[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                             0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    size_t hash_length   = sizeof(hash);

    Test_P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    // TODO: Need to make this large number (1k+) to catch some signature serialization corner cases
    //       but this is too slow on QEMU/embedded, so we need to parametrize. Signing with ECDSA
    //       is non-deterministic by design (since knowledge of the value `k` used allows recovery
    //       of the private key).
    constexpr int kNumSigningIterations = 3;

    for (int i = 0; i < kNumSigningIterations; ++i)
    {
        P256ECDSASignature signature;
        CHIP_ERROR signing_error = keypair.ECDSA_sign_hash(hash, hash_length, signature);
        NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

        CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(hash, hash_length, signature);
        NL_TEST_ASSERT(inSuite, validation_error == CHIP_NO_ERROR);

        if ((signing_error != CHIP_NO_ERROR) || (validation_error != CHIP_NO_ERROR))
        {
            ChipLogError(Crypto, "TestECDSA_Signing_SHA256_Hash failed after %d/%d iterations", i + 1, kNumSigningIterations);
            break;
        }
    }
}

static void TestECDSA_ValidationFailsDifferentMessage(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen(msg);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    const char * diff_msg  = "NOT Hello World!";
    size_t diff_msg_length = strlen(msg);
    CHIP_ERROR validation_error =
        keypair.Pubkey().ECDSA_validate_msg_signature(reinterpret_cast<const uint8_t *>(diff_msg), diff_msg_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_SIGNATURE);
}

static void TestECDSA_ValidationFailsDifferentHash(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t hash[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                             0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    size_t hash_length   = sizeof(hash);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_hash(hash, hash_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    const uint8_t diff_hash[] = { 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
                                  0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x02, 0x00 };
    size_t diff_hash_length   = sizeof(diff_hash);

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(diff_hash, diff_hash_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_SIGNATURE);
}

static void TestECDSA_ValidationFailIncorrectMsgSignature(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen(msg);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);
    signature[0] = static_cast<uint8_t>(~signature[0]); // Flipping bits should invalidate the signature.

    CHIP_ERROR validation_error =
        keypair.Pubkey().ECDSA_validate_msg_signature(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_SIGNATURE);
}

static void TestECDSA_ValidationFailIncorrectHashSignature(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t hash[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                             0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    size_t hash_length   = sizeof(hash);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_hash(hash, hash_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);
    signature[0] = static_cast<uint8_t>(~signature[0]); // Flipping bits should invalidate the signature.

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(hash, hash_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_SIGNATURE);
}

static void TestECDSA_SigningMsgInvalidParams(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t * msg = reinterpret_cast<const uint8_t *>("Hello World!");
    size_t msg_length   = strlen(reinterpret_cast<const char *>(msg));

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(nullptr, msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    signing_error = keypair.ECDSA_sign_msg(msg, 0, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;
}

static void TestECDSA_SigningHashInvalidParams(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t hash[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                             0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    size_t hash_length   = sizeof(hash);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_hash(nullptr, hash_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    signing_error = keypair.ECDSA_sign_hash(hash, hash_length - 5, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;
}

static void TestECDSA_ValidationMsgInvalidParam(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen(msg);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(reinterpret_cast<const uint8_t *>(msg), msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_msg_signature(nullptr, msg_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;

    validation_error = keypair.Pubkey().ECDSA_validate_msg_signature(reinterpret_cast<const uint8_t *>(msg), 0, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;
}

static void TestECDSA_ValidationHashInvalidParam(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t hash[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                             0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    size_t hash_length   = sizeof(hash);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_hash(hash, hash_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(nullptr, hash_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    validation_error = keypair.Pubkey().ECDSA_validate_hash_signature(hash, hash_length - 5, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;
}

static void TestECDH_EstablishSecret(nlTestSuite * inSuite, void * inContext)
{
    Test_P256Keypair keypair1;
    NL_TEST_ASSERT(inSuite, keypair1.Initialize() == CHIP_NO_ERROR);

#ifdef ENABLE_HSM_EC_KEY
    Test_P256Keypair keypair2(HSM_ECC_KEYID + 1);
#else
    Test_P256Keypair keypair2;
#endif
    NL_TEST_ASSERT(inSuite, keypair2.Initialize() == CHIP_NO_ERROR);

    P256ECDHDerivedSecret out_secret1;
    out_secret1[0] = 0;

    P256ECDHDerivedSecret out_secret2;
    out_secret2[0] = 1;

    CHIP_ERROR error = CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite,
                   memcmp(Uint8::to_uchar(out_secret1), Uint8::to_uchar(out_secret2), out_secret1.Capacity()) !=
                       0); // Validate that buffers are indeed different.

    error = keypair2.ECDH_derive_secret(keypair1.Pubkey(), out_secret1);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = keypair1.ECDH_derive_secret(keypair2.Pubkey(), out_secret2);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    bool signature_lengths_match = out_secret1.Length() == out_secret2.Length();
    NL_TEST_ASSERT(inSuite, signature_lengths_match);

    bool signatures_match = (memcmp(Uint8::to_uchar(out_secret1), Uint8::to_uchar(out_secret2), out_secret1.Length()) == 0);
    NL_TEST_ASSERT(inSuite, signatures_match);
}

#if CHIP_CRYPTO_OPENSSL
static void TestAddEntropySources(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR error = add_entropy_source(test_entropy_source, nullptr, 10);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    uint8_t buffer[5];
    NL_TEST_ASSERT(inSuite, DRBG_get_bytes(buffer, sizeof(buffer)) == CHIP_NO_ERROR);
}
#endif

#if CHIP_CRYPTO_MBEDTLS
static void TestAddEntropySources(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR error = add_entropy_source(test_entropy_source, nullptr, 10);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    uint8_t buffer[5];
    uint32_t test_entropy_source_call_count = gs_test_entropy_source_called;
    NL_TEST_ASSERT(inSuite, DRBG_get_bytes(buffer, sizeof(buffer)) == CHIP_NO_ERROR);
    for (int i = 0; i < 5000 * 2; i++)
    {
        (void) DRBG_get_bytes(buffer, sizeof(buffer));
    }
    NL_TEST_ASSERT(inSuite, gs_test_entropy_source_called > test_entropy_source_call_count);
}
#endif

static void TestPBKDF2_SHA256_TestVectors(nlTestSuite * inSuite, void * inContext)
{
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
            NL_TEST_ASSERT(inSuite, out_key);

            CHIP_ERROR err = pbkdf1.pbkdf2_sha256(vector->password, vector->plen, vector->salt, vector->slen, vector->iter,
                                                  vector->key_len, out_key.Get());
            NL_TEST_ASSERT(inSuite, err == vector->result);

            if (vector->result == CHIP_NO_ERROR)
            {
                NL_TEST_ASSERT(inSuite, memcmp(out_key.Get(), vector->key, vector->key_len) == 0);
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestP256_Keygen(nlTestSuite * inSuite, void * inContext)
{
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    const char * msg         = "Test Message for Keygen";
    const uint8_t * test_msg = Uint8::from_const_char(msg);
    size_t msglen            = strlen(msg);

    P256ECDSASignature test_sig;
    NL_TEST_ASSERT(inSuite, keypair.ECDSA_sign_msg(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keypair.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
}

static void TestCSR_Gen(nlTestSuite * inSuite, void * inContext)
{
    uint8_t csr[kMAX_CSR_Length];
    size_t length = sizeof(csr);

    Test_P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keypair.NewCertificateSigningRequest(csr, length) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, length > 0);

    P256PublicKey pubkey;
    CHIP_ERROR err = VerifyCertificateSigningRequest(csr, length, pubkey);
    if (err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, pubkey.Length() == kP256_PublicKey_Length);
        NL_TEST_ASSERT(inSuite, memcmp(pubkey.ConstBytes(), keypair.Pubkey().ConstBytes(), pubkey.Length()) == 0);

        // Let's corrupt the CSR buffer and make sure it fails to verify
        csr[length - 2] = (uint8_t)(csr[length - 2] + 1);
        csr[length - 1] = (uint8_t)(csr[length - 1] + 1);

        NL_TEST_ASSERT(inSuite, VerifyCertificateSigningRequest(csr, length, pubkey) != CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(Crypto, "The current platform does not support CSR parsing.");
    }
}

static void TestKeypair_Serialize(nlTestSuite * inSuite, void * inContext)
{
    Test_P256Keypair keypair;

    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256SerializedKeypair serialized;
    NL_TEST_ASSERT(inSuite, keypair.Serialize(serialized) == CHIP_NO_ERROR);

    Test_P256Keypair keypair_dup;
    NL_TEST_ASSERT(inSuite, keypair_dup.Deserialize(serialized) == CHIP_NO_ERROR);

    const char * msg         = "Test Message for Keygen";
    const uint8_t * test_msg = Uint8::from_const_char(msg);
    size_t msglen            = strlen(msg);

    P256ECDSASignature test_sig;
    NL_TEST_ASSERT(inSuite, keypair.ECDSA_sign_msg(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keypair_dup.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, keypair_dup.ECDSA_sign_msg(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keypair.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
}

static void TestSPAKE2P_spake2p_FEMul(nlTestSuite * inSuite, void * inContext)
{
    uint8_t fe_out[kMAX_FE_Length];

    int numOfTestVectors = ArraySize(fe_mul_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_fe_mul_tv * vector = fe_mul_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe1, vector->fe1_len, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe2, vector->fe2_len, spake2p.w1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FEMul(spake2p.xy, spake2p.w0, spake2p.w1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FEWrite(spake2p.xy, fe_out, sizeof(fe_out));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(fe_out, vector->fe_out, vector->fe_out_len) == 0);
        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_FELoadWrite(nlTestSuite * inSuite, void * inContext)
{
    uint8_t fe_out[kMAX_FE_Length];

    int numOfTestVectors = ArraySize(fe_rw_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_fe_rw_tv * vector = fe_rw_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe_in, vector->fe_in_len, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FEWrite(spake2p.w0, fe_out, sizeof(fe_out));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(fe_out, vector->fe_out, vector->fe_out_len) == 0);
        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_Mac(nlTestSuite * inSuite, void * inContext)
{
    uint8_t mac[kMAX_Hash_Length];

    int numOfTestVectors = ArraySize(hmac_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_hmac_tv * vector = hmac_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.Mac(vector->key, vector->key_len, vector->input, vector->input_len, mac);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(mac, vector->output, vector->output_len) == 0);

        err = spake2p.MacVerify(vector->key, vector->key_len, vector->output, vector->output_len, vector->input, vector->input_len);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_PointMul(nlTestSuite * inSuite, void * inContext)
{
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
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar, vector->scalar_len, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointMul(spake2p.X, spake2p.L, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.X, output, out_len);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(output, vector->out_point, vector->out_point_len) == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_PointMulAdd(nlTestSuite * inSuite, void * inContext)
{
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
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point1, vector->point1_len, spake2p.X);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point2, vector->point2_len, spake2p.Y);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar1, vector->scalar1_len, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar2, vector->scalar2_len, spake2p.w1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointAddMul(spake2p.L, spake2p.X, spake2p.w0, spake2p.Y, spake2p.w1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.L, output, out_len);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(output, vector->out_point, vector->out_point_len) == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_PointLoadWrite(nlTestSuite * inSuite, void * inContext)
{
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
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.L, output, out_len);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(output, vector->point, vector->point_len) == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_PointIsValid(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(point_valid_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_point_valid_tv * vector = point_valid_tvs[vectorIndex];

        TestSpake2p_P256_SHA256_HKDF_HMAC spake2p;

        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        // The underlying implementation may (i.e. should) check for validity when loading a point. Let's catch this case.
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR || vector->valid == 0);

        err = spake2p.PointIsValid(spake2p.L);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR || vector->valid == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

// We need to "generate" specific field elements
// to do so we need to override the specific method
class Test_Spake2p_P256_SHA256_HKDF_HMAC :
#ifdef ENABLE_HSM_SPAKE
    public Spake2pHSM_P256_SHA256_HKDF_HMAC
#else
    public Spake2p_P256_SHA256_HKDF_HMAC
#endif
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

static void TestSPAKE2P_RFC(nlTestSuite * inSuite, void * inContext)
{
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
    uint8_t VKe[kMAX_Hash_Length];
    size_t VKe_len = sizeof(VKe);
    uint8_t PKe[kMAX_Hash_Length];
    size_t PKe_len = sizeof(PKe);

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
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        error = Prover.BeginProver(vector->prover_identity, vector->prover_identity_len, vector->verifier_identity,
                                   vector->verifier_identity_len, vector->w0, vector->w0_len, vector->w1, vector->w1_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Monkey patch the generated x coordinate
        error = Prover.TestSetFE(vector->x, vector->x_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Compute the first round and send it to the verifier
        X_len = sizeof(X);
        error = Prover.ComputeRoundOne(NULL, 0, X, &X_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, X_len == vector->X_len);
        NL_TEST_ASSERT(inSuite, memcmp(X, vector->X, vector->X_len) == 0);

        // Start up the verifier
        error = Verifier.Init(vector->context, vector->context_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // First pre-compute L (accessories with dynamic setup codes will do this)
        L_len = sizeof(L);
        error = Verifier.ComputeL(L, &L_len, vector->w1, vector->w1_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, L_len == vector->L_len);
        NL_TEST_ASSERT(inSuite, memcmp(L, vector->L, vector->L_len) == 0);

        // Start up the verifier
        error = Verifier.BeginVerifier(vector->verifier_identity, vector->verifier_identity_len, vector->prover_identity,
                                       vector->prover_identity_len, vector->w0, vector->w0_len, L, L_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Monkey patch the generated y coordinate
        error = Verifier.TestSetFE(vector->y, vector->y_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Compute the first round and send it to the prover
        Y_len = sizeof(Y);
        error = Verifier.ComputeRoundOne(X, X_len, Y, &Y_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, Y_len == vector->Y_len);
        NL_TEST_ASSERT(inSuite, memcmp(Y, vector->Y, vector->Y_len) == 0);

        // Compute the second round to also send to the prover
        Vverifier_len = sizeof(Vverifier);
        error         = Verifier.ComputeRoundTwo(X, X_len, Vverifier, &Vverifier_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, Vverifier_len == vector->MAC_KcB_len);
        NL_TEST_ASSERT(inSuite, memcmp(Vverifier, vector->MAC_KcB, vector->MAC_KcB_len) == 0);

        error = Verifier.PointWrite(Verifier.Z, Z, kP256_Point_Length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(Z, vector->Z, vector->Z_len) == 0);

        error = Verifier.PointWrite(Verifier.V, V, kP256_Point_Length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(V, vector->V, vector->V_len) == 0);

        // Now the prover computes round 2
        Pverifier_len = sizeof(Pverifier);
        error         = Prover.ComputeRoundTwo(Y, Y_len, Pverifier, &Pverifier_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, Pverifier_len == vector->MAC_KcA_len);
        NL_TEST_ASSERT(inSuite, memcmp(Pverifier, vector->MAC_KcA, vector->MAC_KcA_len) == 0);

        error = Prover.PointWrite(Verifier.Z, Z, kP256_Point_Length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(Z, vector->Z, vector->Z_len) == 0);

        error = Prover.PointWrite(Verifier.V, V, kP256_Point_Length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(V, vector->V, vector->V_len) == 0);

        // Both sides now confirm the keys they received
        error = Prover.KeyConfirm(Vverifier, Vverifier_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        error = Verifier.KeyConfirm(Pverifier, Pverifier_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        PKe_len = sizeof(PKe);
        error   = Prover.GetKeys(PKe, &PKe_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, PKe_len == vector->Ke_len);
        NL_TEST_ASSERT(inSuite, memcmp(PKe, vector->Ke, vector->Ke_len) == 0);

        VKe_len = sizeof(VKe);
        error   = Verifier.GetKeys(VKe, &VKe_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, VKe_len == vector->Ke_len);
        NL_TEST_ASSERT(inSuite, memcmp(VKe, vector->Ke, vector->Ke_len) == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestCompressedFabricIdentifier(nlTestSuite * inSuite, void * inContext)
{
    // Data from spec test vector (see Operational Discovery section)
    const uint8_t kRootPublicKey[65] = {
        0x04, 0x4a, 0x9f, 0x42, 0xb1, 0xca, 0x48, 0x40, 0xd3, 0x72, 0x92, 0xbb, 0xc7, 0xf6, 0xa7, 0xe1, 0x1e,
        0x22, 0x20, 0x0c, 0x97, 0x6f, 0xc9, 0x00, 0xdb, 0xc9, 0x8a, 0x7a, 0x38, 0x3a, 0x64, 0x1c, 0xb8, 0x25,
        0x4a, 0x2e, 0x56, 0xd4, 0xe2, 0x95, 0xa8, 0x47, 0x94, 0x3b, 0x4e, 0x38, 0x97, 0xc4, 0xa7, 0x73, 0xe9,
        0x30, 0x27, 0x7b, 0x4d, 0x9f, 0xbe, 0xde, 0x8a, 0x05, 0x26, 0x86, 0xbf, 0xac, 0xfa,
    };
    P256PublicKey root_public_key(kRootPublicKey);

    constexpr uint64_t kFabricId = 0x2906C908D115D362;

    const uint8_t kExpectedCompressedFabricIdentifier[8] = {
        0x87, 0xe1, 0xb0, 0x04, 0xe2, 0x35, 0xa1, 0x30,
    };
    static_assert(sizeof(kExpectedCompressedFabricIdentifier) == kCompressedFabricIdentifierSize,
                  "Expected compressed fabric identifier must the correct size");

    uint8_t compressed_fabric_id[kCompressedFabricIdentifierSize];
    MutableByteSpan compressed_fabric_id_span(compressed_fabric_id);
    ClearSecretData(compressed_fabric_id, sizeof(compressed_fabric_id));

    CHIP_ERROR error = GenerateCompressedFabricId(root_public_key, kFabricId, compressed_fabric_id_span);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, compressed_fabric_id_span.size() == kCompressedFabricIdentifierSize);
    NL_TEST_ASSERT(inSuite,
                   0 ==
                       memcmp(compressed_fabric_id_span.data(), kExpectedCompressedFabricIdentifier,
                              sizeof(kExpectedCompressedFabricIdentifier)));

    // Test bigger input buffer than needed
    uint8_t compressed_fabric_id_large[3 * kCompressedFabricIdentifierSize];
    MutableByteSpan compressed_fabric_id_large_span(compressed_fabric_id_large);
    ClearSecretData(compressed_fabric_id_large, sizeof(compressed_fabric_id_large));

    error = GenerateCompressedFabricId(root_public_key, kFabricId, compressed_fabric_id_large_span);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, compressed_fabric_id_large_span.size() == kCompressedFabricIdentifierSize);
    NL_TEST_ASSERT(inSuite,
                   0 ==
                       memcmp(compressed_fabric_id_large_span.data(), kExpectedCompressedFabricIdentifier,
                              sizeof(kExpectedCompressedFabricIdentifier)));

    // Test smaller buffer than needed
    MutableByteSpan compressed_fabric_id_small_span(compressed_fabric_id, kCompressedFabricIdentifierSize - 1);
    error = GenerateCompressedFabricId(root_public_key, kFabricId, compressed_fabric_id_small_span);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test invalid public key
    const uint8_t kInvalidRootPublicKey[65] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    P256PublicKey invalid_root_public_key(kInvalidRootPublicKey);

    error = GenerateCompressedFabricId(invalid_root_public_key, kFabricId, compressed_fabric_id_span);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INVALID_ARGUMENT);
}

#if CHIP_CRYPTO_OPENSSL
static void TestX509_PKCS7Extraction(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int status     = 0;
    X509DerCertificate x509list[3];
    uint32_t max_certs = sizeof(x509list) / sizeof(X509DerCertificate);

    err = LoadCertsFromPKCS7(pem_pkcs7_blob, x509list, &max_certs);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    status = memcmp(certificate_blob_leaf, x509list[0], x509list[0].Length());
    NL_TEST_ASSERT(inSuite, status == 0);

    status = memcmp(certificate_blob_intermediate, x509list[1], x509list[1].Length());
    NL_TEST_ASSERT(inSuite, status == 0);

    status = memcmp(certificate_blob_root, x509list[2], x509list[2].Length());
    NL_TEST_ASSERT(inSuite, status == 0);
}
#endif // CHIP_CRYPTO_OPENSSL

static void TestPubkey_x509Extraction(nlTestSuite * inSuite, void * inContext)
{
    using namespace TestCerts;

    CHIP_ERROR err = CHIP_NO_ERROR;
    P256PublicKey publicKey;

    ByteSpan cert;
    const uint8_t * certPubkey;
    uint32_t certPubkeyLen;

    for (size_t i = 0; i < gNumTestCerts; i++)
    {
        uint8_t certType = TestCerts::gTestCerts[i];

        err = GetTestCert(certType, TestCertLoadFlags::kDERForm, cert);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = GetTestCertPubkey(certType, &certPubkey, certPubkeyLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = ExtractPubkeyFromX509Cert(cert, publicKey);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(publicKey.ConstBytes(), certPubkey, certPubkeyLen) == 0);
    }
}

/**
 *   Test Suite. It lists all the test functions.
 */

static const nlTest sTests[] = {

    NL_TEST_DEF("Test encrypting AES-CCM-128 test vectors", TestAES_CCM_128EncryptTestVectors),
    NL_TEST_DEF("Test decrypting AES-CCM-128 test vectors", TestAES_CCM_128DecryptTestVectors),
    NL_TEST_DEF("Test encrypting AES-CCM-128 invalid plain text", TestAES_CCM_128EncryptInvalidPlainText),
    NL_TEST_DEF("Test encrypting AES-CCM-128 using nil key", TestAES_CCM_128EncryptNilKey),
    NL_TEST_DEF("Test encrypting AES-CCM-128 using invalid IV", TestAES_CCM_128EncryptInvalidIVLen),
    NL_TEST_DEF("Test encrypting AES-CCM-128 using invalid tag", TestAES_CCM_128EncryptInvalidTagLen),
    NL_TEST_DEF("Test decrypting AES-CCM-128 invalid ct", TestAES_CCM_128DecryptInvalidCipherText),
    NL_TEST_DEF("Test decrypting AES-CCM-128 invalid key", TestAES_CCM_128DecryptInvalidKey),
    NL_TEST_DEF("Test decrypting AES-CCM-128 invalid IV", TestAES_CCM_128DecryptInvalidIVLen),
    NL_TEST_DEF("Test encrypting AES-CCM-256 test vectors", TestAES_CCM_256EncryptTestVectors),
    NL_TEST_DEF("Test decrypting AES-CCM-256 test vectors", TestAES_CCM_256DecryptTestVectors),
    NL_TEST_DEF("Test encrypting AES-CCM-256 invalid plain text", TestAES_CCM_256EncryptInvalidPlainText),
    NL_TEST_DEF("Test encrypting AES-CCM-256 using nil key", TestAES_CCM_256EncryptNilKey),
    NL_TEST_DEF("Test encrypting AES-CCM-256 using invalid IV", TestAES_CCM_256EncryptInvalidIVLen),
    NL_TEST_DEF("Test encrypting AES-CCM-256 using invalid tag", TestAES_CCM_256EncryptInvalidTagLen),
    NL_TEST_DEF("Test decrypting AES-CCM-256 invalid ct", TestAES_CCM_256DecryptInvalidCipherText),
    NL_TEST_DEF("Test decrypting AES-CCM-256 invalid key", TestAES_CCM_256DecryptInvalidKey),
    NL_TEST_DEF("Test decrypting AES-CCM-256 invalid IV", TestAES_CCM_256DecryptInvalidIVLen),
    NL_TEST_DEF("Test decrypting AES-CCM-256 invalid vectors", TestAES_CCM_256DecryptInvalidTestVectors),
    NL_TEST_DEF("Test ASN.1 signature conversion routines", TestAsn1Conversions),
    NL_TEST_DEF("Test Integer to ASN.1 DER conversion", TestRawIntegerToDerValidCases),
    NL_TEST_DEF("Test Integer to ASN.1 DER conversion error cases", TestRawIntegerToDerInvalidCases),
    NL_TEST_DEF("Test ECDSA signing and validation message using SHA256", TestECDSA_Signing_SHA256_Msg),
    NL_TEST_DEF("Test ECDSA signing and validation SHA256 Hash", TestECDSA_Signing_SHA256_Hash),
    NL_TEST_DEF("Test ECDSA signature validation fail - Different msg", TestECDSA_ValidationFailsDifferentMessage),
    NL_TEST_DEF("Test ECDSA signature validation fail - Different hash", TestECDSA_ValidationFailsDifferentHash),
    NL_TEST_DEF("Test ECDSA signature validation fail - Different msg signature", TestECDSA_ValidationFailIncorrectMsgSignature),
    NL_TEST_DEF("Test ECDSA signature validation fail - Different hash signature", TestECDSA_ValidationFailIncorrectHashSignature),
    NL_TEST_DEF("Test ECDSA sign msg invalid parameters", TestECDSA_SigningMsgInvalidParams),
    NL_TEST_DEF("Test ECDSA sign hash invalid parameters", TestECDSA_SigningHashInvalidParams),
    NL_TEST_DEF("Test ECDSA msg signature validation invalid parameters", TestECDSA_ValidationMsgInvalidParam),
    NL_TEST_DEF("Test ECDSA hash signature validation invalid parameters", TestECDSA_ValidationHashInvalidParam),
    NL_TEST_DEF("Test Hash SHA 256", TestHash_SHA256),
    NL_TEST_DEF("Test Hash SHA 256 Stream", TestHash_SHA256_Stream),
    NL_TEST_DEF("Test HKDF SHA 256", TestHKDF_SHA256),
    NL_TEST_DEF("Test HMAC SHA 256", TestHMAC_SHA256),
    NL_TEST_DEF("Test DRBG invalid inputs", TestDRBG_InvalidInputs),
    NL_TEST_DEF("Test DRBG output", TestDRBG_Output),
    NL_TEST_DEF("Test ECDH derive shared secret", TestECDH_EstablishSecret),
    NL_TEST_DEF("Test adding entropy sources", TestAddEntropySources),
    NL_TEST_DEF("Test PBKDF2 SHA256", TestPBKDF2_SHA256_TestVectors),
    NL_TEST_DEF("Test P256 Keygen", TestP256_Keygen),
    NL_TEST_DEF("Test CSR Generation", TestCSR_Gen),
    NL_TEST_DEF("Test Keypair Serialize", TestKeypair_Serialize),
    NL_TEST_DEF("Test Spake2p_spake2p FEMul", TestSPAKE2P_spake2p_FEMul),
    NL_TEST_DEF("Test Spake2p_spake2p FELoad/FEWrite", TestSPAKE2P_spake2p_FELoadWrite),
    NL_TEST_DEF("Test Spake2p_spake2p Mac", TestSPAKE2P_spake2p_Mac),
    NL_TEST_DEF("Test Spake2p_spake2p PointMul", TestSPAKE2P_spake2p_PointMul),
    NL_TEST_DEF("Test Spake2p_spake2p PointMulAdd", TestSPAKE2P_spake2p_PointMulAdd),
    NL_TEST_DEF("Test Spake2p_spake2p PointLoad/PointWrite", TestSPAKE2P_spake2p_PointLoadWrite),
    NL_TEST_DEF("Test Spake2p_spake2p PointIsValid", TestSPAKE2P_spake2p_PointIsValid),
    NL_TEST_DEF("Test Spake2+ against RFC test vectors", TestSPAKE2P_RFC),
    NL_TEST_DEF("Test compressed fabric identifier", TestCompressedFabricIdentifier),
    NL_TEST_DEF("Test Pubkey Extraction from x509 Certificate", TestPubkey_x509Extraction),
#if CHIP_CRYPTO_OPENSSL
    NL_TEST_DEF("Test x509 Certificate Extraction from PKCS7", TestX509_PKCS7Extraction),
#endif // CHIP_CRYPTO_OPENSSL
    NL_TEST_SENTINEL()
};

/**
 *  Set up the test suite.
 */
int TestCHIPCryptoPAL_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestCHIPCryptoPAL_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestCHIPCryptoPAL(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "CHIP Crypto PAL tests",
        &sTests[0],
        TestCHIPCryptoPAL_Setup,
        TestCHIPCryptoPAL_Teardown
    };
    // clang-format on
    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);

    add_entropy_source(test_entropy_source, nullptr, 16);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCHIPCryptoPAL)
