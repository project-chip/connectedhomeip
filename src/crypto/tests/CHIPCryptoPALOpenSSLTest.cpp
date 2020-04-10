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

#include "CHIPCryptoPAL.h"
#include "AES_CCM_256_test_vectors.h"

#include <nlunit-test.h>
#include <stdio.h>
#include <string.h>
#include <support/CodeUtils.h>

static void TestAES_CCM_256EncryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            unsigned char out_ct[vector->ct_len];
            unsigned char out_tag[vector->tag_len];

            CHIP_ERROR err    = CHIP_aes_ccm_256_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key,
                                                      vector->iv, vector->iv_len, out_ct, out_tag, vector->tag_len);
            bool areCTsEqual  = memcmp(out_ct, vector->ct, vector->ct_len) == 0;
            bool areTagsEqual = memcmp(out_tag, vector->tag, vector->tag_len) == 0;
            NL_TEST_ASSERT(inSuite, areCTsEqual);
            NL_TEST_ASSERT(inSuite, areTagsEqual);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            if (!areCTsEqual)
            {
                printf("\n Test %d failed due to mismatchinng ciphertext", vector->tcId);
            }
            if (!areTagsEqual)
            {
                printf("\n Test %d failed due to mismatchinng tags", vector->tcId);
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            unsigned char out_pt[vector->pt_len];
            CHIP_ERROR err = CHIP_aes_ccm_256_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag,
                                                      vector->tag_len, vector->key, vector->iv, vector->iv_len, out_pt);

            bool arePTsEqual = memcmp(vector->pt, out_pt, vector->pt_len) == 0;
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, arePTsEqual);
            if (!arePTsEqual)
            {
                printf("\n Test %d failed due to mismatchinng plaintext", vector->tcId);
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptInvalidPlainText(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            unsigned char out_ct[vector->ct_len];
            unsigned char out_tag[vector->tag_len];

            CHIP_ERROR err = CHIP_aes_ccm_256_encrypt(vector->pt, 0, vector->aad, vector->aad_len, vector->key, vector->iv,
                                                      vector->iv_len, out_ct, out_tag, vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 1);
}

static void TestAES_CCM_256EncryptNilKey(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {

            unsigned char out_ct[vector->ct_len];
            unsigned char out_tag[vector->tag_len];

            CHIP_ERROR err = CHIP_aes_ccm_256_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, NULL, vector->iv,
                                                      vector->iv_len, out_ct, out_tag, vector->tag_len);
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
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {

            unsigned char out_ct[vector->ct_len];
            unsigned char out_tag[vector->tag_len];

            CHIP_ERROR err = CHIP_aes_ccm_256_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key,
                                                      vector->iv, 0, out_ct, out_tag, vector->tag_len);
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
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {

            unsigned char out_ct[vector->ct_len];
            unsigned char out_tag[vector->tag_len];

            CHIP_ERROR err = CHIP_aes_ccm_256_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key,
                                                      vector->iv, vector->iv_len, out_ct, out_tag, 13);
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
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {

            unsigned char out_pt[vector->pt_len];
            CHIP_ERROR err = CHIP_aes_ccm_256_decrypt(vector->ct, 0, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                                      vector->key, vector->iv, vector->iv_len, out_pt);
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
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {

            unsigned char out_pt[vector->pt_len];
            CHIP_ERROR err = CHIP_aes_ccm_256_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag,
                                                      vector->tag_len, NULL, vector->iv, vector->iv_len, out_pt);
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
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {

            unsigned char out_pt[vector->pt_len];
            CHIP_ERROR err = CHIP_aes_ccm_256_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag,
                                                      vector->tag_len, vector->key, vector->iv, 0, out_pt);
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
    for (numOfTestsRan = 0; numOfTestsRan < numOfTestVectors; numOfTestsRan++)
    {
        const ccm_test_vector * vector = ccm_invalid_test_vectors[numOfTestsRan];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            unsigned char out_pt[vector->pt_len];
            CHIP_ERROR err = CHIP_aes_ccm_256_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag,
                                                      vector->tag_len, vector->key, vector->iv, vector->iv_len, out_pt);

            bool arePTsEqual = memcmp(vector->pt, out_pt, vector->pt_len) == 0;
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INTERNAL);
            NL_TEST_ASSERT(inSuite, arePTsEqual == false);
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Test encrypting test vectors", TestAES_CCM_256EncryptTestVectors),
    NL_TEST_DEF("Test decrypting test vectors", TestAES_CCM_256DecryptTestVectors),
    NL_TEST_DEF("Test encrypting invalid plain text", TestAES_CCM_256EncryptInvalidPlainText),
    NL_TEST_DEF("Test encrypting using nil key", TestAES_CCM_256EncryptNilKey),
    NL_TEST_DEF("Test encrypting using invalid IV", TestAES_CCM_256EncryptInvalidIVLen),
    NL_TEST_DEF("Test encrypting using invalid tag", TestAES_CCM_256EncryptInvalidTagLen),
    NL_TEST_DEF("Test decrypting invalid ct", TestAES_CCM_256DecryptInvalidCipherText),
    NL_TEST_DEF("Test decrypting invalid key", TestAES_CCM_256DecryptInvalidKey), 
    NL_TEST_DEF("Test decrypting invalid IV", TestAES_CCM_256DecryptInvalidIVLen),
    NL_TEST_DEF("Test decrypting invalid vectors", TestAES_CCM_256DecryptInvalidTestVectors), 
    NL_TEST_SENTINEL()
};

int main(void)
{
    nlTestSuite theSuite =
	{
        "CHIP Crypto PAL tests",
        &sTests[0],
        NULL,
        NULL
    };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
