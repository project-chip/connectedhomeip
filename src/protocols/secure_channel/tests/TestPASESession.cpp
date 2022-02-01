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

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/secure_channel/PASESession.h>
#include <stdarg.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

namespace {

// Test Set #01 of Spake2p Parameters (PIN Code, Iteration Count, Salt, and matching Verifier):
constexpr uint32_t sTestSpake2p01_PinCode          = 20202021;
constexpr uint32_t sTestSpake2p01_IterationCount   = 1000;
constexpr uint8_t sTestSpake2p01_Salt[]            = { 0x53, 0x50, 0x41, 0x4B, 0x45, 0x32, 0x50, 0x20,
                                            0x4B, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6C, 0x74 };
constexpr PASEVerifier sTestSpake2p01_PASEVerifier = { .mW0 = { 0xab, 0xa6, 0x0c, 0x30, 0x41, 0x6b, 0x8f, 0x41, 0x77, 0xf5,
                                                                0xe1, 0x6a, 0xd5, 0x14, 0xcf, 0xd9, 0x57, 0x75, 0x13, 0xf0,
                                                                0x2f, 0xd6, 0x05, 0x06, 0xb1, 0x04, 0x9d, 0x0f, 0x2c, 0x73,
                                                                0x10, 0x01, 0x0e, 0x5e, 0x40, 0xbf, 0xd8, 0x6b, 0x4e, 0xf6 },
                                                       .mL  = { 0x81, 0xa8, 0x8b, 0x71, 0xe9, 0xe2, 0xa8, 0x53, 0x98, 0x5a,
                                                               0x7d, 0xef, 0x91, 0x6e, 0xa3, 0x0e, 0x01, 0xb8, 0x72, 0x2f,
                                                               0xbf, 0x7d, 0x0e, 0x38, 0x85, 0x6c, 0x12, 0xcd, 0x64, 0xc2,
                                                               0x25, 0xbb, 0x24, 0xef, 0x21, 0x41, 0x7e, 0x0e, 0x44, 0xe5 } };
constexpr PASEVerifierSerialized sTestSpake2p01_SerializedVerifier = {
    0xab, 0xa6, 0x0c, 0x30, 0x41, 0x6b, 0x8f, 0x41, 0x77, 0xf5, 0xe1, 0x6a, 0xd5, 0x14, 0xcf, 0xd9, 0x57, 0x75, 0x13, 0xf0,
    0x2f, 0xd6, 0x05, 0x06, 0xb1, 0x04, 0x9d, 0x0f, 0x2c, 0x73, 0x10, 0x01, 0x0e, 0x5e, 0x40, 0xbf, 0xd8, 0x6b, 0x4e, 0xf6,
    0x81, 0xa8, 0x8b, 0x71, 0xe9, 0xe2, 0xa8, 0x53, 0x98, 0x5a, 0x7d, 0xef, 0x91, 0x6e, 0xa3, 0x0e, 0x01, 0xb8, 0x72, 0x2f,
    0xbf, 0x7d, 0x0e, 0x38, 0x85, 0x6c, 0x12, 0xcd, 0x64, 0xc2, 0x25, 0xbb, 0x24, 0xef, 0x21, 0x41, 0x7e, 0x0e, 0x44, 0xe5
};

class PASETestLoopbackTransport : public Test::LoopbackTransport
{
    void MessageDropped() override { mMessageDropped = true; }

public:
    bool CanSendToPeer(const PeerAddress & address) override { return true; }

    bool mMessageDropped = false;
};

using TestContext = chip::Test::LoopbackMessagingContext<PASETestLoopbackTransport>;

TestContext sContext;
auto & gLoopback = sContext.GetLoopback();

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
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}
};

using namespace System::Clock::Literals;

void SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;
    PASESession pairing;

    NL_TEST_ASSERT(inSuite, pairing.GetSecureSessionType() == SecureSession::Type::kPASE);
    CATValues peerCATs;
    peerCATs = pairing.GetPeerCATs();
    NL_TEST_ASSERT(inSuite, memcmp(&peerCATs, &kUndefinedCATs, sizeof(CATValues)) == 0);

    gLoopback.Reset();

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount, ByteSpan(nullptr, 0),
                                          kDefaultCommissioningPasscodeId, 0, Optional<ReliableMessageProtocolConfig>::Missing(),
                                          &delegate) == CHIP_ERROR_INVALID_ARGUMENT);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount,
                                          ByteSpan(reinterpret_cast<const uint8_t *>("saltSalt"), 8),
                                          kDefaultCommissioningPasscodeId, 0, Optional<ReliableMessageProtocolConfig>::Missing(),
                                          nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount,
                                          ByteSpan(reinterpret_cast<const uint8_t *>("saltSalt"), 8),
                                          kDefaultCommissioningPasscodeId, 0, Optional<ReliableMessageProtocolConfig>::Missing(),
                                          &delegate) == CHIP_ERROR_INVALID_ARGUMENT);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForPairing(sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount, ByteSpan(sTestSpake2p01_Salt),
                                          kDefaultCommissioningPasscodeId, 0, Optional<ReliableMessageProtocolConfig>::Missing(),
                                          &delegate) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();
}

void SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestSecurePairingDelegate delegate;

    PASESession pairing;

    gLoopback.Reset();

    ExchangeContext * context = ctx.NewUnauthenticatedExchangeToBob(&pairing);

    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(Transport::PeerAddress(Transport::Type::kBle), sTestSpake2p01_PinCode, 0,
                                Optional<ReliableMessageProtocolConfig>::Missing(), nullptr, nullptr) != CHIP_NO_ERROR);

    gLoopback.Reset();
    NL_TEST_ASSERT(inSuite,
                   pairing.Pair(Transport::PeerAddress(Transport::Type::kBle), sTestSpake2p01_PinCode, 0,
                                Optional<ReliableMessageProtocolConfig>::Missing(), context, &delegate) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);

    // Clear pending packet in CRMP
    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = context->GetReliableMessageContext();
    rm->ClearRetransTable(rc);

    gLoopback.Reset();
    gLoopback.mSentMessageCount = 0;
    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    PASESession pairing1;
    ExchangeContext * context1 = ctx.NewUnauthenticatedExchangeToBob(&pairing1);
    NL_TEST_ASSERT(inSuite,
                   pairing1.Pair(Transport::PeerAddress(Transport::Type::kBle), sTestSpake2p01_PinCode, 0,
                                 Optional<ReliableMessageProtocolConfig>::Missing(), context1,
                                 &delegate) == CHIP_ERROR_BAD_REQUEST);
    ctx.DrainAndServiceIO();

    gLoopback.mMessageSendError = CHIP_NO_ERROR;
}

void SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, PASESession & pairingCommissioner,
                                      Optional<ReliableMessageProtocolConfig> mrpCommissionerConfig,
                                      Optional<ReliableMessageProtocolConfig> mrpAccessoryConfig,
                                      TestSecurePairingDelegate & delegateCommissioner)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TestSecurePairingDelegate delegateAccessory;
    PASESession pairingAccessory;

    gLoopback.mSentMessageCount = 0;

    ExchangeContext * contextCommissioner = ctx.NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    if (gLoopback.mNumMessagesToDrop != 0)
    {
        ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
        ReliableMessageContext * rc = contextCommissioner->GetReliableMessageContext();
        NL_TEST_ASSERT(inSuite, rm != nullptr);
        NL_TEST_ASSERT(inSuite, rc != nullptr);

        contextCommissioner->GetSessionHandle()->AsUnauthenticatedSession()->SetMRPConfig({
            64_ms32, // CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL
            64_ms32, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
        });
    }

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::PBKDFParamRequest, &pairingAccessory) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount,
                                                   ByteSpan(sTestSpake2p01_Salt), kDefaultCommissioningPasscodeId, 0,
                                                   mrpAccessoryConfig, &delegateAccessory) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(Transport::PeerAddress(Transport::Type::kBle), sTestSpake2p01_PinCode, 0,
                                            mrpCommissionerConfig, contextCommissioner, &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    while (gLoopback.mMessageDropped)
    {
        chip::test_utils::SleepMillis(65);
        gLoopback.mMessageDropped = false;
        ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), ctx.GetExchangeManager().GetReliableMessageMgr());
        ctx.DrainAndServiceIO();
    };

    // Standalone acks also increment the mSentMessageCount. But some messages could be acked
    // via piggybacked acks. So we cannot check for a specific value of mSentMessageCount.
    // Let's make sure atleast number is >= than the minimum messages required to complete the
    // handshake.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount >= 5);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);

    if (mrpCommissionerConfig.HasValue())
    {
        NL_TEST_ASSERT(inSuite,
                       pairingAccessory.GetMRPConfig().mIdleRetransTimeout == mrpCommissionerConfig.Value().mIdleRetransTimeout);
        NL_TEST_ASSERT(
            inSuite, pairingAccessory.GetMRPConfig().mActiveRetransTimeout == mrpCommissionerConfig.Value().mActiveRetransTimeout);
    }

    if (mrpAccessoryConfig.HasValue())
    {
        NL_TEST_ASSERT(inSuite,
                       pairingCommissioner.GetMRPConfig().mIdleRetransTimeout == mrpAccessoryConfig.Value().mIdleRetransTimeout);
        NL_TEST_ASSERT(
            inSuite, pairingCommissioner.GetMRPConfig().mActiveRetransTimeout == mrpAccessoryConfig.Value().mActiveRetransTimeout);
    }
}

void SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    gLoopback.Reset();
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, Optional<ReliableMessageProtocolConfig>::Missing(),
                                     Optional<ReliableMessageProtocolConfig>::Missing(), delegateCommissioner);
}

void SecurePairingHandshakeWithCommissionerMRPTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    gLoopback.Reset();
    ReliableMessageProtocolConfig config(1000_ms32, 10000_ms32);
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner,
                                     Optional<ReliableMessageProtocolConfig>::Value(config),
                                     Optional<ReliableMessageProtocolConfig>::Missing(), delegateCommissioner);
}

void SecurePairingHandshakeWithDeviceMRPTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    gLoopback.Reset();
    ReliableMessageProtocolConfig config(1000_ms32, 10000_ms32);
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, Optional<ReliableMessageProtocolConfig>::Missing(),
                                     Optional<ReliableMessageProtocolConfig>::Value(config), delegateCommissioner);
}

void SecurePairingHandshakeWithAllMRPTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    gLoopback.Reset();
    ReliableMessageProtocolConfig commissionerConfig(1000_ms32, 10000_ms32);
    ReliableMessageProtocolConfig deviceConfig(2000_ms32, 7000_ms32);
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner,
                                     Optional<ReliableMessageProtocolConfig>::Value(commissionerConfig),
                                     Optional<ReliableMessageProtocolConfig>::Value(deviceConfig), delegateCommissioner);
}

void SecurePairingHandshakeWithPacketLossTest(nlTestSuite * inSuite, void * inContext)
{
    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;
    gLoopback.Reset();
    gLoopback.mNumMessagesToDrop = 2;
    SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, Optional<ReliableMessageProtocolConfig>::Missing(),
                                     Optional<ReliableMessageProtocolConfig>::Missing(), delegateCommissioner);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 0);
}

void SecurePairingFailedHandshake(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TestSecurePairingDelegate delegateCommissioner;
    PASESession pairingCommissioner;

    TestSecurePairingDelegate delegateAccessory;
    PASESession pairingAccessory;

    gLoopback.Reset();
    gLoopback.mSentMessageCount = 0;

    ExchangeContext * contextCommissioner = ctx.NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = contextCommissioner->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    contextCommissioner->GetSessionHandle()->AsUnauthenticatedSession()->SetMRPConfig({
        64_ms32, // CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::PBKDFParamRequest, &pairingAccessory) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForPairing(sTestSpake2p01_PASEVerifier, sTestSpake2p01_IterationCount,
                                                   ByteSpan(sTestSpake2p01_Salt), kDefaultCommissioningPasscodeId, 0,
                                                   Optional<ReliableMessageProtocolConfig>::Missing(),
                                                   &delegateAccessory) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.Pair(Transport::PeerAddress(Transport::Type::kBle), 4321, 0,
                                            Optional<ReliableMessageProtocolConfig>::Missing(), contextCommissioner,
                                            &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

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

    SecurePairingHandshakeTestCommon(inSuite, inContext, *testPairingSession1, Optional<ReliableMessageProtocolConfig>::Missing(),
                                     Optional<ReliableMessageProtocolConfig>::Missing(), delegateCommissioner);
    SecurePairingDeserialize(inSuite, inContext, *testPairingSession1, *testPairingSession2);

    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t encrypted[64];
    PacketHeader header;
    MessageAuthenticationCode mac;

    header.SetSessionId(1);
    NL_TEST_ASSERT(inSuite, header.IsEncrypted() == true);
    NL_TEST_ASSERT(inSuite, header.MICTagLength() == 16);

    // Let's try encrypting using original session, and decrypting using deserialized
    {
        CryptoContext session1;

        CHIP_ERROR err = testPairingSession1->DeriveSecureSession(session1, CryptoContext::SessionRole::kInitiator);

        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = session1.Encrypt(plain_text, sizeof(plain_text), encrypted, header, mac);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    {
        CryptoContext session2;
        NL_TEST_ASSERT(inSuite,
                       testPairingSession2->DeriveSecureSession(session2, CryptoContext::SessionRole::kResponder) == CHIP_NO_ERROR);

        uint8_t decrypted[64];
        NL_TEST_ASSERT(inSuite, session2.Decrypt(encrypted, sizeof(plain_text), decrypted, header, mac) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(plain_text, decrypted, sizeof(plain_text)) == 0);
    }

    chip::Platform::Delete(testPairingSession1);
    chip::Platform::Delete(testPairingSession2);
}

void PASEVerifierSerializeTest(nlTestSuite * inSuite, void * inContext)
{
    PASEVerifier verifier;
    NL_TEST_ASSERT(inSuite, verifier.Deserialize(ByteSpan(sTestSpake2p01_SerializedVerifier)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(&verifier, &sTestSpake2p01_PASEVerifier, sizeof(PASEVerifier)) == 0);

    PASEVerifierSerialized serializedVerifier;
    MutableByteSpan serializedVerifierSpan(serializedVerifier);
    NL_TEST_ASSERT(inSuite, verifier.Serialize(serializedVerifierSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, serializedVerifierSpan.size() == kSpake2pSerializedVerifierSize);
    NL_TEST_ASSERT(inSuite, memcmp(serializedVerifier, sTestSpake2p01_SerializedVerifier, kSpake2pSerializedVerifierSize) == 0);

    PASEVerifierSerialized serializedVerifier2;
    MutableByteSpan serializedVerifier2Span(serializedVerifier2);
    NL_TEST_ASSERT(inSuite, chip::Crypto::DRBG_get_bytes(serializedVerifier, kSpake2pSerializedVerifierSize) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, verifier.Deserialize(ByteSpan(serializedVerifier)) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, verifier.Serialize(serializedVerifier2Span) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(serializedVerifier, serializedVerifier2, kSpake2pSerializedVerifierSize) == 0);
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
    NL_TEST_DEF("Handshake with Commissioner MRP Parameters", SecurePairingHandshakeWithCommissionerMRPTest),
    NL_TEST_DEF("Handshake with Device MRP Parameters", SecurePairingHandshakeWithDeviceMRPTest),
    NL_TEST_DEF("Handshake with Both MRP Parameters", SecurePairingHandshakeWithAllMRPTest),
    NL_TEST_DEF("Handshake with packet loss", SecurePairingHandshakeWithPacketLossTest),
    NL_TEST_DEF("Failed Handshake", SecurePairingFailedHandshake),
    NL_TEST_DEF("Serialize",   SecurePairingSerializeTest),
    NL_TEST_DEF("PASE Verifier Serialize", PASEVerifierSerializeTest),

    NL_TEST_SENTINEL()
};

int TestSecurePairing_Setup(void * inContext);
int TestSecurePairing_Teardown(void * inContext);

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing-PASE",
    &sTests[0],
    TestSecurePairing_Setup,
    TestSecurePairing_Teardown,
};
// clang-format on

// clang-format on
//
/**
 *  Set up the test suite.
 */
int TestSecurePairing_Setup(void * inContext)
{
    // Initialize System memory and resources
    VerifyOrReturnError(TestContext::InitializeAsync(inContext) == SUCCESS, FAILURE);

    auto & ctx = *static_cast<TestContext *>(inContext);
    ctx.SetBobNodeId(kPlaceholderNodeId);
    ctx.SetAliceNodeId(kPlaceholderNodeId);
    ctx.SetBobKeyId(0);
    ctx.SetAliceKeyId(0);
    ctx.SetFabricIndex(kUndefinedFabricIndex);

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestSecurePairing_Teardown(void * inContext)
{
    return TestContext::Finalize(inContext);
}

} // anonymous namespace

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
