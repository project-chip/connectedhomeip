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
#include <transport/SecurePairingSession.h>

#include <stdarg.h>
#include <support/CodeUtils.h>
#include <support/TestUtils.h>

using namespace chip;

class TestSecurePairingDelegate : public SecurePairingSessionDelegate
{
public:
    virtual CHIP_ERROR OnNewMessageForPeer(System::PacketBuffer * msgBuf)
    {
        mNumMessageSend++;
        if (peer != nullptr)
        {
            MessageHeader hdr;
            size_t headerSize = 0;

            hdr.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
            msgBuf->ConsumeHead(headerSize);

            return peer->HandlePeerMessage(hdr, msgBuf);
        }
        return mMessageSendError;
    }

    virtual void OnPairingError(CHIP_ERROR error) { mNumPairingErrors++; }

    virtual void OnPairingComplete(Optional<NodeId> peerNodeId, uint16_t peerKeyId, uint16_t localKeyId) { mNumPairingComplete++; }

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

void SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegateAccessory, deleageCommissioner;
    SecurePairingSession pairingAccessory, pairingCommissioner;

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

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing",
    &sTests[0],
    NULL,
    NULL
};
// clang-format on

/**
 *  Main
 */
int TestSecurePairingSession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, NULL);

    return (nlTestRunnerStats(&sSuite));
}

static void __attribute__((constructor)) TestSecurePairingSessionCtor(void)
{
    VerifyOrDie(RegisterUnitTests(&TestSecurePairingSession) == CHIP_NO_ERROR);
}

namespace chip {
namespace Logging {
void LogV(uint8_t module, uint8_t category, const char * format, va_list argptr)
{
    (void) module, (void) category;
    vfprintf(stderr, format, argptr);
}
} // namespace Logging
} // namespace chip
