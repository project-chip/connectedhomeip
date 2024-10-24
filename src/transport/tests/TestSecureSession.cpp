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
 *      This file implements unit tests for the CryptoContext implementation.
 */

#include <errno.h>
#include <stdarg.h>

#include <pw_unit_test/framework.h>

#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <transport/CryptoContext.h>

using namespace chip;
using namespace Crypto;

TEST(TestSecureSession, SecureChannelInitTest)
{
    Crypto::DefaultSessionKeystore sessionKeystore;
    CryptoContext channel;

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDH), CHIP_NO_ERROR);

    P256Keypair keypair2;
    EXPECT_EQ(keypair2.Initialize(ECPKeyTarget::ECDH), CHIP_NO_ERROR);

    // Test the channel is successfully created with valid parameters
    EXPECT_EQ(channel.InitFromKeyPair(sessionKeystore, keypair, keypair2.Pubkey(), ByteSpan(),
                                      CryptoContext::SessionInfoType::kSessionEstablishment,
                                      CryptoContext::SessionRole::kInitiator),
              CHIP_NO_ERROR);

    // Test the channel cannot be reinitialized
    EXPECT_EQ(channel.InitFromKeyPair(sessionKeystore, keypair, keypair2.Pubkey(), ByteSpan(),
                                      CryptoContext::SessionInfoType::kSessionEstablishment,
                                      CryptoContext::SessionRole::kInitiator),
              CHIP_ERROR_INCORRECT_STATE);

    // Test the channel can be initialized with valid salt
    const char * salt = "Test Salt";
    CryptoContext channel2;
    EXPECT_EQ(channel2.InitFromKeyPair(sessionKeystore, keypair, keypair2.Pubkey(), ByteSpan((const uint8_t *) salt, strlen(salt)),
                                       CryptoContext::SessionInfoType::kSessionEstablishment,
                                       CryptoContext::SessionRole::kInitiator),
              CHIP_NO_ERROR);
}

TEST(TestSecureSession, SecureChannelEncryptTest)
{
    Crypto::DefaultSessionKeystore sessionKeystore;
    CryptoContext channel;
    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t output[128];
    PacketHeader packetHeader;
    MessageAuthenticationCode mac;

    packetHeader.SetSessionId(1);
    EXPECT_TRUE(packetHeader.IsEncrypted());
    EXPECT_EQ(packetHeader.MICTagLength(), 16);

    CryptoContext::NonceStorage nonce;
    CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), packetHeader.GetMessageCounter(), 0);

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDH), CHIP_NO_ERROR);

    P256Keypair keypair2;
    EXPECT_EQ(keypair2.Initialize(ECPKeyTarget::ECDH), CHIP_NO_ERROR);

    // Test uninitialized channel
    EXPECT_EQ(channel.Encrypt(plain_text, sizeof(plain_text), output, nonce, packetHeader, mac),
              CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);

    const char * salt = "Test Salt";
    EXPECT_EQ(channel.InitFromKeyPair(sessionKeystore, keypair, keypair2.Pubkey(), ByteSpan((const uint8_t *) salt, strlen(salt)),
                                      CryptoContext::SessionInfoType::kSessionEstablishment,
                                      CryptoContext::SessionRole::kInitiator),
              CHIP_NO_ERROR);

    // Test initialized channel, but invalid arguments
    EXPECT_EQ(channel.Encrypt(nullptr, 0, nullptr, nonce, packetHeader, mac), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(channel.Encrypt(plain_text, 0, nullptr, nonce, packetHeader, mac), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(channel.Encrypt(plain_text, sizeof(plain_text), nullptr, nonce, packetHeader, mac), CHIP_ERROR_INVALID_ARGUMENT);

    // Valid arguments
    EXPECT_EQ(channel.Encrypt(plain_text, sizeof(plain_text), output, nonce, packetHeader, mac), CHIP_NO_ERROR);
}

TEST(TestSecureSession, SecureChannelDecryptTest)
{
    Crypto::DefaultSessionKeystore sessionKeystore;
    CryptoContext channel;
    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t encrypted[128];
    PacketHeader packetHeader;
    MessageAuthenticationCode mac;

    packetHeader.SetSessionId(1);
    EXPECT_TRUE(packetHeader.IsEncrypted());
    EXPECT_EQ(packetHeader.MICTagLength(), 16);

    CryptoContext::NonceStorage nonce;
    CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), packetHeader.GetMessageCounter(), 0);

    const char * salt = "Test Salt";

    P256Keypair keypair;
    EXPECT_EQ(keypair.Initialize(ECPKeyTarget::ECDH), CHIP_NO_ERROR);

    P256Keypair keypair2;
    EXPECT_EQ(keypair2.Initialize(ECPKeyTarget::ECDH), CHIP_NO_ERROR);

    EXPECT_EQ(channel.InitFromKeyPair(sessionKeystore, keypair, keypair2.Pubkey(), ByteSpan((const uint8_t *) salt, strlen(salt)),
                                      CryptoContext::SessionInfoType::kSessionEstablishment,
                                      CryptoContext::SessionRole::kInitiator),
              CHIP_NO_ERROR);
    EXPECT_EQ(channel.Encrypt(plain_text, sizeof(plain_text), encrypted, nonce, packetHeader, mac), CHIP_NO_ERROR);

    CryptoContext channel2;
    uint8_t output[128];
    // Uninitialized channel
    EXPECT_EQ(channel2.Decrypt(encrypted, sizeof(plain_text), output, nonce, packetHeader, mac),
              CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    EXPECT_EQ(channel2.InitFromKeyPair(sessionKeystore, keypair2, keypair.Pubkey(), ByteSpan((const uint8_t *) salt, strlen(salt)),
                                       CryptoContext::SessionInfoType::kSessionEstablishment,
                                       CryptoContext::SessionRole::kResponder),
              CHIP_NO_ERROR);

    // Channel initialized, but invalid arguments to decrypt
    EXPECT_EQ(channel2.Decrypt(nullptr, 0, nullptr, nonce, packetHeader, mac), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(channel2.Decrypt(encrypted, 0, nullptr, nonce, packetHeader, mac), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(channel2.Decrypt(encrypted, sizeof(encrypted), nullptr, nonce, packetHeader, mac), CHIP_ERROR_INVALID_ARGUMENT);

    // Valid arguments
    EXPECT_EQ(channel2.Decrypt(encrypted, sizeof(plain_text), output, nonce, packetHeader, mac), CHIP_NO_ERROR);

    EXPECT_EQ(memcmp(plain_text, output, sizeof(plain_text)), 0);
}
