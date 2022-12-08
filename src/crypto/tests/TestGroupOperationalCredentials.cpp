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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Crypto;

namespace {

constexpr auto KEY_LENGTH = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;

// kFabricId1/kCompressedFabricIdBuffer1 matches the Compressed Fabric Identifier
// example of spec section `4.3.2.2. Compressed Fabric Identifier`. It is based on
// the public key in `kExampleOperationalRootPublicKey`.
static const uint8_t kCompressedFabricIdBuffer1[] = { 0x87, 0xe1, 0xb0, 0x04, 0xe2, 0x35, 0xa1, 0x30 };
constexpr ByteSpan kCompressedFabricId1(kCompressedFabricIdBuffer1);

static const uint8_t kEpochKeys0[][KEY_LENGTH] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f },
    { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f }
};

static GroupOperationalCredentials kGroupKeys0[] = {
    { .start_time     = 0x0000000000000000,
      .hash           = 0x479e,
      .encryption_key = { 0xc5, 0xf2, 0x69, 0x01, 0x87, 0x11, 0x51, 0x50, 0xc3, 0x56, 0xad, 0x93, 0xb3, 0x85, 0xbb, 0x0f },
      .privacy_key    = { 0xf5, 0xce, 0x81, 0x88, 0x1d, 0x11, 0x18, 0xc5, 0xc8, 0x91, 0xc9, 0x06, 0x0b, 0xc7, 0x70, 0x09 } },
    { .start_time     = 0x1111111111111111,
      .hash           = 0xa512,
      .encryption_key = { 0xae, 0xd9, 0x56, 0x95, 0xf3, 0x75, 0xd2, 0xce, 0x78, 0x55, 0x6a, 0x41, 0x73, 0x0c, 0x3f, 0x43 },
      .privacy_key    = { 0xdc, 0x90, 0xdb, 0x2e, 0x61, 0x76, 0x5a, 0x42, 0x60, 0x7f, 0xaf, 0xaf, 0x16, 0x37, 0x40, 0x0a } },
    { .start_time     = 0x2222222222222222,
      .hash           = 0xd800,
      .encryption_key = { 0x35, 0xca, 0x34, 0x6e, 0x5e, 0x24, 0xbb, 0xbe, 0x88, 0x9c, 0xf4, 0xd3, 0x5c, 0x5e, 0x82, 0x0a },
      .privacy_key    = { 0xb1, 0xfb, 0x3e, 0x79, 0xa2, 0xff, 0x27, 0xf3, 0x70, 0x4d, 0x2b, 0xe9, 0x19, 0x2d, 0xb6, 0x07 } },
};

struct GroupKeySetTestEntry
{
    const uint8_t * epochKey;                ///< Raw epoch key
    GroupOperationalCredentials * groupKeys; ///< Struct of encryption key, privacy key, and group key hash (sessionId)
};

struct GroupKeySetTestEntry theGroupKeySetTestVector[] = {
    {
        .epochKey  = kEpochKeys0[0],
        .groupKeys = &kGroupKeys0[0],
    },
    {
        .epochKey  = kEpochKeys0[1],
        .groupKeys = &kGroupKeys0[1],
    },
    {
        .epochKey  = kEpochKeys0[2],
        .groupKeys = &kGroupKeys0[2],
    },
};

void TestDeriveGroupOperationalCredentials(nlTestSuite * apSuite, void * apContext)
{
    GroupOperationalCredentials opCreds;

    for (const auto & testVector : theGroupKeySetTestVector)
    {
        const ByteSpan epochKey(testVector.epochKey, KEY_LENGTH);
        NL_TEST_ASSERT(apSuite,
                       CHIP_NO_ERROR == Crypto::DeriveGroupOperationalCredentials(epochKey, kCompressedFabricId1, opCreds));

        NL_TEST_ASSERT(apSuite, opCreds.hash == testVector.groupKeys->hash);
        NL_TEST_ASSERT(apSuite, 0 == memcmp(opCreds.encryption_key, testVector.groupKeys->encryption_key, KEY_LENGTH));
        NL_TEST_ASSERT(apSuite, 0 == memcmp(opCreds.privacy_key, testVector.groupKeys->privacy_key, KEY_LENGTH));
    }
}

/**
 *   Test Suite. It lists all the test functions.
 */
const nlTest sTests[] = { NL_TEST_DEF("TestDeriveGroupOperationalCredentials", TestDeriveGroupOperationalCredentials),
                          NL_TEST_SENTINEL() };

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
int TestGroupOperationalCredentials()
{
    nlTestSuite theSuite = { "TestGroupOperationalCredentials", &sTests[0], Test_Setup, Test_Teardown };

    // Run test suite againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestGroupOperationalCredentials)
