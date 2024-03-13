/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <app/icd/server/ICDServerConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/secure_channel/PASESession.h>
#include <stdarg.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDConfigurationData.h> // nogncheck
#endif

// This test suite pushes multiple PASESession objects onto the stack for the
// purposes of testing device-to-device communication.  However, in the real
// world, these won't live in a single device's memory.  Hence, disable stack
// warning.
#pragma GCC diagnostic ignored "-Wstack-usage="

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

namespace {

#if CHIP_CONFIG_SLOW_CRYPTO
constexpr uint32_t sTestPaseMessageCount = 8;
#else  // CHIP_CONFIG_SLOW_CRYPTO
constexpr uint32_t sTestPaseMessageCount = 5;
#endif // CHIP_CONFIG_SLOW_CRYPTO

// Test Set #01 of Spake2p Parameters (PIN Code, Iteration Count, Salt, and matching Verifier):
constexpr uint32_t sTestSpake2p01_PinCode        = 20202021;
constexpr uint32_t sTestSpake2p01_IterationCount = 1000;
constexpr uint8_t sTestSpake2p01_Salt[]          = { 0x53, 0x50, 0x41, 0x4B, 0x45, 0x32, 0x50, 0x20,
                                                     0x4B, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6C, 0x74 };
constexpr Spake2pVerifier sTestSpake2p01_PASEVerifier = { .mW0 = {
    0xB9, 0x61, 0x70, 0xAA, 0xE8, 0x03, 0x34, 0x68, 0x84, 0x72, 0x4F, 0xE9, 0xA3, 0xB2, 0x87, 0xC3,
    0x03, 0x30, 0xC2, 0xA6, 0x60, 0x37, 0x5D, 0x17, 0xBB, 0x20, 0x5A, 0x8C, 0xF1, 0xAE, 0xCB, 0x35,
},
    .mL  = {
    0x04, 0x57, 0xF8, 0xAB, 0x79, 0xEE, 0x25, 0x3A, 0xB6, 0xA8, 0xE4, 0x6B, 0xB0, 0x9E, 0x54, 0x3A,
    0xE4, 0x22, 0x73, 0x6D, 0xE5, 0x01, 0xE3, 0xDB, 0x37, 0xD4, 0x41, 0xFE, 0x34, 0x49, 0x20, 0xD0,
    0x95, 0x48, 0xE4, 0xC1, 0x82, 0x40, 0x63, 0x0C, 0x4F, 0xF4, 0x91, 0x3C, 0x53, 0x51, 0x38, 0x39,
    0xB7, 0xC0, 0x7F, 0xCC, 0x06, 0x27, 0xA1, 0xB8, 0x57, 0x3A, 0x14, 0x9F, 0xCD, 0x1F, 0xA4, 0x66,
    0xCF
} };
constexpr Spake2pVerifierSerialized sTestSpake2p01_SerializedVerifier = {
    0xB9, 0x61, 0x70, 0xAA, 0xE8, 0x03, 0x34, 0x68, 0x84, 0x72, 0x4F, 0xE9, 0xA3, 0xB2, 0x87, 0xC3, 0x03, 0x30, 0xC2, 0xA6,
    0x60, 0x37, 0x5D, 0x17, 0xBB, 0x20, 0x5A, 0x8C, 0xF1, 0xAE, 0xCB, 0x35, 0x04, 0x57, 0xF8, 0xAB, 0x79, 0xEE, 0x25, 0x3A,
    0xB6, 0xA8, 0xE4, 0x6B, 0xB0, 0x9E, 0x54, 0x3A, 0xE4, 0x22, 0x73, 0x6D, 0xE5, 0x01, 0xE3, 0xDB, 0x37, 0xD4, 0x41, 0xFE,
    0x34, 0x49, 0x20, 0xD0, 0x95, 0x48, 0xE4, 0xC1, 0x82, 0x40, 0x63, 0x0C, 0x4F, 0xF4, 0x91, 0x3C, 0x53, 0x51, 0x38, 0x39,
    0xB7, 0xC0, 0x7F, 0xCC, 0x06, 0x27, 0xA1, 0xB8, 0x57, 0x3A, 0x14, 0x9F, 0xCD, 0x1F, 0xA4, 0x66, 0xCF
};

class TestContext : public chip::Test::LoopbackMessagingContext
{
public:
    // Performs shared setup for all tests in the test suite
    CHIP_ERROR SetUpTestSuite() override
    {
        ConfigInitializeNodes(false);
        return chip::Test::LoopbackMessagingContext::SetUpTestSuite();
    }
};

class PASETestLoopbackTransportDelegate : public Test::LoopbackTransportDelegate
{
public:
    void OnMessageDropped() override { mMessageDropped = true; }
    bool mMessageDropped = false;
};

class TestSecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished(const SessionHandle & session) override { mNumPairingComplete++; }

    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
};

class MockAppDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}
};

class TemporarySessionManager
{
public:
    TemporarySessionManager(nlTestSuite * suite, TestContext & ctx) : mCtx(ctx)
    {
        NL_TEST_ASSERT(suite,
                       CHIP_NO_ERROR ==
                           mSessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &ctx.GetMessageCounterManager(),
                                                &mStorage, &ctx.GetFabricTable(), ctx.GetSessionKeystore()));
        // The setup here is really weird: we are using one session manager for
        // the actual messages we send (the PASE handshake, so the
        // unauthenticated sessions) and a different one for allocating the PASE
        // sessions.  Since our Init() set us up as the thing to handle messages
        // on the transport manager, undo that.
        mCtx.GetTransportMgr().SetSessionManager(&mCtx.GetSecureSessionManager());
    }

    ~TemporarySessionManager()
    {
        mSessionManager.Shutdown();
        // Reset the session manager on the transport again, just in case
        // shutdown messed with it.
        mCtx.GetTransportMgr().SetSessionManager(&mCtx.GetSecureSessionManager());
    }

    operator SessionManager &() { return mSessionManager; }

private:
    TestContext & mCtx;
    TestPersistentStorageDelegate mStorage;
    SessionManager mSessionManager;
};

using namespace System::Clock::Literals;

void SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TemporarySessionManager sessionManager(inSuite, ctx);

    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;
    PASESession pairing;

    NL_TEST_ASSERT(inSuite, pairing.GetSecureSessionType() == SecureSession::Type::kPASE);

    auto & loopback = ctx.GetLoopback();
    loopback.Reset();

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(sessionManager, sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount, ByteSpan(),
                                          Optional<ReliableMessageProtocolConfig>::Missing(),
                                          &delegate) == CHIP_ERROR_INVALID_ARGUMENT);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(sessionManager, sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount,
                                          ByteSpan(reinterpret_cast<const uint8_t *>("saltSalt"), 8),
                                          Optional<ReliableMessageProtocolConfig>::Missing(),
                                          nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(sessionManager, sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount,
                                          ByteSpan(reinterpret_cast<const uint8_t *>("saltSalt"), 8),
                                          Optional<ReliableMessageProtocolConfig>::Missing(),
                                          &delegate) == CHIP_ERROR_INVALID_ARGUMENT);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(sessionManager, sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount,
                                          ByteSpan(sTestSpake2p01_Salt), Optional<ReliableMessageProtocolConfig>::Missing(),
                                          &delegate) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();
}

void SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TemporarySessionManager sessionManager(inSuite, ctx);

    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;
    PASESession pairing;

    auto & loopback = ctx.GetLoopback();
    loopback.Reset();

    ExchangeContext * context = ctx.NewUnauthenticatedExchangeToBob(&pairing);

    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(sessionManager, sTestSpake2p01_PinCode, Optional<ReliableMessageProtocolConfig>::Missing(), nullptr,
                                nullptr) != CHIP_NO_ERROR);

    loopback.Reset();
    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(sessionManager, sTestSpake2p01_PinCode, Optional<ReliableMessageProtocolConfig>::Missing(), context,
                                &delegate) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // There should have been two messages sent: PBKDFParamRequest and an ack.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    loopback.Reset();
    loopback.mSentMessageCount = 0;
    loopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    PASESession pairing1;
    ExchangeContext * context1 = ctx.NewUnauthenticatedExchangeToBob(&pairing1);
    NL_TEST_ASSERT(inSuite,
                   pairing1.Pair(sessionManager, sTestSpake2p01_PinCode, Optional<ReliableMessageProtocolConfig>::Missing(),
                                 context1, &delegate) == CHIP_ERROR_BAD_REQUEST);
    ctx.DrainAndServiceIO();

    loopback.mMessageSendError = CHIP_NO_ERROR;
}

void SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, SessionManager & sessionManager,
                                      PASESession & pairingCommissioner,
                                      Optional<ReliableMessageProtocolConfig> mrpCommissionerConfig,
                                      Optional<ReliableMessageProtocolConfig> mrpAccessoryConfig,
                                      TestSecurePairingDelegate & delegateCommissioner)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TestSecurePairingDelegate delegateAccessory;
    PASESession pairingAccessory;

    PASETestLoopbackTransportDelegate delegate;
    auto & loopback = ctx.GetLoopback();
    loopback.SetLoopbackTransportDelegate(&delegate);
    loopback.mSentMessageCount = 0;

    ExchangeContext * contextCommissioner = ctx.NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    if (loopback.mNumMessagesToDrop != 0)
    {
        ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
        ReliableMessageContext * rc = contextCommissioner->GetReliableMessageContext();
        NL_TEST_ASSERT(inSuite, rm != nullptr);
        NL_TEST_ASSERT(inSuite, rc != nullptr);

        // Adding an if-else to avoid affecting non-ICD tests
#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
        // Increase local MRP retry intervals to take into account the increase response delay from an ICD
        contextCommissioner->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(
            ReliableMessageProtocolConfig({
                1000_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
                1000_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
            }));
#else  // CHIP_CONFIG_ENABLE_ICD_SERVER != 1
        contextCommissioner->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(
            ReliableMessageProtocolConfig({
                64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
                64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
            }));
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
    }

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::PBKDFParamRequest, &pairingAccessory) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(sessionManager, sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount,
                                                   ByteSpan(sTestSpake2p01_Salt), mrpAccessoryConfig,
                                                   &delegateAccessory) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(sessionManager, sTestSpake2p01_PinCode, mrpCommissionerConfig, contextCommissioner,
                                            &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    while (delegate.mMessageDropped)
    {
        auto waitTimeout = 100_ms + CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST;

#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
        // If running as an ICD, increase waitTimeout to account for the polling interval
        waitTimeout += ICDConfigurationData::GetInstance().GetSlowPollingInterval();
#endif

        // Wait some time so the dropped message will be retransmitted when we drain the IO.
        chip::test_utils::SleepMillis(waitTimeout.count());
        delegate.mMessageDropped = false;
        ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), ctx.GetExchangeManager().GetReliableMessageMgr());
        ctx.DrainAndServiceIO();
    };

    // Standalone acks also increment the mSentMessageCount. But some messages could be acked
    // via piggybacked acks. So we cannot check for a specific value of mSentMessageCount.
    // Let's make sure atleast number is >= than the minimum messages required to complete the
    // handshake.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount >= sTestPaseMessageCount);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingErrors == 0);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingErrors == 0);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);

    if (mrpCommissionerConfig.HasValue())
    {
        NL_TEST_ASSERT(inSuite,
                       pairingAccessory.GetRemoteMRPConfig().mIdleRetransTimeout ==
                           mrpCommissionerConfig.Value().mIdleRetransTimeout);
        NL_TEST_ASSERT(inSuite,
                       pairingAccessory.GetRemoteMRPConfig().mActiveRetransTimeout ==
                           mrpCommissionerConfig.Value().mActiveRetransTimeout);
    }

    if (mrpAccessoryConfig.HasValue())
    {
        NL_TEST_ASSERT(inSuite,
                       pairingCommissioner.GetRemoteMRPConfig().mIdleRetransTimeout ==
                           mrpAccessoryConfig.Value().mIdleRetransTimeout);
        NL_TEST_ASSERT(inSuite,
                       pairingCommissioner.GetRemoteMRPConfig().mActiveRetransTimeout ==
                           mrpAccessoryConfig.Value().mActiveRetransTimeout);
    }

    // Now evict the PASE sessions.
    auto session = pairingCommissioner.CopySecureSession();
    NL_TEST_ASSERT(inSuite, session.HasValue());
    session.Value()->AsSecureSession()->MarkForEviction();

    session = pairingAccessory.CopySecureSession();
    NL_TEST_ASSERT(inSuite, session.HasValue());
    session.Value()->AsSecureSession()->MarkForEviction();

    // Evicting a session async notifies the PASESession's delegate.  Normally
    // that notification is what would delete the PASESession, but in our case
    // that will happen as soon as things come off the stack.  So make sure to
    // process the async bits before that happens.
    ctx.DrainAndServiceIO();

    // And check that this did not result in any new notifications.
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingErrors == 0);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingErrors == 0);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);

    loopback.SetLoopbackTransportDelegate(nullptr);
}

void SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TemporarySessionManager sessionManager(inSuite, ctx);

    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    auto & loopback = ctx.GetLoopback();
    loopback.Reset();
    SecurePairingHandshakeTestCommon(inSuite, inContext, sessionManager, pairingCommissioner,
                                     Optional<ReliableMessageProtocolConfig>::Missing(),
                                     Optional<ReliableMessageProtocolConfig>::Missing(), delegateCommissioner);
}

void SecurePairingHandshakeWithCommissionerMRPTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TemporarySessionManager sessionManager(inSuite, ctx);

    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    auto & loopback = ctx.GetLoopback();
    loopback.Reset();
    ReliableMessageProtocolConfig config(1000_ms32, 10000_ms32, 4000_ms16);
    SecurePairingHandshakeTestCommon(inSuite, inContext, sessionManager, pairingCommissioner,
                                     Optional<ReliableMessageProtocolConfig>::Value(config),
                                     Optional<ReliableMessageProtocolConfig>::Missing(), delegateCommissioner);
}

void SecurePairingHandshakeWithDeviceMRPTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TemporarySessionManager sessionManager(inSuite, ctx);

    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    auto & loopback = ctx.GetLoopback();
    loopback.Reset();
    ReliableMessageProtocolConfig config(1000_ms32, 10000_ms32, 4000_ms16);
    SecurePairingHandshakeTestCommon(inSuite, inContext, sessionManager, pairingCommissioner,
                                     Optional<ReliableMessageProtocolConfig>::Missing(),
                                     Optional<ReliableMessageProtocolConfig>::Value(config), delegateCommissioner);
}

void SecurePairingHandshakeWithAllMRPTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TemporarySessionManager sessionManager(inSuite, ctx);

    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    auto & loopback = ctx.GetLoopback();
    loopback.Reset();
    ReliableMessageProtocolConfig commissionerConfig(1000_ms32, 10000_ms32, 4000_ms16);
    ReliableMessageProtocolConfig deviceConfig(2000_ms32, 7000_ms32, 4000_ms16);
    SecurePairingHandshakeTestCommon(inSuite, inContext, sessionManager, pairingCommissioner,
                                     Optional<ReliableMessageProtocolConfig>::Value(commissionerConfig),
                                     Optional<ReliableMessageProtocolConfig>::Value(deviceConfig), delegateCommissioner);
}

void SecurePairingHandshakeWithPacketLossTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TemporarySessionManager sessionManager(inSuite, ctx);

    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    auto & loopback = ctx.GetLoopback();
    loopback.Reset();
    loopback.mNumMessagesToDrop = 2;
    SecurePairingHandshakeTestCommon(inSuite, inContext, sessionManager, pairingCommissioner,
                                     Optional<ReliableMessageProtocolConfig>::Missing(),
                                     Optional<ReliableMessageProtocolConfig>::Missing(), delegateCommissioner);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
}

void SecurePairingFailedHandshake(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    TemporarySessionManager sessionManager(inSuite, ctx);

    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;

    TestSecurePairingDelegate delegateAccessory;
    PASESession pairingAccessory;

    auto & loopback = ctx.GetLoopback();
    loopback.Reset();
    loopback.mSentMessageCount = 0;

    ExchangeContext * contextCommissioner = ctx.NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = contextCommissioner->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    contextCommissioner->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::PBKDFParamRequest, &pairingAccessory) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(
                       sessionManager, sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount, ByteSpan(sTestSpake2p01_Salt),
                       Optional<ReliableMessageProtocolConfig>::Missing(), &delegateAccessory) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(sessionManager, 4321, Optional<ReliableMessageProtocolConfig>::Missing(),
                                            contextCommissioner, &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 0);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingErrors == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 0);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingErrors == 1);
}

void PASEVerifierSerializeTest(nlTestSuite * inSuite, void * inContext)
{
    Spake2pVerifier verifier;
    NL_TEST_ASSERT(inSuite, verifier.Deserialize(ByteSpan(sTestSpake2p01_SerializedVerifier)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(&verifier, &sTestSpake2p01_PASEVerifier, sizeof(Spake2pVerifier)) == 0);

    Spake2pVerifierSerialized serializedVerifier;
    MutableByteSpan serializedVerifierSpan(serializedVerifier);
    NL_TEST_ASSERT(inSuite, verifier.Serialize(serializedVerifierSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, serializedVerifierSpan.size() == kSpake2p_VerifierSerialized_Length);
    NL_TEST_ASSERT(inSuite, memcmp(serializedVerifier, sTestSpake2p01_SerializedVerifier, kSpake2p_VerifierSerialized_Length) == 0);

    Spake2pVerifierSerialized serializedVerifier2;
    MutableByteSpan serializedVerifier2Span(serializedVerifier2);
    NL_TEST_ASSERT(inSuite, chip::Crypto::DRBG_get_bytes(serializedVerifier, kSpake2p_VerifierSerialized_Length) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, verifier.Deserialize(ByteSpan(serializedVerifier)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, verifier.Serialize(serializedVerifier2Span) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(serializedVerifier, serializedVerifier2, kSpake2p_VerifierSerialized_Length) == 0);
}

// Test Suite

static const nlTest sTests[] = {
    NL_TEST_DEF("WaitInit", SecurePairingWaitTest),
    NL_TEST_DEF("Start", SecurePairingStartTest),
    NL_TEST_DEF("Handshake", SecurePairingHandshakeTest),
    NL_TEST_DEF("Handshake with Commissioner MRP Parameters", SecurePairingHandshakeWithCommissionerMRPTest),
    NL_TEST_DEF("Handshake with Device MRP Parameters", SecurePairingHandshakeWithDeviceMRPTest),
    NL_TEST_DEF("Handshake with Both MRP Parameters", SecurePairingHandshakeWithAllMRPTest),
    NL_TEST_DEF("Handshake with packet loss", SecurePairingHandshakeWithPacketLossTest),
    NL_TEST_DEF("Failed Handshake", SecurePairingFailedHandshake),
    NL_TEST_DEF("PASE Verifier Serialize", PASEVerifierSerializeTest),
    NL_TEST_SENTINEL(),
};

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing-PASE",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
// clang-format on

} // anonymous namespace

/**
 *  Main
 */
int TestPASESession()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPASESession)
