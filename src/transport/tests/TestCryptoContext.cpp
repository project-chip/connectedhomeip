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

#include <inttypes.h>

#include <pw_unit_test/framework.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <transport/CryptoContext.h>

using namespace chip;

namespace {

constexpr size_t MIC_LENGTH   = chip::Crypto::CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;
constexpr size_t NONCE_LENGTH = CryptoContext::kAESCCMNonceLen;

struct PrivacyNonceTestEntry
{
    uint16_t sessionId;
    uint8_t mic[MIC_LENGTH];
    uint8_t privacyNonce[NONCE_LENGTH];
};

struct PrivacyNonceTestEntry thePrivacyNonceTestVector[] = {
    {
        .sessionId    = 0x002a,
        .mic          = { 0xc5, 0xa0, 0x06, 0x3a, 0xd5, 0xd2, 0x51, 0x81, 0x91, 0x40, 0x0d, 0xd6, 0x8c, 0x5c, 0x16, 0x3b },
        .privacyNonce = { 0x00, 0x2a, 0xd2, 0x51, 0x81, 0x91, 0x40, 0x0d, 0xd6, 0x8c, 0x5c, 0x16, 0x3b },
    },
};

class TestGroupCryptoContext : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestGroupCryptoContext, TestBuildPrivacyNonce)
{
    for (const auto & testVector : thePrivacyNonceTestVector)
    {
        MessageAuthenticationCode mic;
        uint16_t sessionId = testVector.sessionId;
        const ByteSpan expectedPrivacyNonce(testVector.privacyNonce, NONCE_LENGTH);
        CryptoContext::NonceStorage privacyNonce;
        CryptoContext::ConstNonceView privacyNonceView(privacyNonce);

        mic.SetTag(nullptr, testVector.mic, MIC_LENGTH);

        EXPECT_EQ(CHIP_NO_ERROR, chip::CryptoContext::BuildPrivacyNonce(privacyNonce, sessionId, mic));
        EXPECT_EQ(0, memcmp(privacyNonceView.data(), expectedPrivacyNonce.data(), NONCE_LENGTH));
    }
}

} // namespace
