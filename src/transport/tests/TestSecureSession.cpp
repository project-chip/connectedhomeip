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

#include <errno.h>
#include <nlunit-test.h>

#include <lib/core/CHIPCore.h>
#include <transport/SecureSession.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <stdarg.h>

using namespace chip;
using namespace Crypto;

void SecureChannelInitTest(nlTestSuite * inSuite, void * inContext)
{
    SecureSession channel;

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256Keypair keypair2;
    NL_TEST_ASSERT(inSuite, keypair2.Initialize() == CHIP_NO_ERROR);

    // Test all combinations of invalid parameters
    NL_TEST_ASSERT(inSuite,
                   channel.Init(keypair, keypair2.Pubkey(), ByteSpan(nullptr, 10),
                                SecureSession::SessionInfoType::kSessionEstablishment,
                                SecureSession::SessionRole::kInitiator) == CHIP_ERROR_INVALID_ARGUMENT);

    // Test the channel is successfully created with valid parameters
    NL_TEST_ASSERT(inSuite,
                   channel.Init(keypair, keypair2.Pubkey(), ByteSpan(nullptr, 0),
                                SecureSession::SessionInfoType::kSessionEstablishment,
                                SecureSession::SessionRole::kInitiator) == CHIP_NO_ERROR);

    // Test the channel cannot be reinitialized
    NL_TEST_ASSERT(inSuite,
                   channel.Init(keypair, keypair2.Pubkey(), ByteSpan(nullptr, 0),
                                SecureSession::SessionInfoType::kSessionEstablishment,
                                SecureSession::SessionRole::kInitiator) == CHIP_ERROR_INCORRECT_STATE);

    // Test the channel can be initialized with valid salt
    const char * salt = "Test Salt";
    SecureSession channel2;
    NL_TEST_ASSERT(inSuite,
                   channel2.Init(keypair, keypair2.Pubkey(), ByteSpan((const uint8_t *) salt, sizeof(salt)),
                                 SecureSession::SessionInfoType::kSessionEstablishment,
                                 SecureSession::SessionRole::kInitiator) == CHIP_NO_ERROR);
}

void SecureChannelEncryptTest(nlTestSuite * inSuite, void * inContext)
{
    SecureSession channel;
    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t output[128];
    PacketHeader packetHeader;
    MessageAuthenticationCode mac;

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256Keypair keypair2;
    NL_TEST_ASSERT(inSuite, keypair2.Initialize() == CHIP_NO_ERROR);

    // Test uninitialized channel
    NL_TEST_ASSERT(inSuite,
                   channel.Encrypt(plain_text, sizeof(plain_text), output, packetHeader, mac) ==
                       CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);

    const char * salt = "Test Salt";
    NL_TEST_ASSERT(inSuite,
                   channel.Init(keypair, keypair2.Pubkey(), ByteSpan((const uint8_t *) salt, sizeof(salt)),
                                SecureSession::SessionInfoType::kSessionEstablishment,
                                SecureSession::SessionRole::kInitiator) == CHIP_NO_ERROR);

    // Test initialized channel, but invalid arguments
    NL_TEST_ASSERT(inSuite, channel.Encrypt(nullptr, 0, nullptr, packetHeader, mac) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, channel.Encrypt(plain_text, 0, nullptr, packetHeader, mac) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   channel.Encrypt(plain_text, sizeof(plain_text), nullptr, packetHeader, mac) == CHIP_ERROR_INVALID_ARGUMENT);

    // Valid arguments
    NL_TEST_ASSERT(inSuite, channel.Encrypt(plain_text, sizeof(plain_text), output, packetHeader, mac) == CHIP_NO_ERROR);
}

void SecureChannelDecryptTest(nlTestSuite * inSuite, void * inContext)
{
    SecureSession channel;
    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t encrypted[128];
    PacketHeader packetHeader;
    MessageAuthenticationCode mac;

    const char * salt = "Test Salt";

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    P256Keypair keypair2;
    NL_TEST_ASSERT(inSuite, keypair2.Initialize() == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   channel.Init(keypair, keypair2.Pubkey(), ByteSpan((const uint8_t *) salt, sizeof(salt)),
                                SecureSession::SessionInfoType::kSessionEstablishment,
                                SecureSession::SessionRole::kInitiator) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, channel.Encrypt(plain_text, sizeof(plain_text), encrypted, packetHeader, mac) == CHIP_NO_ERROR);

    SecureSession channel2;
    uint8_t output[128];
    // Uninitialized channel
    NL_TEST_ASSERT(inSuite,
                   channel2.Decrypt(encrypted, sizeof(plain_text), output, packetHeader, mac) ==
                       CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    NL_TEST_ASSERT(inSuite,
                   channel2.Init(keypair2, keypair.Pubkey(), ByteSpan((const uint8_t *) salt, sizeof(salt)),
                                 SecureSession::SessionInfoType::kSessionEstablishment,
                                 SecureSession::SessionRole::kResponder) == CHIP_NO_ERROR);

    // Channel initialized, but invalid arguments to decrypt
    NL_TEST_ASSERT(inSuite, channel2.Decrypt(nullptr, 0, nullptr, packetHeader, mac) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, channel2.Decrypt(encrypted, 0, nullptr, packetHeader, mac) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   channel2.Decrypt(encrypted, sizeof(encrypted), nullptr, packetHeader, mac) == CHIP_ERROR_INVALID_ARGUMENT);

    // Valid arguments
    NL_TEST_ASSERT(inSuite, channel2.Decrypt(encrypted, sizeof(plain_text), output, packetHeader, mac) == CHIP_NO_ERROR);

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
    "Test-CHIP-SecureSession",
    &sTests[0],
    nullptr,
    nullptr
};
// clang-format on

/**
 *  Main
 */
int TestSecureSession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSecureSession)
