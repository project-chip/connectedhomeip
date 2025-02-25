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

#include <pw_unit_test/framework.h>

#include <crypto/DefaultSessionKeystore.h>
#include <crypto/RandUtils.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <protocols/secure_channel/tests/CheckIn_Message_test_vectors.h>
#include <transport/CryptoContext.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::Protocols::SecureChannel;
using namespace chip::Crypto;
using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

namespace {

/**
 * @brief Helper function that generates the Check-In message based on the test vector
 *        and verifies the generated Check-In message
 *        Helper is to avoid having the same code three times in different tests
 *
 * @return CHIP_NO_ERROR if the generation was successful
 *         error code if the generation failed - see GenerateCheckinMessagePayload
 */
CHIP_ERROR GenerateAndVerifyPayload(MutableByteSpan & output, const CheckIn_Message_test_vector & vector)
{
    TestSessionKeystoreImpl keystore;

    // Two distinct key material buffers to ensure crypto-hardware-assist with single-usage keys create two different handles.
    Symmetric128BitsKeyByteArray aesKeyMaterial;
    memcpy(aesKeyMaterial, vector.key, vector.key_len);

    Symmetric128BitsKeyByteArray hmacKeyMaterial;
    memcpy(hmacKeyMaterial, vector.key, vector.key_len);

    Aes128KeyHandle aes128KeyHandle;
    EXPECT_EQ(keystore.CreateKey(aesKeyMaterial, aes128KeyHandle), CHIP_NO_ERROR);

    Hmac128KeyHandle hmac128KeyHandle;
    EXPECT_EQ(keystore.CreateKey(hmacKeyMaterial, hmac128KeyHandle), CHIP_NO_ERROR);

    // Create application data ByteSpan
    ByteSpan applicationData(vector.application_data, vector.application_data_len);

    // Verify that the generation succeeded
    CHIP_ERROR err =
        CheckinMessage::GenerateCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, vector.counter, applicationData, output);
    if (err != CHIP_NO_ERROR)
    {
        keystore.DestroyKey(aes128KeyHandle);
        keystore.DestroyKey(hmac128KeyHandle);

        return err;
    }

    // Validate Full payload
    EXPECT_EQ(output.size(), vector.payload_len);
    EXPECT_EQ(memcmp(vector.payload, output.data(), output.size()), 0);

    size_t cursorIndex = 0;

    // Validate Nonce
    MutableByteSpan nonce = output.SubSpan(cursorIndex, vector.nonce_len);
    EXPECT_EQ(memcmp(vector.nonce, nonce.data(), nonce.size()), 0);
    cursorIndex += nonce.size();

    // Validate ciphertext
    MutableByteSpan ciphertext = output.SubSpan(cursorIndex, vector.ciphertext_len);
    EXPECT_EQ(memcmp(vector.ciphertext, ciphertext.data(), ciphertext.size()), 0);
    cursorIndex += ciphertext.size();

    // Validate MIC
    MutableByteSpan mic = output.SubSpan(cursorIndex, vector.mic_len);
    EXPECT_EQ(memcmp(vector.mic, mic.data(), mic.size()), 0);
    cursorIndex += mic.size();

    // Clean up
    keystore.DestroyKey(aes128KeyHandle);
    keystore.DestroyKey(hmac128KeyHandle);

    return err;
}

/**
 * @brief Helper function that parses the Check-In message based on the test vector
 *        and verifies parsed Check-In message
 *        Helper is to avoid having the same code in multiple tests
 *
 * @return CHIP_NO_ERROR if the parsing was successful
 *         error code if the generation failed - see ParseCheckinMessagePayload
 */
CHIP_ERROR ParseAndVerifyPayload(MutableByteSpan & applicationData, const CheckIn_Message_test_vector & vector,
                                 bool injectInvalidNonce)
{
    TestSessionKeystoreImpl keystore;

    // Copy payload to be able to modify it for invalid nonce tests
    uint8_t payloadBuffer[300] = { 0 };
    memcpy(payloadBuffer, vector.payload, vector.payload_len);

    if (injectInvalidNonce)
    {
        // Modify nonce to validate that the parsing can detect that the message was manipulated
        payloadBuffer[0] ^= 0xFF;
    }

    // Create payload byte span
    ByteSpan payload(payloadBuffer, vector.payload_len);

    CounterType decryptedCounter = 0;

    // Two distinct key material buffers to ensure crypto-hardware-assist with single-usage keys create two different handles.
    Symmetric128BitsKeyByteArray aesKeyMaterial;
    memcpy(aesKeyMaterial, vector.key, vector.key_len);

    Symmetric128BitsKeyByteArray hmacKeyMaterial;
    memcpy(hmacKeyMaterial, vector.key, vector.key_len);

    Aes128KeyHandle aes128KeyHandle;
    EXPECT_EQ(keystore.CreateKey(aesKeyMaterial, aes128KeyHandle), CHIP_NO_ERROR);

    Hmac128KeyHandle hmac128KeyHandle;
    EXPECT_EQ(keystore.CreateKey(hmacKeyMaterial, hmac128KeyHandle), CHIP_NO_ERROR);

    // Verify that the Parsing succeeded
    CHIP_ERROR err =
        CheckinMessage::ParseCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, payload, decryptedCounter, applicationData);
    if (err != CHIP_NO_ERROR)
    {
        keystore.DestroyKey(aes128KeyHandle);
        keystore.DestroyKey(hmac128KeyHandle);

        return err;
    }

    // Verify decrypted counter value
    EXPECT_EQ(vector.counter, decryptedCounter);

    // Verify application data
    EXPECT_EQ(vector.application_data_len, applicationData.size());
    EXPECT_EQ(memcmp(vector.application_data, applicationData.data(), applicationData.size()), 0);

    // Cleanup
    keystore.DestroyKey(aes128KeyHandle);
    keystore.DestroyKey(hmac128KeyHandle);

    return err;
}

/**
 * @brief Test verifies that the Check-In message generation is successful when using an output size equal to the payload size
 */
TEST(TestCheckInMsg, TestCheckinMessageGenerate_ValidInputsSameSizeOutputAsPayload)
{
    int numOfTestCases = MATTER_ARRAY_SIZE(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        // Create output buffer
        uint8_t buffer[300] = { 0 };
        MutableByteSpan output(buffer, sizeof(buffer));

        // Force output buffer to the payload size
        output.reduce_size(vector.payload_len);

        EXPECT_EQ(GenerateAndVerifyPayload(output, vector), CHIP_NO_ERROR);
    }
}

/**
 * @brief Test verifies that the Check-In message generation is successful when using an output size greater than the payload size
 */
TEST(TestCheckInMsg, TestCheckinMessageGenerate_ValidInputsBiggerSizeOutput)
{
    int numOfTestCases = MATTER_ARRAY_SIZE(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        // Create output buffer
        uint8_t buffer[300] = { 0 };
        MutableByteSpan output(buffer, sizeof(buffer));

        EXPECT_EQ(GenerateAndVerifyPayload(output, vector), CHIP_NO_ERROR);
    }
}

/**
 * @brief Test verifies that the Check-In message generation returns an error if the output buffer is too small
 */
TEST(TestCheckInMsg, TestCheckinMessageGenerate_ValidInputsTooSmallOutput)
{
    CheckIn_Message_test_vector vector = checkIn_message_test_vectors[0];

    // Create output buffer with 0 size
    MutableByteSpan output;
    EXPECT_EQ(GenerateAndVerifyPayload(output, vector), CHIP_ERROR_BUFFER_TOO_SMALL);
}

/**
 * @brief Test verifies that the Check-In Message generations returns an error if the AesKeyHandle is empty
 */
TEST(TestCheckInMsg, TestCheckInMessageGenerate_EmptyAesKeyHandle)
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
    EXPECT_EQ(keystore.CreateKey(hmacKeyMaterial, hmac128KeyHandle), CHIP_NO_ERROR);

    // Create application data ByteSpan
    ByteSpan applicationData(vector.application_data, vector.application_data_len);

/*
    TODO(#28986): Passing an empty key handle while using PSA crypto will result in a failure.
                  When using OpenSSL this same test result in a success.
*/
#if 0
    // Verify that the generation fails with an empty key handle
    EXPECT_NE(
        CHIP_NO_ERROR,
        CheckinMessage::GenerateCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, vector.counter, applicationData, output));
#endif

    // Clean up
    keystore.DestroyKey(hmac128KeyHandle);
}

/**
 * @brief Test verifies that the Check-In Message generations returns an error if the HmacKeyHandle is empty
 */
TEST(TestCheckInMsg, TestCheckInMessageGenerate_EmptyHmacKeyHandle)
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
    EXPECT_EQ(keystore.CreateKey(aesKeyMaterial, aes128KeyHandle), CHIP_NO_ERROR);

    Hmac128KeyHandle hmac128KeyHandle;

    // Create application data ByteSpan
    ByteSpan applicationData(vector.application_data, vector.application_data_len);

/*
    TODO(#28986): Passing an empty key handle while using PSA crypto will result in a failure.
                  When using OpenSSL this same test result in a success.
*/
#if 0
    // Verify that the generation fails with an empty key handle
    EXPECT_NE(
        CHIP_NO_ERROR,
        CheckinMessage::GenerateCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, vector.counter, applicationData, output));
#endif

    // Clean up
    keystore.DestroyKey(aes128KeyHandle);
}

/**
 * @brief Test verifies that the Check-In message parsing succeeds with the Application buffer set to the minimum required size
 */
TEST(TestCheckInMsg, TestCheckinMessageParse_ValidInputsSameSizeMinAppData)
{
    int numOfTestCases = MATTER_ARRAY_SIZE(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        uint8_t applicationDataBuffer[128] = { 0 };
        MutableByteSpan applicationData(applicationDataBuffer, sizeof(applicationDataBuffer));
        applicationData.reduce_size(vector.application_data_len + sizeof(CounterType));

        EXPECT_EQ(ParseAndVerifyPayload(applicationData, vector, false), CHIP_NO_ERROR);
    }
}

/**
 * @brief Test verifies that the Check-In message parsing succeeds with the Application buffer set to a larger than necessary size
 */
TEST(TestCheckInMsg, TestCheckinMessageParse_ValidInputsBiggerSizeMinAppData)
{
    int numOfTestCases = MATTER_ARRAY_SIZE(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        uint8_t applicationDataBuffer[128] = { 0 };
        MutableByteSpan applicationData(applicationDataBuffer, sizeof(applicationDataBuffer));

        EXPECT_EQ(ParseAndVerifyPayload(applicationData, vector, false), CHIP_NO_ERROR);
    }
}

/**
 * @brief Test verifies that the Check-In message throws an error if the application data buffer is too small
 */
TEST(TestCheckInMsg, TestCheckinMessageParse_ValidInputsTooSmallAppData)
{
    CheckIn_Message_test_vector vector = checkIn_message_test_vectors[0];

    // Create applicationData buffer with 0 size
    MutableByteSpan applicationData;

    EXPECT_EQ(ParseAndVerifyPayload(applicationData, vector, false), CHIP_ERROR_BUFFER_TOO_SMALL);
}

/**
 * @brief Test verifies that the Check-In Message parsing returns an error if the AesKeyHandle is empty
 */
TEST(TestCheckInMsg, TestCheckInMessageParse_EmptyAesKeyHandle)
{
    TestSessionKeystoreImpl keystore;
    CheckIn_Message_test_vector vector = checkIn_message_test_vectors[0];

    // Create application data ByteSpan
    uint8_t applicationDataBuffer[128] = { 0 };
    MutableByteSpan applicationData(applicationDataBuffer, sizeof(applicationDataBuffer));
    applicationData.reduce_size(vector.application_data_len + sizeof(CounterType));

    // Create payload byte span
    ByteSpan payload(vector.payload, vector.payload_len);

    CounterType decryptedCounter = 0;
    //
    (void) decryptedCounter;

    // Empty AES Key handle
    Aes128KeyHandle aes128KeyHandle;

    Symmetric128BitsKeyByteArray hmacKeyMaterial;
    memcpy(hmacKeyMaterial, vector.key, vector.key_len);

    Hmac128KeyHandle hmac128KeyHandle;
    EXPECT_EQ(keystore.CreateKey(hmacKeyMaterial, hmac128KeyHandle), CHIP_NO_ERROR);

/*
    TODO(#28986): Passing an empty key handle while using PSA crypto will result in a failure.
                  When using OpenSSL this same test result in a success.
*/
#if 0
    // Verify that the generation fails with an empty key handle
    EXPECT_NE(
        CHIP_NO_ERROR,
        CheckinMessage::ParseCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, payload, decryptedCounter, applicationData));
#endif

    // Clean up
    keystore.DestroyKey(hmac128KeyHandle);
}

/**
 * @brief Test verifies that the Check-In Message parsing returns an error if the HmacKeyHandle is empty
 */
TEST(TestCheckInMsg, TestCheckInMessageParse_EmptyHmacKeyHandle)
{
    TestSessionKeystoreImpl keystore;
    CheckIn_Message_test_vector vector = checkIn_message_test_vectors[0];

    // Create application data ByteSpan
    uint8_t applicationDataBuffer[128] = { 0 };
    MutableByteSpan applicationData(applicationDataBuffer, sizeof(applicationDataBuffer));
    applicationData.reduce_size(vector.application_data_len + sizeof(CounterType));

    // Create payload byte span
    ByteSpan payload(vector.payload, vector.payload_len);

    CounterType decryptedCounter = 0;
    //
    (void) decryptedCounter;

    // Empty Hmac Key handle
    Hmac128KeyHandle hmac128KeyHandle;

    Symmetric128BitsKeyByteArray aesKeyMaterial;
    memcpy(aesKeyMaterial, vector.key, vector.key_len);

    Aes128KeyHandle aes128KeyHandle;
    EXPECT_EQ(keystore.CreateKey(aesKeyMaterial, aes128KeyHandle), CHIP_NO_ERROR);

/*
    TODO(#28986): Passing an empty key handle while using PSA crypto will result in a failure.
                  When using OpenSSL this same test result in a success.
*/
#if 0
    // Verify that the generation fails with an empty key handle
    EXPECT_NE(
        CHIP_NO_ERROR,
        CheckinMessage::ParseCheckinMessagePayload(aes128KeyHandle, hmac128KeyHandle, payload, decryptedCounter, applicationData));
#endif

    // Clean up
    keystore.DestroyKey(aes128KeyHandle);
}

/**
 * @brief Test verifies that the Check-In message processing throws an error if the nonce is corrupted
 */
TEST(TestCheckInMsg, TestCheckinMessageParse_CorruptedNonce)
{
    int numOfTestCases = MATTER_ARRAY_SIZE(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        uint8_t applicationDataBuffer[128] = { 0 };
        MutableByteSpan applicationData(applicationDataBuffer, sizeof(applicationDataBuffer));
        applicationData.reduce_size(vector.application_data_len + sizeof(CounterType));

        EXPECT_EQ(ParseAndVerifyPayload(applicationData, vector, true), CHIP_ERROR_INTERNAL);
    }
}

/**
 * @brief Test verifies that the Check-In message processing throws an error if the nonce was not calculated with the counter in the
 * payload
 */
TEST(TestCheckInMsg, TestCheckinMessageParse_InvalidNonce)
{
    CheckIn_Message_test_vector vector = invalidNonceVector;

    uint8_t applicationDataBuffer[128] = { 0 };
    MutableByteSpan applicationData(applicationDataBuffer, sizeof(applicationDataBuffer));
    applicationData.reduce_size(vector.application_data_len + sizeof(CounterType));

    EXPECT_EQ(ParseAndVerifyPayload(applicationData, vector, true), CHIP_ERROR_INTERNAL);
}

/**
 * @brief test verifies that GetAppDataSize returns the correct application data size
 */
TEST(TestCheckInMsg, TestCheckInMessagePayloadSize)
{
    int numOfTestCases = MATTER_ARRAY_SIZE(checkIn_message_test_vectors);
    for (int numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        CheckIn_Message_test_vector vector = checkIn_message_test_vectors[numOfTestsExecuted];

        ByteSpan payload(vector.payload, vector.payload_len);
        size_t calculated_size = CheckinMessage::GetAppDataSize(payload);

        // Verify the AppData size matches the expected application data size
        EXPECT_EQ(vector.application_data_len, calculated_size);
    }
}

/**
 * @brief test verifies that GetAppDataSize returns 0 if the payload is smaller that the minimum size
 */
TEST(TestCheckInMsg, TestCheckInMessagePayloadSizeNullBuffer)
{
    ByteSpan payload;
    size_t calculated_size = CheckinMessage::GetAppDataSize(payload);
    size_t expected_size   = 0;

    // Verify that the size is 0
    EXPECT_EQ(calculated_size, expected_size);
}

} // namespace
