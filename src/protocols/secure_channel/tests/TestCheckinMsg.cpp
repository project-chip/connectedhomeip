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

#include <crypto/DefaultSessionKeystore.h>
#include <crypto/RandUtils.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <protocols/secure_channel/tests/CheckIn_Message_test_vectors.h>
#include <transport/CryptoContext.h>
// AES_CCM_128_test_vectors is being replaced by the CheckIn_Message_test_vectors
// New tests need to use the CheckIn_Message_test_vectors
#include <crypto/tests/AES_CCM_128_test_vectors.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::Protocols::SecureChannel;
using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

namespace chip {
namespace Protocols {
namespace SecureChannel {

class TestCheckInMsg
{
public:
    static void TestCheckinMessageGenerate_ValidInputsSameSizeOutputAsPayload(nlTestSuite * inSuite, void * inContext);
    static void TestCheckinMessageGenerate_ValidInputsBiggerSizeOutput(nlTestSuite * inSuite, void * inContext);
    static void TestCheckInMessagePayloadSize(nlTestSuite * inSuite, void * inContext);
    static void TestCheckInMessagePayloadSizeNullBuffer(nlTestSuite * inSuite, void * inContext);
    static void TestCheckinMessageGenerate_ValidInputsTooSmallOutput(nlTestSuite * inSuite, void * inContext);
    static void TestCheckInMessageGenerate_EmptyAesKeyHandle(nlTestSuite * inSuite, void * inContext);
    static void TestCheckInMessageGenerate_EmptyHmacKeyHandle(nlTestSuite * inSuite, void * inContext);

    static void TestCheckinParse(nlTestSuite * inSuite, void * inContext);
    static void TestCheckinGenerateParse(nlTestSuite * inSuite, void * inContext);

private:
    static CHIP_ERROR GenerateAndVerifyPayload(nlTestSuite * inSuite, MutableByteSpan & output,
                                               const CheckIn_Message_test_vector & vector);
};

/**
 * @brief Helper function that generates the Check-In message based on the test vector
 *        and verifies the generated Check-In message
 *        Helper is to avoid having the same code three times in different tests
 *
 * @return CHIP_NO_ERROR if the generation was successful
 *         error code if the generation failed - see GenerateCheckinMessagePayload
 */
CHIP_ERROR TestCheckInMsg::GenerateAndVerifyPayload(nlTestSuite * inSuite, MutableByteSpan & output,
                                                    const CheckIn_Message_test_vector & vector)
{
    TestSessionKeystoreImpl keystore;

    // Two distinct key material buffers to ensure crypto-hardware-assist with single-usage keys create two different handles.
    Symmetric128BitsKeyByteArray aesKeyMaterial;
    memcpy(aesKeyMaterial, vector.key, vector.key_len);

    Symmetric128BitsKeyByteArray hmacKeyMaterial;
    memcpy(hmacKeyMaterial, vector.key, vector.key_len);

    Aes128KeyHandle aes128KeyHandle;
    NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(aesKeyMaterial, aes128KeyHandle));

    Hmac128KeyHandle hmac128KeyHandle;
    NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(hmacKeyMaterial, hmac128KeyHandle));

    // Create application data ByteSpan
    ByteSpan applicationData(vector.application_data, vector.application_data_len);

    // Verify that the generation succeeded
    ReturnErrorOnFailure(
        CheckinMessage::GenerateCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, vector.counter, applicationData, output));

    // Validate Full payload
    NL_TEST_ASSERT_EQUALS(inSuite, output.size(), vector.payload_len);
    NL_TEST_ASSERT(inSuite, (memcmp(vector.payload, output.data(), output.size()) == 0));

    size_t cursorIndex = 0;

    // Validate Nonce
    MutableByteSpan nonce = output.SubSpan(cursorIndex, vector.nonce_len);
    NL_TEST_ASSERT(inSuite, (memcmp(vector.nonce, nonce.data(), nonce.size()) == 0));
    cursorIndex += nonce.size();

    // Validate ciphertext
    MutableByteSpan ciphertext = output.SubSpan(cursorIndex, vector.ciphertext_len);
    NL_TEST_ASSERT(inSuite, (memcmp(vector.ciphertext, ciphertext.data(), ciphertext.size()) == 0));
    cursorIndex += ciphertext.size();

    // Validate MIC
    MutableByteSpan mic = output.SubSpan(cursorIndex, vector.mic_len);
    NL_TEST_ASSERT(inSuite, (memcmp(vector.mic, mic.data(), mic.size()) == 0));
    cursorIndex += mic.size();

    // Clean up
    keystore.DestroyKey(aes128KeyHandle);
    keystore.DestroyKey(hmac128KeyHandle);

    return CHIP_NO_ERROR;
}

/**
 * @brief Test verifies that the Check-In message generation is successful when using an output size equal to the payload size
 */
void TestCheckInMsg::TestCheckinMessageGenerate_ValidInputsSameSizeOutputAsPayload(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases = ArraySize(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        // Create output buffer
        uint8_t buffer[300] = { 0 };
        MutableByteSpan output(buffer, sizeof(buffer));

        // Force output buffer to the payload size
        output.reduce_size(vector.payload_len);

        NL_TEST_ASSERT_SUCCESS(inSuite, GenerateAndVerifyPayload(inSuite, output, vector));
    }
}

/**
 * @brief Test verifies that the Check-In message generation is successful when using an output size greater than the payload size
 */
void TestCheckInMsg::TestCheckinMessageGenerate_ValidInputsBiggerSizeOutput(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases = ArraySize(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        // Create output buffer
        uint8_t buffer[300] = { 0 };
        MutableByteSpan output(buffer, sizeof(buffer));

        NL_TEST_ASSERT_SUCCESS(inSuite, GenerateAndVerifyPayload(inSuite, output, vector));
    }
}

/**
 * @brief Test verifies that the Check-In message generation returns an error if the output buffer is too small
 */
void TestCheckInMsg::TestCheckinMessageGenerate_ValidInputsTooSmallOutput(nlTestSuite * inSuite, void * inContext)
{
    CheckIn_Message_test_vector vector = checkIn_message_test_vectors[0];

    // Create output buffer with 0 size
    MutableByteSpan output;

    NL_TEST_ASSERT(inSuite, CHIP_ERROR_BUFFER_TOO_SMALL == GenerateAndVerifyPayload(inSuite, output, vector));
}

/**
 * @brief Test verifies that the Check-In Message generations returns an error if the AesKeyHandle is empty
 */
void TestCheckInMsg::TestCheckInMessageGenerate_EmptyAesKeyHandle(nlTestSuite * inSuite, void * inContexT)
{
    TestSessionKeystoreImpl keystore;
    CheckIn_Message_test_vector vector = checkIn_message_test_vectors[0];

    // Create output buffer
    uint8_t buffer[300] = { 0 };
    MutableByteSpan output(buffer, sizeof(buffer));

    // Force output buffer to the payload size
    output.reduce_size(vector.payload_len);

    // Empty AES Key handle
    Aes128KeyHandle aes128KeyHandle;

    Symmetric128BitsKeyByteArray hmacKeyMaterial;
    memcpy(hmacKeyMaterial, vector.key, vector.key_len);

    Hmac128KeyHandle hmac128KeyHandle;
    NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(hmacKeyMaterial, hmac128KeyHandle));

    // Create application data ByteSpan
    ByteSpan applicationData(vector.application_data, vector.application_data_len);

    /*
        Passing an empty key handle while using PSA crypto will result in a failure.
        When using OpenSSL this same test result in a success.
        Issue #28986

    Verify that the generation fails with an empty key handle
    NL_TEST_ASSERT_(inSuite,
                    CHIP_NO_ERROR !=
                        CheckinMessage::GenerateCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, vector.counter,
                                                                      applicationData, output));
    */

    // Clean up
    keystore.DestroyKey(hmac128KeyHandle);
}

/**
 * @brief Test verifies that the Check-In Message generations returns an error if the HmacKeyHandle is empty
 */
void TestCheckInMsg::TestCheckInMessageGenerate_EmptyHmacKeyHandle(nlTestSuite * inSuite, void * inContexT)
{
    TestSessionKeystoreImpl keystore;
    CheckIn_Message_test_vector vector = checkIn_message_test_vectors[0];

    // Create output buffer
    uint8_t buffer[300] = { 0 };
    MutableByteSpan output(buffer, sizeof(buffer));

    // Force output buffer to the payload size
    output.reduce_size(vector.payload_len);

    Symmetric128BitsKeyByteArray aesKeyMaterial;
    memcpy(aesKeyMaterial, vector.key, vector.key_len);

    Aes128KeyHandle aes128KeyHandle;
    NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(aesKeyMaterial, aes128KeyHandle));

    Hmac128KeyHandle hmac128KeyHandle;

    // Create application data ByteSpan
    ByteSpan applicationData(vector.application_data, vector.application_data_len);

    /*
        Passing an empty key handle while using PSA crypto will result in a failure.
        When using OpenSSL this same test result in a success.
        Issue #28986

    Verify that the generation fails with an empty key handle
    NL_TEST_ASSERT_(inSuite,
                    CHIP_NO_ERROR !=
                        CheckinMessage::GenerateCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, vector.counter,
                                                                      applicationData, output));
    */

    // Clean up
    keystore.DestroyKey(aes128KeyHandle);
}

void TestCheckInMsg::TestCheckinParse(nlTestSuite * inSuite, void * inContext)
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

    // Two distinct key material buffers to ensure crypto-hardware-assist with single-usage keys create two different handles.
    Symmetric128BitsKeyByteArray aesKeyMaterial;
    memcpy(aesKeyMaterial, test.key, test.key_len);

    Symmetric128BitsKeyByteArray hmacKeyMaterial;
    memcpy(hmacKeyMaterial, test.key, test.key_len);

    Aes128KeyHandle aes128KeyHandle;
    NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(aesKeyMaterial, aes128KeyHandle));

    Hmac128KeyHandle hmac128KeyHandle;
    NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(hmacKeyMaterial, hmac128KeyHandle));

    //=================Encrypt=======================

    err = CheckinMessage::GenerateCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, counter, userData, outputBuffer);
    ByteSpan payload = chip::ByteSpan(outputBuffer.data(), outputBuffer.size());
    NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

    //=================Decrypt=======================

    MutableByteSpan empty;
    err = CheckinMessage::ParseCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, payload, decryptedCounter, empty);
    NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR != err));

    ByteSpan emptyPayload;
    err = CheckinMessage::ParseCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, emptyPayload, decryptedCounter, buffer);
    NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR != err));

    // Cleanup
    keystore.DestroyKey(aes128KeyHandle);
    keystore.DestroyKey(hmac128KeyHandle);
}

void TestCheckInMsg::TestCheckinGenerateParse(nlTestSuite * inSuite, void * inContext)
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

        // Two disctint key material to force the PSA unit tests to create two different Key IDs
        Symmetric128BitsKeyByteArray aesKeyMaterial;
        memcpy(aesKeyMaterial, test.key, test.key_len);

        Symmetric128BitsKeyByteArray hmacKeyMaterial;
        memcpy(hmacKeyMaterial, test.key, test.key_len);

        Aes128KeyHandle aes128KeyHandle;
        NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(aesKeyMaterial, aes128KeyHandle));

        Hmac128KeyHandle hmac128KeyHandle;
        NL_TEST_ASSERT_SUCCESS(inSuite, keystore.CreateKey(hmacKeyMaterial, hmac128KeyHandle));

        //=================Encrypt=======================

        err = CheckinMessage::GenerateCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, counter, userData, outputBuffer);
        NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

        //=================Decrypt=======================
        uint32_t decryptedCounter = 0;
        ByteSpan payload          = chip::ByteSpan(outputBuffer.data(), outputBuffer.size());

        err = CheckinMessage::ParseCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, payload, decryptedCounter, buffer);
        NL_TEST_ASSERT(inSuite, (CHIP_NO_ERROR == err));

        NL_TEST_ASSERT(inSuite, (memcmp(data, buffer.data(), sizeof(data)) == 0));
        NL_TEST_ASSERT(inSuite, (counter == decryptedCounter));

        // reset buffers
        memset(a, 0, sizeof(a));
        memset(b, 0, sizeof(b));
        outputBuffer = MutableByteSpan(a);
        buffer       = MutableByteSpan(b);

        counter += chip::Crypto::GetRandU32() + 1;

        // Cleanup
        keystore.DestroyKey(aes128KeyHandle);
        keystore.DestroyKey(hmac128KeyHandle);
    }
}

/**
 * @brief test verifies that GetAppDataSize returns the correct application data size
 */
void TestCheckInMsg::TestCheckInMessagePayloadSize(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases = ArraySize(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        ByteSpan payload(vector.payload, vector.payload_len);
        size_t calculated_size = CheckinMessage::GetAppDataSize(payload);

        // Verify the AppData size matches the expected application data size
        NL_TEST_ASSERT_EQUALS(inSuite, vector.application_data_len, calculated_size);
    }
}

/**
 * @brief test verifies that GetAppDataSize returns 0 if the payload is smaller that the minimum size
 */
void TestCheckInMsg::TestCheckInMessagePayloadSizeNullBuffer(nlTestSuite * inSuite, void * inContext)
{
    ByteSpan payload;
    size_t calculated_size = CheckinMessage::GetAppDataSize(payload);

    // Verify that the size is 0
    NL_TEST_ASSERT_EQUALS(inSuite, calculated_size, 0);
}

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestCheckinMessageGenerate_ValidInputsSameSizeOutputAsPayload", TestCheckInMsg::TestCheckinMessageGenerate_ValidInputsSameSizeOutputAsPayload),
    NL_TEST_DEF("TestCheckinMessageGenerate_ValidInputsBiggerSizeOutput", TestCheckInMsg::TestCheckinMessageGenerate_ValidInputsBiggerSizeOutput),
    NL_TEST_DEF("TestCheckinMessageGenerate_ValidInputsTooSmallOutput", TestCheckInMsg::TestCheckinMessageGenerate_ValidInputsTooSmallOutput),
    NL_TEST_DEF("TestCheckinMessageGenerate_ValidInputsTooSmallOutput", TestCheckInMsg::TestCheckInMessageGenerate_EmptyAesKeyHandle),
    NL_TEST_DEF("TestCheckinMessageGenerate_ValidInputsTooSmallOutput", TestCheckInMsg::TestCheckInMessageGenerate_EmptyHmacKeyHandle),
    NL_TEST_DEF("TestCheckinParse", TestCheckInMsg::TestCheckinParse),
    NL_TEST_DEF("TestCheckinGenerateParse", TestCheckInMsg::TestCheckinGenerateParse),
    NL_TEST_DEF("TestCheckInMessagePayloadSize", TestCheckInMsg::TestCheckInMessagePayloadSize),
    NL_TEST_DEF("TestCheckInMessagePayloadSizeNullBuffer", TestCheckInMsg::TestCheckInMessagePayloadSizeNullBuffer),

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
int TestCheckInMessage()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCheckInMessage)
