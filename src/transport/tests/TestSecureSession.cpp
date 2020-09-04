/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements unit tests for the SecureSession implementation.
 */

#include "TestTransportLayer.h"

#include <errno.h>
#include <nlunit-test.h>

#include <core/CHIPCore.h>
#include <transport/SecureSession.h>

#include <stdarg.h>
#include <support/CodeUtils.h>
#include <support/TestUtils.h>

using namespace chip;

static const uint8_t secure_channel_test_private_key1[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                                            0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                                            0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

static const uint8_t secure_channel_test_public_key1[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3,
                                                           0x0a, 0xc4, 0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29,
                                                           0x39, 0x95, 0xa2, 0xf7, 0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca,
                                                           0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79, 0x48, 0x30, 0x3c, 0x53, 0x86,
                                                           0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf, 0x9f, 0xdc, 0x35,
                                                           0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

static const uint8_t secure_channel_test_private_key2[] = { 0x00, 0xd1, 0x90, 0xd9, 0xb3, 0x95, 0x1c, 0x5f, 0xa4, 0xe7, 0x47,
                                                            0x92, 0x5b, 0x0a, 0xa9, 0xa7, 0xc1, 0x1c, 0xe7, 0x06, 0x10, 0xe2,
                                                            0xdd, 0x16, 0x41, 0x52, 0x55, 0xb7, 0xb8, 0x80, 0x8d, 0x87, 0xa1 };

static const uint8_t secure_channel_test_public_key2[] = { 0x04, 0x30, 0x77, 0x2c, 0xe7, 0xd4, 0x0a, 0xf2, 0xf3, 0x19, 0xbd,
                                                           0xfb, 0x1f, 0xcc, 0x88, 0xd9, 0x83, 0x25, 0x89, 0xf2, 0x09, 0xf3,
                                                           0xab, 0xe4, 0x33, 0xb6, 0x7a, 0xff, 0x73, 0x3b, 0x01, 0x35, 0x34,
                                                           0x92, 0x73, 0x14, 0x59, 0x0b, 0xbd, 0x44, 0x72, 0x1b, 0xcd, 0xb9,
                                                           0x02, 0x53, 0xd9, 0xaf, 0xcc, 0x1a, 0xcd, 0xae, 0xe8, 0x87, 0x2e,
                                                           0x52, 0x3b, 0x98, 0xf0, 0xa1, 0x88, 0x4a, 0xe3, 0x03, 0x75 };

void SecureChannelInitTest(nlTestSuite * inSuite, void * inContext)
{
    SecureSession channel;
    // Test all combinations of invalid parameters
    NL_TEST_ASSERT(inSuite, channel.Init(NULL, 0, NULL, 0, NULL, 0, NULL, 0) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, 0, NULL, 0, NULL, 0, NULL, 0) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1), NULL, 0, NULL, 0, NULL,
                                0) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1),
                                secure_channel_test_private_key2, 0, NULL, 0, NULL, 0) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1),
                                secure_channel_test_private_key2, sizeof(secure_channel_test_private_key2), NULL, 0, NULL,
                                0) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1),
                                secure_channel_test_private_key2, sizeof(secure_channel_test_private_key2), NULL, 10, NULL,
                                0) == CHIP_ERROR_INVALID_ARGUMENT);

    // Test the channel is successfully created with valid parameters
    const char * info = "Test Info";
    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1),
                                secure_channel_test_private_key2, sizeof(secure_channel_test_private_key2), NULL, 0,
                                (const uint8_t *) info, sizeof(info)) == CHIP_NO_ERROR);

    // Test the channel cannot be reinitialized
    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1),
                                secure_channel_test_private_key2, sizeof(secure_channel_test_private_key2), NULL, 0,
                                (const uint8_t *) info, sizeof(info)) == CHIP_ERROR_INCORRECT_STATE);

    // Test the channel can be initialized with valid salt
    const char * salt = "Test Salt";
    SecureSession channel2;
    NL_TEST_ASSERT(inSuite,
                   channel2.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1),
                                 secure_channel_test_private_key2, sizeof(secure_channel_test_private_key2), (const uint8_t *) salt,
                                 sizeof(salt), (const uint8_t *) info, sizeof(info)) == CHIP_NO_ERROR);
}

void SecureChannelEncryptTest(nlTestSuite * inSuite, void * inContext)
{
    SecureSession channel;
    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t output[128];
    MessageHeader header;

    // Test uninitialized channel
    NL_TEST_ASSERT(inSuite,
                   channel.Encrypt(plain_text, sizeof(plain_text), output, header) == CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);

    const char * info = "Test Info";
    const char * salt = "Test Salt";
    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1),
                                secure_channel_test_private_key2, sizeof(secure_channel_test_private_key2), (const uint8_t *) salt,
                                sizeof(salt), (const uint8_t *) info, sizeof(info)) == CHIP_NO_ERROR);

    // Test initialized channel, but invalid arguments
    NL_TEST_ASSERT(inSuite, channel.Encrypt(NULL, 0, NULL, header) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, channel.Encrypt(plain_text, 0, NULL, header) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, channel.Encrypt(plain_text, sizeof(plain_text), NULL, header) == CHIP_ERROR_INVALID_ARGUMENT);

    // Valid arguments
    NL_TEST_ASSERT(inSuite, channel.Encrypt(plain_text, sizeof(plain_text), output, header) == CHIP_NO_ERROR);
}

void SecureChannelDecryptTest(nlTestSuite * inSuite, void * inContext)
{
    SecureSession channel;
    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t encrypted[128];
    MessageHeader header;

    const char * info = "Test Info";
    const char * salt = "Test Salt";

    NL_TEST_ASSERT(inSuite,
                   channel.Init(secure_channel_test_public_key1, sizeof(secure_channel_test_public_key1),
                                secure_channel_test_private_key2, sizeof(secure_channel_test_private_key2), (const uint8_t *) salt,
                                sizeof(salt), (const uint8_t *) info, sizeof(info)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, channel.Encrypt(plain_text, sizeof(plain_text), encrypted, header) == CHIP_NO_ERROR);

    SecureSession channel2;
    uint8_t output[128];
    // Uninitialized channel
    NL_TEST_ASSERT(inSuite,
                   channel2.Decrypt(encrypted, sizeof(plain_text), output, header) == CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    NL_TEST_ASSERT(inSuite,
                   channel2.Init(secure_channel_test_public_key2, sizeof(secure_channel_test_public_key2),
                                 secure_channel_test_private_key1, sizeof(secure_channel_test_private_key1), (const uint8_t *) salt,
                                 sizeof(salt), (const uint8_t *) info, sizeof(info)) == CHIP_NO_ERROR);

    // Channel initialized, but invalid arguments to decrypt
    NL_TEST_ASSERT(inSuite, channel2.Decrypt(NULL, 0, NULL, header) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, channel2.Decrypt(encrypted, 0, NULL, header) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, channel2.Decrypt(encrypted, sizeof(encrypted), NULL, header) == CHIP_ERROR_INVALID_ARGUMENT);

    // Valid arguments
    NL_TEST_ASSERT(inSuite, channel2.Decrypt(encrypted, sizeof(plain_text), output, header) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, memcmp(plain_text, output, sizeof(plain_text)) == 0);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Init",    SecureChannelInitTest),
    NL_TEST_DEF("Encrypt", SecureChannelEncryptTest),
    NL_TEST_DEF("Decrypt", SecureChannelDecryptTest),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecureChannel",
    &sTests[0],
    NULL,
    NULL
};
// clang-format on

/**
 *  Main
 */
int TestSecureSession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, NULL);

    return (nlTestRunnerStats(&sSuite));
}

static void __attribute__((constructor)) TestSecureSessionCtor(void)
{
    VerifyOrDie(RegisterUnitTests(&TestSecureSession) == CHIP_NO_ERROR);
}

namespace chip {
namespace Logging {
void __attribute__((weak)) LogV(uint8_t module, uint8_t category, const char * format, va_list argptr)
{
    (void) module, (void) category;
    vfprintf(stderr, format, argptr);
}
} // namespace Logging
} // namespace chip
