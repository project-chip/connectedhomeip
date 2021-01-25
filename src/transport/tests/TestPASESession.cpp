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
 *      This file implements unit tests for the PASESession implementation.
 */

#include <errno.h>
#include <nlunit-test.h>

#include <core/CHIPCore.h>
#include <core/CHIPSafeCasts.h>
#include <stdarg.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>
#include <transport/PASESession.h>

using namespace chip;

class TestSecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    CHIP_ERROR SendSessionEstablishmentMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
                                               System::PacketBufferHandle msgBuf) override
    {
        mNumMessageSend++;
        return (peer != nullptr) ? peer->HandlePeerMessage(header, peerAddress, std::move(msgBuf)) : mMessageSendError;
    }

    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished() override { mNumPairingComplete++; }

    uint32_t mNumMessageSend     = 0;
    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
    CHIP_ERROR mMessageSendError = CHIP_NO_ERROR;

    PASESession * peer = nullptr;
};

void SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;
    PASESession pairing;

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(1234, 500, nullptr, 0, Optional<NodeId>::Value(1), 0, &delegate) ==
                       CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(1), 0, nullptr) ==
                       CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(1), 0, &delegate) ==
                       CHIP_NO_ERROR);
}

void SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;
    PASESession pairing;

    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, Optional<NodeId>::Value(1), 2, 0, nullptr) !=
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, Optional<NodeId>::Value(1), 2, 0, &delegate) ==
                       CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, delegate.mNumMessageSend == 1);

    delegate.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    PASESession pairing1;

    NL_TEST_ASSERT(inSuite,
                   pairing1.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, Optional<NodeId>::Value(1), 2, 0,
                                 &delegate) == CHIP_ERROR_BAD_REQUEST);
}

void SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, PASESession & pairingCommissioner,
                                      TestSecurePairingDelegate & delegateCommissioner)
{
    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegateAccessory;
    PASESession pairingAccessory;

    delegateCommissioner.peer = &pairingAccessory;
    delegateAccessory.peer    = &pairingCommissioner;

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(1), 0,
                                                   &delegateAccessory) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, Optional<NodeId>::Value(2), 1, 0,
                                            &delegateCommissioner) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumMessageSend == 2);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);

    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumMessageSend == 3);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);
}

void SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, delegateCommissioner);
}

void SecurePairingDeserialize(nlTestSuite * inSuite, void * inContext, PASESession & pairingCommissioner,
                              PASESession & deserialized)
{
    PASESessionSerialized serialized;
    NL_TEST_ASSERT(inSuite, pairingCommissioner.Serialize(serialized) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, deserialized.Deserialize(serialized) == CHIP_NO_ERROR);

    // Serialize from the deserialized session, and check we get the same string back
    PASESessionSerialized serialized2;
    NL_TEST_ASSERT(inSuite, deserialized.Serialize(serialized2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strncmp(Uint8::to_char(serialized.inner), Uint8::to_char(serialized2.inner), sizeof(serialized)) == 0);
}

void SecurePairingSerializeTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;

    // Allocate on the heap to avoid stack overflow in some restricted test scenarios (e.g. QEMU)
    auto * testPairingSession1 = chip::Platform::New<PASESession>();
    auto * testPairingSession2 = chip::Platform::New<PASESession>();

    SecurePairingHandshakeTestCommon(inSuite, inContext, *testPairingSession1, delegateCommissioner);
    SecurePairingDeserialize(inSuite, inContext, *testPairingSession1, *testPairingSession2);

    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t encrypted[64];
    PacketHeader header;
    MessageAuthenticationCode mac;

    // Let's try encrypting using original session, and decrypting using deserialized
    {
        SecureSession session1;

        NL_TEST_ASSERT(inSuite,
                       testPairingSession1->DeriveSecureSession(Uint8::from_const_char("abc"), 3, session1) == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, session1.Encrypt(plain_text, sizeof(plain_text), encrypted, header, mac) == CHIP_NO_ERROR);
    }

    {
        SecureSession session2;
        NL_TEST_ASSERT(inSuite,
                       testPairingSession2->DeriveSecureSession(Uint8::from_const_char("abc"), 3, session2) == CHIP_NO_ERROR);

        uint8_t decrypted[64];
        NL_TEST_ASSERT(inSuite, session2.Decrypt(encrypted, sizeof(plain_text), decrypted, header, mac) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(plain_text, decrypted, sizeof(plain_text)) == 0);
    }

    chip::Platform::Delete(testPairingSession1);
    chip::Platform::Delete(testPairingSession2);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("WaitInit",    SecurePairingWaitTest),
    NL_TEST_DEF("Start",       SecurePairingStartTest),
    NL_TEST_DEF("Handshake",   SecurePairingHandshakeTest),
    NL_TEST_DEF("Serialize",   SecurePairingSerializeTest),

    NL_TEST_SENTINEL()
};
// clang-format on
//
/**
 *  Set up the test suite.
 */
int TestSecurePairing_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestSecurePairing_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing",
    &sTests[0],
    TestSecurePairing_Setup,
    TestSecurePairing_Teardown,
};
// clang-format on

/**
 *  Main
 */
int TestPASESession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPASESession)

namespace chip {
namespace Logging {
void LogV(uint8_t module, uint8_t category, const char * format, va_list argptr)
{
    (void) module, (void) category;
    vfprintf(stderr, format, argptr);
    fprintf(stderr, "\n");
}
} // namespace Logging
} // namespace chip
