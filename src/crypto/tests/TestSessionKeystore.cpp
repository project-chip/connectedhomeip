/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AES_CCM_128_test_vectors.h"

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

#if CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif

using namespace chip;
using namespace chip::Crypto;

namespace {

using TestSessionKeystoreImpl = DefaultSessionKeystore;

struct DeriveKeyTestVector
{
    // KDF parameters
    const char * secret;
    const char * salt;
    const char * info;
    // AES CTR input
    uint8_t plaintext[16];
    uint8_t nonce[13];
    // Expected AES CTR output
    uint8_t ciphertext[16];
};

struct DeriveSessionKeysTestVector
{
    // KDF parameters
    const char * secret;
    const char * salt;
    const char * info;
    // AES CTR input
    uint8_t plaintext[16];
    uint8_t nonce[13];
    // Expected AES CTR output
    uint8_t i2rCiphertext[16];
    uint8_t r2iCiphertext[16];
    uint8_t attestationChallenge[16];
};

DeriveKeyTestVector deriveKeyTestVectors[] = {
    {
        .secret = "secret",
        .salt   = "salt123",
        .info   = "info123",
        // derived key: a134e284e8628486f4d620a711f3cb50
        .plaintext  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
        .nonce      = { 0 },
        .ciphertext = { 0x62, 0x1b, 0x8d, 0x7a, 0x6f, 0xea, 0x7f, 0xca, 0x03, 0x64, 0x21, 0xb4, 0x3c, 0xbc, 0xa9, 0xbb },
    },
};

DeriveSessionKeysTestVector deriveSessionKeysTestVectors[] = {
    {
        .secret = "secret",
        .salt   = "salt123",
        .info   = "info123",
        // derived keys: a134e284e8628486f4d620a711f3cb50
        //               8a84a74c1550cf1dc57e5f8a099dcf37
        //               739184dd1465856473706661f5116be5
        .plaintext            = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
        .nonce                = { 0 },
        .i2rCiphertext        = { 0x62, 0x1b, 0x8d, 0x7a, 0x6f, 0xea, 0x7f, 0xca, 0x03, 0x64, 0x21, 0xb4, 0x3c, 0xbc, 0xa9, 0xbb },
        .r2iCiphertext        = { 0x65, 0x90, 0xf8, 0xab, 0x85, 0x55, 0x02, 0xcf, 0x87, 0xc5, 0xd9, 0x45, 0x75, 0xcd, 0xdb, 0x01 },
        .attestationChallenge = { 0x73, 0x91, 0x84, 0xdd, 0x14, 0x65, 0x85, 0x64, 0x73, 0x70, 0x66, 0x61, 0xf5, 0x11, 0x6b, 0xe5 },
    },
};

ByteSpan ToSpan(const char * str)
{
    return ByteSpan(reinterpret_cast<const uint8_t *>(str), strlen(str));
}

void TestBasicImport(nlTestSuite * inSuite, void * inContext)
{
    TestSessionKeystoreImpl keystore;

    // Verify that keys imported to the keystore behave as expected.
    for (const ccm_128_test_vector * testPtr : ccm_128_test_vectors)
    {
        const ccm_128_test_vector & test = *testPtr;

        Symmetric128BitsKeyByteArray keyMaterial;
        memcpy(keyMaterial, test.key, test.key_len);

        Aes128KeyHandle keyHandle;
        NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(keyMaterial, keyHandle));

        Platform::ScopedMemoryBuffer<uint8_t> ciphertext;
        Platform::ScopedMemoryBuffer<uint8_t> tag;
        NL_TEST_ASSERT(inSuite, ciphertext.Alloc(test.ct_len));
        NL_TEST_ASSERT(inSuite, tag.Alloc(test.tag_len));
        NL_TEST_ASSERT(inSuite,
                       AES_CCM_encrypt(test.pt, test.pt_len, test.aad, test.aad_len, keyHandle, test.nonce, test.nonce_len,
                                       ciphertext.Get(), tag.Get(), test.tag_len) == test.result);
        NL_TEST_ASSERT(inSuite, memcmp(ciphertext.Get(), test.ct, test.ct_len) == 0);
        NL_TEST_ASSERT(inSuite, memcmp(tag.Get(), test.tag, test.tag_len) == 0);

        keystore.DestroyKey(keyHandle);
    }
}

void TestDeriveKey(nlTestSuite * inSuite, void * inContext)
{
    TestSessionKeystoreImpl keystore;

    for (const DeriveKeyTestVector & test : deriveKeyTestVectors)
    {
        P256ECDHDerivedSecret secret;
        memcpy(secret.Bytes(), test.secret, strlen(test.secret));
        secret.SetLength(strlen(test.secret));

        Aes128KeyHandle keyHandle;
        NL_TEST_ASSERT_SUCCESS(inSuite, keystore.DeriveKey(secret, ToSpan(test.salt), ToSpan(test.info), keyHandle));

        uint8_t ciphertext[sizeof(test.ciphertext)];
        NL_TEST_ASSERT_SUCCESS(
            inSuite, AES_CTR_crypt(test.plaintext, sizeof(test.plaintext), keyHandle, test.nonce, sizeof(test.nonce), ciphertext));
        NL_TEST_ASSERT(inSuite, memcmp(ciphertext, test.ciphertext, sizeof(test.ciphertext)) == 0);

        keystore.DestroyKey(keyHandle);
    }
}

void TestDeriveSessionKeys(nlTestSuite * inSuite, void * inContext)
{
    TestSessionKeystoreImpl keystore;

    for (const DeriveSessionKeysTestVector & test : deriveSessionKeysTestVectors)
    {
        P256ECDHDerivedSecret secret;
        memcpy(secret.Bytes(), test.secret, strlen(test.secret));
        secret.SetLength(strlen(test.secret));

        Aes128KeyHandle i2r;
        Aes128KeyHandle r2i;
        AttestationChallenge challenge;
        NL_TEST_ASSERT_SUCCESS(
            inSuite, keystore.DeriveSessionKeys(ToSpan(test.secret), ToSpan(test.salt), ToSpan(test.info), i2r, r2i, challenge));

        uint8_t ciphertext[sizeof(test.i2rCiphertext)];

        // Test I2R key
        NL_TEST_ASSERT_SUCCESS(
            inSuite, AES_CTR_crypt(test.plaintext, sizeof(test.plaintext), i2r, test.nonce, sizeof(test.nonce), ciphertext));
        NL_TEST_ASSERT(inSuite, memcmp(ciphertext, test.i2rCiphertext, sizeof(test.i2rCiphertext)) == 0);

        // Test R2I key
        NL_TEST_ASSERT_SUCCESS(
            inSuite, AES_CTR_crypt(test.plaintext, sizeof(test.plaintext), r2i, test.nonce, sizeof(test.nonce), ciphertext));
        NL_TEST_ASSERT(inSuite, memcmp(ciphertext, test.r2iCiphertext, sizeof(test.r2iCiphertext)) == 0);

        // Check attestation challenge
        NL_TEST_ASSERT(inSuite, memcmp(challenge.Bytes(), test.attestationChallenge, sizeof(test.attestationChallenge)) == 0);

        keystore.DestroyKey(i2r);
        keystore.DestroyKey(r2i);
    }
}

const nlTest sTests[] = { NL_TEST_DEF("Test basic import", TestBasicImport), NL_TEST_DEF("Test derive key", TestDeriveKey),
                          NL_TEST_DEF("Test derive session keys", TestDeriveSessionKeys), NL_TEST_SENTINEL() };

int Test_Setup(void * inContext)
{
    CHIP_ERROR error = Platform::MemoryInit();
    VerifyOrReturnError(error == CHIP_NO_ERROR, FAILURE);

#if CHIP_CRYPTO_PSA
    psa_crypto_init();
#endif

    return SUCCESS;
}

int Test_Teardown(void * inContext)
{
    Platform::MemoryShutdown();

    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestSessionKeystore()
{
    nlTestSuite theSuite = { "SessionKeystore tests", &sTests[0], Test_Setup, Test_Teardown };

    // Run test suite againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSessionKeystore)
