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
 *      This file implements unit tests for the SecurePairingSession implementation.
 */

#include "TestTransportLayer.h"

#include <errno.h>
#include <nlunit-test.h>

#include <core/CHIPCore.h>
#include <core/CHIPSafeCasts.h>
#include <transport/SecurePairingSession.h>

#include <stdarg.h>
#include <support/CodeUtils.h>
#include <support/TestUtils.h>

using namespace chip;

class TestSecurePairingDelegate : public SecurePairingSessionDelegate
{
public:
    CHIP_ERROR SendMessage(System::PacketBuffer * msgBuf) override
    {
        mNumMessageSend++;
        if (peer != nullptr)
        {
            PacketHeader hdr;
            uint16_t headerSize = 0;

            hdr.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
            msgBuf->ConsumeHead(headerSize);

            return peer->HandlePeerMessage(hdr, msgBuf);
        }
        return mMessageSendError;
    }

    void OnPairingError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnPairingComplete() override { mNumPairingComplete++; }

    uint32_t mNumMessageSend     = 0;
    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
    CHIP_ERROR mMessageSendError = CHIP_NO_ERROR;

    SecurePairingSession * peer = nullptr;
};

void SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;
    SecurePairingSession pairing;

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
    SecurePairingSession pairing;

    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(1234, 500, nullptr, 0, Optional<NodeId>::Value(1), 0, &delegate) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(1), 0, nullptr) ==
                       CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(1), 0, &delegate) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, delegate.mNumMessageSend == 1);

    delegate.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    SecurePairingSession pairing1;

    NL_TEST_ASSERT(inSuite,
                   pairing1.Pair(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(1), 0, &delegate) ==
                       CHIP_ERROR_BAD_REQUEST);
}

void SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, SecurePairingSession & pairingCommissioner)
{
    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegateAccessory, deleageCommissioner;
    SecurePairingSession pairingAccessory;

    deleageCommissioner.peer = &pairingAccessory;
    delegateAccessory.peer   = &pairingCommissioner;

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(1), 0,
                                                   &delegateAccessory) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(2), 0,
                                            &deleageCommissioner) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumMessageSend == 1);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);

    NL_TEST_ASSERT(inSuite, deleageCommissioner.mNumMessageSend == 2);
    NL_TEST_ASSERT(inSuite, deleageCommissioner.mNumPairingComplete == 1);
}

void SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    SecurePairingSession pairingCommissioner;
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner);
}

void SecurePairingDeserialize(nlTestSuite * inSuite, void * inContext, SecurePairingSession & pairingCommissioner,
                              SecurePairingSession & deserialized)
{
    SecurePairingSessionSerialized serialized;
    NL_TEST_ASSERT(inSuite, pairingCommissioner.Serialize(serialized) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, deserialized.Deserialize(serialized) == CHIP_NO_ERROR);

    // Serialize from the deserialized session, and check we get the same string back
    SecurePairingSessionSerialized serialized2;
    NL_TEST_ASSERT(inSuite, deserialized.Serialize(serialized2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strncmp(Uint8::to_char(serialized.inner), Uint8::to_char(serialized2.inner), sizeof(serialized)) == 0);
}

// Defining these globally to avoid stack overflow in some restricted test scenarios (e.g. QEMU)
static SecurePairingSession gTestPairingSession1, gTestPairingSession2;

void SecurePairingSerializeTest(nlTestSuite * inSuite, void * inContext)
{
    SecurePairingHandshakeTestCommon(inSuite, inContext, gTestPairingSession1);
    SecurePairingDeserialize(inSuite, inContext, gTestPairingSession1, gTestPairingSession2);

    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t encrypted[64];
    PacketHeader header;
    MessageAuthenticationCode mac;

    // Let's try encrypting using original session, and decrypting using deserialized
    {
        SecureSession session1;

        NL_TEST_ASSERT(inSuite,
                       gTestPairingSession1.DeriveSecureSession(Uint8::from_const_char("abc"), 3, session1) == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite,
                       session1.Encrypt(plain_text, sizeof(plain_text), encrypted, header, Header::Flags(), mac) == CHIP_NO_ERROR);
    }

    {
        SecureSession session2;
        NL_TEST_ASSERT(inSuite,
                       gTestPairingSession2.DeriveSecureSession(Uint8::from_const_char("abc"), 3, session2) == CHIP_NO_ERROR);

        uint8_t decrypted[64];
        NL_TEST_ASSERT(inSuite,
                       session2.Decrypt(encrypted, sizeof(plain_text), decrypted, header, Header::Flags(), mac) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(plain_text, decrypted, sizeof(plain_text)) == 0);
    }
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

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing",
    &sTests[0],
    nullptr,
    nullptr
};
// clang-format on

/**
 *  Main
 */
int TestSecurePairingSession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSecurePairingSession)

namespace chip {
namespace Logging {
void LogV(uint8_t module, uint8_t category, const char * format, va_list argptr)
{
    (void) module, (void) category;
    vfprintf(stderr, format, argptr);
}
} // namespace Logging
} // namespace chip
