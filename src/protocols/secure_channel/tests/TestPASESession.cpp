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
#include <messaging/tests/MessagingContext.h>
#include <protocols/secure_channel/PASESession.h>
#include <stdarg.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::MessagingContext;

class LoopbackTransport : public Transport::Base
{
public:
    CHIP_ERROR SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf) override
    {
        ReturnErrorOnFailure(mMessageSendError);
        mSentMessageCount++;
        HandleMessageReceived(address, std::move(msgBuf));

        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }

    uint32_t mSentMessageCount   = 0;
    CHIP_ERROR mMessageSendError = CHIP_NO_ERROR;
};

TransportMgrBase gTransportMgr;
LoopbackTransport gLoopback;

class TestSecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished() override { mNumPairingComplete++; }

    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
};

class MockAppDelegate : public ExchangeDelegate
{
public:
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && buffer) override
    {
        ec->Close();
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}
};

void SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;
    PASESession pairing;

    NL_TEST_ASSERT(inSuite, pairing.WaitForPairing(1234, 500, nullptr, 0, 0, &delegate) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(1234, 500, (const uint8_t *) "saltSalt", 8, 0, nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, pairing.WaitForPairing(1234, 500, (const uint8_t *) "saltSalt", 8, 0, &delegate) == CHIP_NO_ERROR);
}

void SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;

    PASESession pairing;

    NL_TEST_ASSERT(inSuite, pairing.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    ExchangeContext * context = ctx.NewExchangeToLocal(&pairing);

    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, 0, nullptr, nullptr) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, 0, context, &delegate) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);

    gLoopback.mSentMessageCount = 0;
    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    PASESession pairing1;
    NL_TEST_ASSERT(inSuite, pairing1.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    ExchangeContext * context1 = ctx.NewExchangeToLocal(&pairing1);
    NL_TEST_ASSERT(inSuite,
                   pairing1.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, 0, context1, &delegate) ==
                       CHIP_ERROR_BAD_REQUEST);
    gLoopback.mMessageSendError = CHIP_NO_ERROR;
}

void SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, PASESession & pairingCommissioner,
                                      TestSecurePairingDelegate & delegateCommissioner)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TestSecurePairingDelegate delegateAccessory;
    PASESession pairingAccessory;

    gLoopback.mSentMessageCount = 0;

    NL_TEST_ASSERT(inSuite, pairingCommissioner.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, pairingAccessory.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::PBKDFParamRequest, &pairingAccessory) == CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = ctx.NewExchangeToLocal(&pairingCommissioner);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(1234, 500, (const uint8_t *) "saltSALT", 8, 0, &delegateAccessory) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, 0, contextCommissioner,
                                            &delegateCommissioner) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 5);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);
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

        CHIP_ERROR err = testPairingSession1->DeriveSecureSession(session1, SecureSession::SessionRole::kInitiator);

        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = session1.Encrypt(plain_text, sizeof(plain_text), encrypted, header, mac);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    {
        SecureSession session2;
        NL_TEST_ASSERT(inSuite,
                       testPairingSession2->DeriveSecureSession(session2, SecureSession::SessionRole::kResponder) == CHIP_NO_ERROR);

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

int TestSecurePairing_Setup(void * inContext);
int TestSecurePairing_Teardown(void * inContext);

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing",
    &sTests[0],
    TestSecurePairing_Setup,
    TestSecurePairing_Teardown,
};
// clang-format on

static TestContext sContext;

// clang-format on
//
/**
 *  Set up the test suite.
 */
int TestSecurePairing_Setup(void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    CHIP_ERROR err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
        return FAILURE;

    gTransportMgr.Init(&gLoopback);

    err = ctx.Init(&sSuite, &gTransportMgr);
    if (err != CHIP_NO_ERROR)
        return FAILURE;

    ctx.SetSourceNodeId(kAnyNodeId);
    ctx.SetDestinationNodeId(kAnyNodeId);
    ctx.SetLocalKeyId(0);
    ctx.SetPeerKeyId(0);
    ctx.SetAdminId(kUndefinedAdminId);

    gTransportMgr.SetSecureSessionMgr(&ctx.GetSecureSessionManager());

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestSecurePairing_Teardown(void * inContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(inContext)->Shutdown();
    chip::Platform::MemoryShutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

// TODO: TestPASESession is currently disabled due to lacking convenient way of creating exchange context

/**
 *  Main
 */
int TestPASESession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPASESession)
