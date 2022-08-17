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
#include <nlunit-test.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

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

const uint16_t thePrivacyNonceTestVectorLength = sizeof(thePrivacyNonceTestVector) / sizeof(thePrivacyNonceTestVector[0]);

void TestBuildPrivacyNonce(nlTestSuite * apSuite, void * apContext)
{
    for (unsigned i = 0; i < thePrivacyNonceTestVectorLength; i++)
    {
        MessageAuthenticationCode mic;
        uint16_t sessionId = thePrivacyNonceTestVector[i].sessionId;
        const ByteSpan expectedPrivacyNonce(thePrivacyNonceTestVector[i].privacyNonce, NONCE_LENGTH);
        CryptoContext::NonceStorage privacyNonce;
        CryptoContext::ConstNonceView privacyNonceView(privacyNonce);

        mic.SetTag(nullptr, thePrivacyNonceTestVector[i].mic, MIC_LENGTH);

        NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == chip::CryptoContext::BuildPrivacyNonce(privacyNonce, sessionId, mic));
        NL_TEST_ASSERT(apSuite, 0 == memcmp(privacyNonceView.data(), expectedPrivacyNonce.data(), NONCE_LENGTH));
    }
}

/**
 *   Test Suite. It lists all the test functions.
 */
const nlTest sTests[] = { NL_TEST_DEF("TestBuildPrivacyNonce", TestBuildPrivacyNonce), NL_TEST_SENTINEL() };

/**
 *  Set up the test suite.
 */
int Test_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    VerifyOrReturnError(error == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int Test_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestGroupCryptoContext()
{
    nlTestSuite theSuite = { "TestGroupCryptoContext", &sTests[0], Test_Setup, Test_Teardown };

    // Run test suite againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestGroupCryptoContext)
