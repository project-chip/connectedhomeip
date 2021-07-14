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
#include <transport/raw/tests/NetworkTestHelpers.h>

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::MessagingContext;

static void test_os_sleep_ms(uint64_t millisecs)
{
    struct timespec sleep_time;
    uint64_t s = millisecs / 1000;

    millisecs -= s * 1000;
    sleep_time.tv_sec  = static_cast<time_t>(s);
    sleep_time.tv_nsec = static_cast<long>(millisecs * 1000000);

    nanosleep(&sleep_time, nullptr);
}

class PASETestLoopbackTransport : public Test::LoopbackTransport
{
    void MessageDropped() override
    {
        // Trigger a retransmit.
        if (mContext != nullptr)
        {
            test_os_sleep_ms(65);
            ReliableMessageMgr * rm = mContext->GetExchangeManager().GetReliableMessageMgr();
            ReliableMessageMgr::Timeout(&mContext->GetSystemLayer(), rm, CHIP_NO_ERROR);
        }
    }

public:
    bool CanSendToPeer(const PeerAddress & address) override { return true; }

    void Reset()
    {
        Test::LoopbackTransport::Reset();
        mContext = nullptr;
    }

    TestContext * mContext = nullptr;
};

TransportMgrBase gTransportMgr;
PASETestLoopbackTransport gLoopback;

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
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}
};

void SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;
    PASESession pairing;

    gLoopback.Reset();

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

    gLoopback.Reset();

    NL_TEST_ASSERT(inSuite, pairing.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    ExchangeContext * context = ctx.NewExchangeToLocal(&pairing);

    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, 0, nullptr, nullptr) != CHIP_NO_ERROR);

    gLoopback.Reset();
    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, 0, context, &delegate) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);

    gLoopback.Reset();
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

    ExchangeContext * contextCommissioner = ctx.NewExchangeToLocal(&pairingCommissioner);

    if (gLoopback.mNumMessagesToDrop != 0)
    {
        pairingCommissioner.MessageDispatch().SetPeerAddress(PeerAddress(Type::kUdp));
        pairingAccessory.MessageDispatch().SetPeerAddress(PeerAddress(Type::kUdp));

        ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
        ReliableMessageContext * rc = contextCommissioner->GetReliableMessageContext();
        NL_TEST_ASSERT(inSuite, rm != nullptr);
        NL_TEST_ASSERT(inSuite, rc != nullptr);

        rc->SetConfig({
            1, // CHIP_CONFIG_MRP_DEFAULT_INITIAL_RETRY_INTERVAL
            1, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
        });
        gLoopback.mContext = &ctx;
    }

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::PBKDFParamRequest, &pairingAccessory) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(1234, 500, (const uint8_t *) "saltSALT", 8, 0, &delegateAccessory) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(Transport::PeerAddress(Transport::Type::kBle), 1234, 0, contextCommissioner,
                                            &delegateCommissioner) == CHIP_NO_ERROR);

    // Standalone acks also increment the mSentMessageCount. But some messages could be acked
    // via piggybacked acks. So we cannot check for a specific value of mSentMessageCount.
    // Let's make sure atleast number is >= than the minimum messages required to complete the
    // handshake.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount >= 5);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);
    gLoopback.mContext = nullptr;
}

void SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    gLoopback.Reset();
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, delegateCommissioner);
}

void SecurePairingHandshakeWithPacketLossTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    gLoopback.Reset();
    gLoopback.mNumMessagesToDrop = 2;
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, delegateCommissioner);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 0);
}

void SecurePairingFailedHandshake(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;

    TestSecurePairingDelegate delegateAccessory;
    PASESession pairingAccessory;

    gLoopback.Reset();
    gLoopback.mSentMessageCount = 0;

    NL_TEST_ASSERT(inSuite, pairingCommissioner.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, pairingAccessory.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);

    TestContext & ctx                     = *reinterpret_cast<TestContext *>(inContext);
    ExchangeContext * contextCommissioner = ctx.NewExchangeToLocal(&pairingCommissioner);

    pairingCommissioner.MessageDispatch().SetPeerAddress(PeerAddress(Type::kUdp));
    pairingAccessory.MessageDispatch().SetPeerAddress(PeerAddress(Type::kUdp));

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = contextCommissioner->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_MRP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });
    gLoopback.mContext = &ctx;

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::PBKDFParamRequest, &pairingAccessory) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(1234, 500, (const uint8_t *) "saltSALT", 8, 0, &delegateAccessory) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(Transport::PeerAddress(Transport::Type::kBle), 4321, 0, contextCommissioner,
                                            &delegateCommissioner) == CHIP_NO_ERROR);

    gLoopback.mContext = nullptr;
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 0);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingErrors == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 0);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingErrors == 1);
}

void SecurePairingDeserialize(nlTestSuite * inSuite, void * inContext, PASESession & pairingCommissioner,
                              PASESession & deserialized)
{
    PASESessionSerialized serialized;
    gLoopback.Reset();
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

    gLoopback.Reset();

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
    NL_TEST_DEF("Handshake with packet loss", SecurePairingHandshakeWithPacketLossTest),
    NL_TEST_DEF("Failed Handshake", SecurePairingFailedHandshake),
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
