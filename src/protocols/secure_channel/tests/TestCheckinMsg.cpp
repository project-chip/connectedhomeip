/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <protocols/secure_channel/CheckinMessage.h>

#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <transport/CryptoContext.h>

#include <crypto/tests/AES_CCM_128_test_vectors.h>

#include <crypto/RandUtils.h>

#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::Protocols::SecureChannel;
using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

void TestCheckin_Generate(nlTestSuite * inSuite, void * inContext)
{
    uint8_t a[300] = { 0 };
    uint8_t b[300] = { 0 };
    MutableByteSpan outputBuffer{ a };
    MutableByteSpan oldOutputBuffer{ b };
    uint32_t counter = 0;
    ByteSpan userData;
    CHIP_ERROR err = CHIP_NO_ERROR;
    TestSessionKeystoreImpl keystore;

    // Verify that keys imported to the keystore behave as expected.
    for (const ccm_128_test_vector * testPtr : ccm_128_test_vectors)
    {
        const ccm_128_test_vector & test = *testPtr;

        Aes128KeyByteArray keyMaterial;
        memcpy(keyMaterial, test.key, test.key_len);

        Aes128KeyHandle keyHandle;
        NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(keyMaterial, keyHandle));

        // Validate that counter change, indeed changes the output buffer content
        counter = 0;
        for (uint8_t j = 0; j < 5; j++)
        {
            err = CheckinMessage::GenerateCheckinMessagePayload(keyHandle, counter, userData, outputBuffer);
            NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

            // Verifiy that the output buffer changed
            NL_TEST_ASSERT(inSuite, !outputBuffer.data_equal(oldOutputBuffer));
            CopySpanToMutableSpan(outputBuffer, oldOutputBuffer);

            // Increment by a random count. On the slim changes the increment is 0 add 1 to change output buffer
            counter += chip::Crypto::GetRandU32() + 1;
            outputBuffer = MutableByteSpan(a);
        }
        keystore.DestroyKey(keyHandle);
    }

    // Parameter check
    {
        uint8_t data[]                                               = { "This is some user Data. It should be encrypted" };
        userData                                                     = chip::ByteSpan(data);
        const ccm_128_test_vector & test                             = *ccm_128_test_vectors[0];
        uint8_t gargantuaBuffer[2 * CheckinMessage::sMaxAppDataSize] = { 0 };

        Aes128KeyByteArray keyMaterial;
        memcpy(keyMaterial, test.key, test.key_len);

        Aes128KeyHandle keyHandle;
        NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(keyMaterial, keyHandle));

        // As of now passing an empty key handle while using PSA crypto will result in a failure.
        // However when using OpenSSL this same test result in a success.
        // Issue #28986

        // Aes128KeyHandle emptyKeyHandle;
        // err = CheckinMessage::GenerateCheckinMessagePayload(emptyKeyHandle, counter, userData, outputBuffer);
        // ChipLogError(Inet, "%s", err.AsString());
        // NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

        ByteSpan emptyData;
        err = CheckinMessage::GenerateCheckinMessagePayload(keyHandle, counter, emptyData, outputBuffer);
        NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

        MutableByteSpan empty;
        err = CheckinMessage::GenerateCheckinMessagePayload(keyHandle, counter, emptyData, empty);
        NL_TEST_ASSERT(inSuite, (CHIP_ERROR_INVALID_ARGUMENT == err));

        userData = chip::ByteSpan(gargantuaBuffer, sizeof(gargantuaBuffer));
        err      = CheckinMessage::GenerateCheckinMessagePayload(keyHandle, counter, userData, outputBuffer);
        NL_TEST_ASSERT(inSuite, (CHIP_ERROR_INVALID_ARGUMENT == err));

        // Cleanup
        keystore.DestroyKey(keyHandle);
    }
}

void TestCheckin_Parse(nlTestSuite * inSuite, void * inContext)
{
    uint8_t a[300] = { 0 };
    uint8_t b[300] = { 0 };
    MutableByteSpan outputBuffer{ a };
    MutableByteSpan buffer{ b };
    uint32_t counter = 0, decryptedCounter;
    ByteSpan userData;

    CHIP_ERROR err = CHIP_NO_ERROR;

    TestSessionKeystoreImpl keystore;

    // Verify User Data Encryption Decryption
    uint8_t data[]                   = { "This is some user Data. It should be encrypted" };
    userData                         = chip::ByteSpan(data);
    const ccm_128_test_vector & test = *ccm_128_test_vectors[0];

    Aes128KeyByteArray keyMaterial;
    memcpy(keyMaterial, test.key, test.key_len);

    Aes128KeyHandle keyHandle;
    NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(keyMaterial, keyHandle));

    //=================Encrypt=======================

    err              = CheckinMessage::GenerateCheckinMessagePayload(keyHandle, counter, userData, outputBuffer);
    ByteSpan payload = chip::ByteSpan(outputBuffer.data(), outputBuffer.size());
    NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

    //=================Decrypt=======================

    MutableByteSpan empty;
    err = CheckinMessage::ParseCheckinMessagePayload(keyHandle, payload, decryptedCounter, empty);
    NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR != err));

    ByteSpan emptyPayload;
    err = CheckinMessage::ParseCheckinMessagePayload(keyHandle, emptyPayload, decryptedCounter, buffer);
    NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR != err));
}

void TestCheckin_GenerateParse(nlTestSuite * inSuite, void * inContext)
{
    uint8_t a[300] = { 0 };
    uint8_t b[300] = { 0 };
    MutableByteSpan outputBuffer{ a };
    MutableByteSpan buffer{ b };
    uint32_t counter = 0xDEADBEEF;
    ByteSpan userData;

    CHIP_ERROR err = CHIP_NO_ERROR;

    TestSessionKeystoreImpl keystore;

    // Verify User Data Encryption Decryption
    uint8_t data[] = { "This is some user Data. It should be encrypted" };
    userData       = chip::ByteSpan(data);
    for (const ccm_128_test_vector * testPtr : ccm_128_test_vectors)
    {
        const ccm_128_test_vector & test = *testPtr;

        Aes128KeyByteArray keyMaterial;
        memcpy(keyMaterial, test.key, test.key_len);

        Aes128KeyHandle keyHandle;
        NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(keyMaterial, keyHandle));

        //=================Encrypt=======================

        err = CheckinMessage::GenerateCheckinMessagePayload(keyHandle, counter, userData, outputBuffer);
        NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

        //=================Decrypt=======================
        uint32_t decryptedCounter = 0;
        ByteSpan payload          = chip::ByteSpan(outputBuffer.data(), outputBuffer.size());

        err = CheckinMessage::ParseCheckinMessagePayload(keyHandle, payload, decryptedCounter, buffer);
        NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

        NL_TEST_ASSERT(inSuite, (memcmp(data, buffer.data(), sizeof(data)) == 0));
        NL_TEST_ASSERT(inSuite, (counter == decryptedCounter));

        // reset buffers
        memset(a, 0, sizeof(a));
        memset(b, 0, sizeof(b));
        outputBuffer = MutableByteSpan(a);
        buffer       = MutableByteSpan(b);

        counter += chip::Crypto::GetRandU32() + 1;
        keystore.DestroyKey(keyHandle);
    }
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestCheckin_Generate", TestCheckin_Generate),
    NL_TEST_DEF("TestCheckin_Parse", TestCheckin_Parse),
    NL_TEST_DEF("TestCheckin_GenerateParse", TestCheckin_GenerateParse),

    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-Checkin-Message",
    &sTests[0],
    TestSetup,
    TestTeardown,
};
// clang-format on

/**
 *  Main
 */
int TestCheckinMessage()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCheckinMessage)
