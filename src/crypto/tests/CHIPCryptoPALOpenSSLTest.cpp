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
#include "HKDF_SHA256_test_vectors.h"

#include <nlunit-test.h>
#include <stdio.h>
#include <string.h>
#include <support/CodeUtils.h>
#include <stdlib.h>

using namespace chip;
using namespace chip::Crypto;

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

            CHIP_ERROR err = AES_CCM_256_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->iv,
                                                 vector->iv_len, out_ct, out_tag, vector->tag_len);
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
            CHIP_ERROR err = AES_CCM_256_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag,
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

            CHIP_ERROR err = AES_CCM_256_encrypt(vector->pt, 0, vector->aad, vector->aad_len, vector->key, vector->iv,
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

            CHIP_ERROR err = AES_CCM_256_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, NULL, vector->iv,
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

            CHIP_ERROR err = AES_CCM_256_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->iv,
                                                 0, out_ct, out_tag, vector->tag_len);
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

            CHIP_ERROR err = AES_CCM_256_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->iv,
                                                 vector->iv_len, out_ct, out_tag, 13);
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
            CHIP_ERROR err = AES_CCM_256_decrypt(vector->ct, 0, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
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
            CHIP_ERROR err = AES_CCM_256_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag,
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
            CHIP_ERROR err = AES_CCM_256_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag,
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
            CHIP_ERROR err = AES_CCM_256_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag,
                                                 vector->tag_len, vector->key, vector->iv, vector->iv_len, out_pt);

            bool arePTsEqual = memcmp(vector->pt, out_pt, vector->pt_len) == 0;
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INTERNAL);
            NL_TEST_ASSERT(inSuite, arePTsEqual == false);
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestHKDF_SHA256(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases     = ArraySize(hkdf_sha256_test_vectors);
    int numOfTestsExecuted = 0;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hkdf_sha256_vector v = hkdf_sha256_test_vectors[numOfTestsExecuted];
        size_t out_length    = v.output_key_material_length;
        unsigned char out_buffer[out_length];
        HKDF_SHA256(v.initial_key_material, v.initial_key_material_length, v.salt, v.salt_length, v.info, v.info_length, out_buffer,
                    v.output_key_material_length);
        bool success = memcmp(v.output_key_material, out_buffer, out_length) == 0;
        NL_TEST_ASSERT(inSuite, success);
    }
    NL_TEST_ASSERT(inSuite, numOfTestsExecuted == 3);
}

static void TestDRBG_InvalidInputs(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    error            = DRBG_get_bytes(NULL, 10);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INVALID_ARGUMENT);
    error = CHIP_NO_ERROR;
    unsigned char buffer[5];
    error = DRBG_get_bytes(buffer, 0);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INVALID_ARGUMENT);
}

static void TestDRBG_Output(nlTestSuite * inSuite, void * inContext)
{
    // No good way to unit test a DRBG. Just validate that we get out something
    CHIP_ERROR error           = CHIP_ERROR_INVALID_ARGUMENT;
    unsigned char out_buf[10]  = { 0 };
    unsigned char orig_buf[10] = { 0 };

    error = DRBG_get_bytes(out_buf, sizeof(out_buf));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(out_buf, orig_buf, sizeof(out_buf)) != 0);
}

static void TestECDSA_Signing_SHA256(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen((const char *) msg);

    unsigned char hex_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                        0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                        0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

    unsigned char hex_public_key[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4,
                                       0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7,
                                       0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79,
                                       0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf,
                                       0x9f, 0xdc, 0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

    unsigned char signature[4096];
    size_t signature_length  = sizeof(signature);
    CHIP_ERROR signing_error = ECDSA_sign_msg((const unsigned char *) msg, msg_length, hex_private_key, sizeof(hex_private_key),
                                              signature, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    CHIP_ERROR validation_error = ECDSA_validate_msg_signature((const unsigned char *) msg, msg_length, hex_public_key,
                                                               sizeof(hex_public_key), signature, signature_length);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_NO_ERROR);
}

static void TestECDSA_ValidationFailsDifferentMessage(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen((const char *) msg);

    unsigned char hex_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                        0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                        0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

    unsigned char hex_public_key[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4,
                                       0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7,
                                       0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79,
                                       0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf,
                                       0x9f, 0xdc, 0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

    unsigned char signature[4096];
    size_t signature_length  = sizeof(signature);
    CHIP_ERROR signing_error = ECDSA_sign_msg((const unsigned char *) msg, msg_length, hex_private_key, sizeof(hex_private_key),
                                              signature, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    const char * diff_msg       = "NOT Hello World!";
    size_t diff_msg_length      = strlen((const char *) msg);
    CHIP_ERROR validation_error = ECDSA_validate_msg_signature((const unsigned char *) diff_msg, diff_msg_length, hex_public_key,
                                                               sizeof(hex_public_key), signature, signature_length);
    NL_TEST_ASSERT(inSuite, validation_error != CHIP_NO_ERROR);
}

static void TestECDSA_ValidationFailIncorrectSignature(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen((const char *) msg);

    unsigned char hex_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                        0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                        0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

    unsigned char hex_public_key[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4,
                                       0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7,
                                       0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79,
                                       0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf,
                                       0x9f, 0xdc, 0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

    unsigned char signature[4096];
    size_t signature_length  = sizeof(signature);
    CHIP_ERROR signing_error = ECDSA_sign_msg((const unsigned char *) msg, msg_length, hex_private_key, sizeof(hex_private_key),
                                              signature, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);
    signature[0] = ~signature[0]; // Flipping bits should invalidate the signature.

    CHIP_ERROR validation_error = ECDSA_validate_msg_signature((const unsigned char *) msg, msg_length, hex_public_key,
                                                               sizeof(hex_public_key), signature, signature_length);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_SIGNATURE);
}

static void TestECDSA_SigningInvalidParams(nlTestSuite * inSuite, void * inContext)
{
    const unsigned char * msg       = (unsigned char *) "Hello World!";
    size_t msg_length               = strlen((const char *) msg);
    unsigned char hex_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                        0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                        0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

    unsigned char signature[4096];
    size_t signature_length = sizeof(signature);
    CHIP_ERROR signing_error =
        ECDSA_sign_msg(NULL, msg_length, hex_private_key, sizeof(hex_private_key), signature, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    signing_error = ECDSA_sign_msg(msg, 0, hex_private_key, sizeof(hex_private_key), signature, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    signing_error = ECDSA_sign_msg(msg, msg_length, NULL, sizeof(hex_private_key), signature, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    signing_error = ECDSA_sign_msg(msg, msg_length, hex_private_key, 0, signature, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    signing_error = ECDSA_sign_msg(msg, msg_length, hex_private_key, sizeof(hex_private_key), NULL, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;
}

static void TestECDSA_ValidationInvalidParam(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen((const char *) msg);

    unsigned char hex_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                        0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                        0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

    unsigned char hex_public_key[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4,
                                       0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7,
                                       0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79,
                                       0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf,
                                       0x9f, 0xdc, 0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

    unsigned char signature[4096];
    size_t signature_length  = sizeof(signature);
    CHIP_ERROR signing_error = ECDSA_sign_msg((const unsigned char *) msg, msg_length, hex_private_key, sizeof(hex_private_key),
                                              signature, signature_length);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    CHIP_ERROR validation_error =
        ECDSA_validate_msg_signature(NULL, msg_length, hex_public_key, sizeof(hex_public_key), signature, signature_length);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;

    validation_error = ECDSA_validate_msg_signature((const unsigned char *) msg, 0, hex_public_key, sizeof(hex_public_key),
                                                    signature, signature_length);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;


    validation_error =
        ECDSA_validate_msg_signature((const unsigned char *) msg, msg_length, hex_public_key, 0, signature, signature_length);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;

    validation_error = ECDSA_validate_msg_signature((const unsigned char *) msg, msg_length, hex_public_key, sizeof(hex_public_key),
                                                    NULL, signature_length);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;

    validation_error =
        ECDSA_validate_msg_signature((const unsigned char *) msg, msg_length, hex_public_key, sizeof(hex_public_key), signature, 0);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;
}

/**
 *   Test Suite. It lists all the test functions.
 */

static const nlTest sTests[] = {
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
    NL_TEST_DEF("Test HKDF SHA 256", TestHKDF_SHA256), NL_TEST_DEF("Test DRBG invalid inputs", TestDRBG_InvalidInputs),
    NL_TEST_DEF("Test DRBG output", TestDRBG_Output),
    NL_TEST_DEF("Test ECDSA signing and validation using SHA256", TestECDSA_Signing_SHA256),
    NL_TEST_DEF("Test ECDSA signature validation fail - Different msg", TestECDSA_ValidationFailsDifferentMessage),
    NL_TEST_DEF("Test ECDSA signature validation fail - Different signature", TestECDSA_ValidationFailIncorrectSignature),
    NL_TEST_DEF("Test ECDSA sign msg invalid parameters", TestECDSA_SigningInvalidParams),
    NL_TEST_DEF("Test ECDSA signature validation invalid parameters", TestECDSA_ValidationInvalidParam),

    NL_TEST_SENTINEL()
};

int main(void)
{
    nlTestSuite theSuite = { "CHIP Crypto PAL tests", &sTests[0], NULL, NULL };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
