/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      FuzzTest harness for CheckInHandler::OnMessageReceived — the state layer
 *      above the Check-In payload parser.
 *
 *      FuzzICDCheckInPW.cpp covers the decrypt path (ProcessCheckInPayload ->
 *      ParseCheckinMessagePayload). Everything that happens once a payload
 *      authenticates lives here instead, and none of it is reachable from that
 *      harness because it links src/app/icd/client:manager and not :handler:
 *
 *        - the counter-offset computation, which is modular arithmetic on the
 *          difference between the received counter and the stored
 *          start_icd_counter, both of which this harness drives;
 *        - the duplicate test against the stored offset;
 *        - the key-refresh threshold, which on crossing hands control to
 *          CheckInDelegate::OnKeyRefreshNeeded and RefreshKeySender.
 *
 *      The interesting inputs are therefore not payload bytes but the
 *      relationship between three counters: the one encoded in the message and
 *      the two held in the stored ICDClientInfo. Payload bytes are generated
 *      correctly under a known key so that every iteration gets past the MIC
 *      and reaches the state machine.
 *
 *      Scaffolding is a one-time AppContext (loopback exchange stack, fabric
 *      table, InteractionModelEngine) plus the production DefaultCheckInDelegate
 *      — not a stub, so the real RefreshKeySender allocation and its
 *      failure paths are exercised. Only the per-entry counters are rewritten
 *      per iteration.
 *
 *      Oracle: no sanitizer error, and OnMessageReceived absorbs every input —
 *      by contract it returns CHIP_NO_ERROR even for malformed or duplicate
 *      messages, so any other return is itself a finding.
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <app/InteractionModelEngine.h>
#include <app/icd/client/CheckInHandler.h>
#include <app/icd/client/DefaultCheckInDelegate.h>
#include <app/icd/client/DefaultICDClientStorage.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <crypto/CryptoBuildConfig.h>
#include <crypto/DefaultSessionKeystore.h>
#if CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/logging/Constants.h>
#include <lib/support/logging/TextOnlyLogging.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <system/SystemPacketBuffer.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::Crypto;
using namespace chip::Protocols::SecureChannel;
using namespace fuzztest;

constexpr FabricIndex kFabricIndex = 1;
constexpr NodeId kNodeId           = 0x1234;
constexpr uint8_t kKeyMaterial[]   = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

// OnMessageReceived is protected on CheckInHandler (it is an ExchangeDelegate
// override). Same wrapper the unit test uses.
class CheckInHandlerWrapper : public CheckInHandler
{
public:
    CHIP_ERROR Deliver(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload)
    {
        return OnMessageReceived(ec, payloadHeader, std::move(payload));
    }
};

// AppContext derives from ::testing::Test, whose TestBody() is pure virtual, so
// it cannot be instantiated outside the gtest machinery without a concrete leaf.
class HarnessContext : public Testing::AppContext
{
public:
    void TestBody() override {}
};

// One-time scaffolding. The exchange stack, fabric table and IM engine cost far
// more to stand up than a single iteration costs to run, so they are built once
// and only the stored counters are rewritten per input.
struct Fixture
{
    HarnessContext context;
    TestPersistentStorageDelegate clientInfoStore;
    DefaultSessionKeystore keystore;
    DefaultICDClientStorage storage;
    DefaultCheckInDelegate delegate;
    CheckInHandlerWrapper handler;
    ICDClientInfo seededClient;

    Fixture()
    {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
#if CHIP_CRYPTO_PSA
        // Fuzz binaries use gmock_main and so miss the unit-test main's PSA
        // initialization; the backend must be initialized before any crypto runs.
        VerifyOrDie(psa_crypto_init() == PSA_SUCCESS);
#endif
        // The check-in path logs per message; at fuzzing rates that dominates
        // both runtime and output volume.
        Logging::SetLogFilter(Logging::kLogCategory_None);

        HarnessContext::SetUpTestSuite();
        context.SetUp();

        // AppContext::SetUp() has already initialized the engine (and the access
        // control), so take the instance rather than initializing it again.
        InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

        VerifyOrDie(storage.Init(&clientInfoStore, &keystore) == CHIP_NO_ERROR);
        VerifyOrDie(delegate.Init(&storage, engine) == CHIP_NO_ERROR);
        VerifyOrDie(handler.Init(&context.GetExchangeManager(), &storage, &delegate, engine) == CHIP_NO_ERROR);

        seededClient.peer_node         = ScopedNodeId(kNodeId, kFabricIndex);
        seededClient.check_in_node     = seededClient.peer_node;
        seededClient.start_icd_counter = 0;
        seededClient.offset            = 0;

        VerifyOrDie(storage.UpdateFabricList(kFabricIndex) == CHIP_NO_ERROR);
        VerifyOrDie(storage.SetKey(seededClient, ByteSpan(kKeyMaterial)) == CHIP_NO_ERROR);
        VerifyOrDie(storage.StoreEntry(seededClient) == CHIP_NO_ERROR);
    }

    // MessagingContextData's destructor asserts the context was shut down
    // (MessagingContext.h:208). Without this the assert trips during static
    // destruction, which aborts the process before the coverage profile is
    // written -- a run that looks like a crash and reports 0% coverage.
    ~Fixture()
    {
        handler.Shutdown();
        storage.Shutdown();
        // AppContext::TearDown() shuts the engine down, so do not do it here too.
        context.TearDown();
        HarnessContext::TearDownTestSuite();
    }
};

Fixture & GetFixture()
{
    static Fixture sFixture;
    return sFixture;
}

// Builds a Check-In payload that authenticates against the seeded entry's key,
// so the state machine is reached on every iteration rather than 1-in-2^128.
std::vector<uint8_t> BuildValidPayload(const ICDClientInfo & info, uint32_t counter, const std::vector<uint8_t> & appData)
{
    std::vector<uint8_t> payload(CheckinMessage::GetCheckinPayloadSize(appData.size()));
    MutableByteSpan output(payload.data(), payload.size());

    ICDClientInfo mutableInfo(info);
    if (CheckinMessage::GenerateCheckinMessagePayload(mutableInfo.aes_key_handle, mutableInfo.hmac_key_handle, counter,
                                                      ByteSpan(appData.data(), appData.size()), output) != CHIP_NO_ERROR)
    {
        return {};
    }
    payload.resize(output.size());
    return payload;
}

// The three counters that decide which arm of the state machine runs: the one
// in the message, and the two persisted alongside the key.
void CheckInStateMachineAbsorbsAnyCounter(uint32_t encodedCounter, uint32_t storedStartCounter, uint32_t storedOffset,
                                          const std::vector<uint8_t> & appData)
{
    Fixture & f = GetFixture();

    ICDClientInfo info     = f.seededClient;
    info.start_icd_counter = storedStartCounter;
    info.offset            = storedOffset;
    ASSERT_EQ(f.storage.StoreEntry(info), CHIP_NO_ERROR);

    std::vector<uint8_t> payload = BuildValidPayload(f.seededClient, encodedCounter, appData);
    if (payload.empty())
    {
        return;
    }

    System::PacketBufferHandle buffer = System::PacketBufferHandle::NewWithData(payload.data(), payload.size());
    if (buffer.IsNull())
    {
        return;
    }

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(MsgType::ICD_CheckIn);

    // Contract: the handler absorbs malformed and duplicate messages and reports
    // success, so anything else is a finding in its own right.
    EXPECT_EQ(f.handler.Deliver(nullptr, payloadHeader, std::move(buffer)), CHIP_NO_ERROR);
}

FUZZ_TEST(ICDCheckInHandlerPW, CheckInStateMachineAbsorbsAnyCounter)
    .WithDomains(
        // Seeded at the boundaries that select each arm: equality and +/-1 around
        // the duplicate test, and values that straddle the key-refresh threshold
        // once the modular difference is taken.
        Arbitrary<uint32_t>().WithSeeds({ 0u, 1u, 2u, 0x7FFFFFFFu, 0x80000000u, 0x80000001u, 0xFFFFFFFFu }),
        Arbitrary<uint32_t>().WithSeeds({ 0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu }),
        Arbitrary<uint32_t>().WithSeeds({ 0u, 1u, 0x7FFFFFFEu, 0x7FFFFFFFu, 0x80000000u }),
        Arbitrary<std::vector<uint8_t>>().WithMaxSize(2));

} // namespace
