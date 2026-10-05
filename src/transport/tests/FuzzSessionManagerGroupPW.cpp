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
 *      Drives SessionManager::OnMessageReceived down its group branch. Real
 *      SessionManager, GroupDataProviderImpl, FabricTable and gGroupPeerTable;
 *      nothing on the parse, crypto or decode path is stubbed.
 */

#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/DefaultSessionKeystore.h>
#if CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/AutoRelease.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/CryptoContext.h>
#include <transport/GroupSession.h>
#include <transport/SecureMessageCodec.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/GroupcastTesting.h>
#include <transport/tests/LoopbackTransportManager.h>

namespace {

using namespace chip;
using namespace chip::Transport;
using namespace chip::Credentials;
using namespace fuzztest;

using chip::System::PacketBufferHandle;

using GroupInfo      = GroupDataProvider::GroupInfo;
using GroupKey       = GroupDataProvider::GroupKey;
using KeySet         = GroupDataProvider::KeySet;
using SecurityPolicy = GroupDataProvider::SecurityPolicy;

// The key stays a valid configured group key; the fuzzer varies the datagram, not the key.
constexpr GroupId kGroupId  = 2;
const uint8_t kEpochKey[16] = { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf };
constexpr uint16_t kTestKeysetId = 0x0123;

// A second keyset so the trial-decryption loop runs more than once. Not kCacheAndSync:
// SetKeySet rejects every policy but kTrustFirst.
constexpr GroupId kGroupIdSecond   = 3;
constexpr uint16_t kKeysetIdSecond = 0x0124;
const uint8_t kEpochKeySecond[16]  = {
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf
};

// An empty std::vector's data() may be null, and PacketBufferHandle::NewWithData passes it
// straight to memcpy, which UBSan's nonnull check rejects. Empty inputs stay in the domain.
System::PacketBufferHandle MakeBuf(const std::vector<uint8_t> & bytes)
{
    if (bytes.empty())
    {
        return MessagePacketBuffer::New(0);
    }
    return MessagePacketBuffer::NewWithData(bytes.data(), bytes.size());
}

// The delegate is the ExchangeManager boundary, past everything measured here, so a
// no-op removes no check.
class NoopDelegate : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader &, const PayloadHeader &, const SessionHandle &, DuplicateMessage,
                           System::PacketBufferHandle &&) override
    {}
};

// A null delegate would leave NotifyDelegate's dispatch arm unexercised.
class TestingSink : public chip::Groupcast::Testing::Delegate
{
public:
    void FlushGroupcastTestingEvent() override { mFlushes++; }

private:
    uint64_t mFlushes = 0;
};

// Built once: gGroupPeerTable is file-static, so rebuilding per input isolates nothing.
//
// The peer table is deliberately not reset between inputs either, unlike the counter harness.
// This case sends a single datagram per input, so a reset would leave the duplicate, window
// and MRU arms unreachable -- measured 61.38% against 71.14% region on
// SecureGroupMessageDispatch. The cost is that a crash here may need its predecessors to
// reproduce; re-run the corpus in order.
struct Fixture
{
    Testing::LoopbackTransportManager ctx;
    FabricTable fabricTable;
    TestPersistentStorageDelegate fabricStorage;
    PersistentStorageOperationalKeystore opKeyStore;
    PersistentStorageOpCertStore opCertStore;

    TestPersistentStorageDelegate providerStorage;
    Crypto::DefaultSessionKeystore providerKeystore;
    GroupDataProviderImpl provider{ /*maxGroupsPerFabric*/ 5, /*maxGroupKeysPerFabric*/ 8 };

    TestPersistentStorageDelegate deviceStorage;
    Crypto::DefaultSessionKeystore sessionKeystore;
    secure_channel::MessageCounterManager messageCounterManager;

    SessionManager sessionManager;
    NoopDelegate delegate;
    TestingSink testingSink;

    FabricIndex fabricIndex = kUndefinedFabricIndex;
    uint16_t sessionId      = 0; // learned from the prepared valid message
    std::vector<uint8_t> validSeed;

    PeerAddress peer;
};

Fixture * gFixture = nullptr;

// Needed for IterateGroupSessions() to yield a real group session.
void SetupGroupKeys(Fixture & fx)
{
    using namespace chip::TestCerts;

    FabricTable * fabricTable = fx.sessionManager.GetFabricTable();
    VerifyOrDie(fabricTable != nullptr);
    VerifyOrDie(fabricTable->AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                                   GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey,
                                                                   &fx.fabricIndex) == CHIP_NO_ERROR);

    uint8_t compressedFabricBuf[sizeof(uint64_t)];
    MutableByteSpan compressedFabricSpan(compressedFabricBuf);
    VerifyOrDie(fabricTable->FindFabricWithIndex(fx.fabricIndex)->GetCompressedFabricIdBytes(compressedFabricSpan) ==
                CHIP_NO_ERROR);

    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrDie(provider != nullptr);

    KeySet keySet(kTestKeysetId, GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
    memcpy(keySet.epoch_keys[0].key, kEpochKey, 16);
    keySet.epoch_keys[0].start_time = 0;
    GroupKey groupKey(kGroupId, kTestKeysetId);
    GroupInfo groupInfo(kGroupId, "Privacy Group");

    VerifyOrDie(provider->SetKeySet(fx.fabricIndex, compressedFabricSpan, keySet) == CHIP_NO_ERROR);
    VerifyOrDie(provider->SetGroupKeyAt(fx.fabricIndex, 0, groupKey) == CHIP_NO_ERROR);
    VerifyOrDie(provider->SetGroupInfoAt(fx.fabricIndex, 0, groupInfo) == CHIP_NO_ERROR);

    KeySet secondKeySet(kKeysetIdSecond, GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
    memcpy(secondKeySet.epoch_keys[0].key, kEpochKeySecond, 16);
    secondKeySet.epoch_keys[0].start_time = 0;
    GroupKey secondGroupKey(kGroupIdSecond, kKeysetIdSecond);
    GroupInfo secondGroupInfo(kGroupIdSecond, "Second Group");

    VerifyOrDie(provider->SetKeySet(fx.fabricIndex, compressedFabricSpan, secondKeySet) == CHIP_NO_ERROR);
    VerifyOrDie(provider->SetGroupKeyAt(fx.fabricIndex, 1, secondGroupKey) == CHIP_NO_ERROR);
    VerifyOrDie(provider->SetGroupInfoAt(fx.fabricIndex, 1, secondGroupInfo) == CHIP_NO_ERROR);
}

// Process-wide singleton: set on every input, or the mode leaks across iterations.
void ApplyTestingMode(Fixture & fx, bool enabled)
{
    auto & testing = chip::Groupcast::GetTesting();
    testing.SetDelegate(enabled ? &fx.testingSink : nullptr);
    testing.SetFabricIndex(fx.fabricIndex);
    testing.SetEnabled(enabled);
}

// Seed source, and how the installed sessionId is learned. Any mutation breaks its MIC.
void BuildValidSeed(Fixture & fx)
{
    Transport::OutgoingGroupSession outgoingSession(kGroupId, fx.fabricIndex);
    SessionHandle outgoingHandle(outgoingSession);
    SessionHolder outgoingHolder(outgoingHandle);

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);
    const char testPayload[] = "PrivacyTest";
    System::PacketBufferHandle payloadBuf =
        MessagePacketBuffer::NewWithData(reinterpret_cast<const uint8_t *>(testPayload), sizeof(testPayload));
    VerifyOrDie(!payloadBuf.IsNull());

    EncryptedPacketBufferHandle preparedMessage;
    VerifyOrDie(fx.sessionManager.PrepareMessage(outgoingHolder.Get().Value(), payloadHeader, std::move(payloadBuf),
                                                 preparedMessage) == CHIP_NO_ERROR);

    System::PacketBufferHandle writableMsg = preparedMessage.CastToWritable();
    VerifyOrDie(!writableMsg.IsNull());

    PacketHeader decodedHeader;
    uint16_t headerSize = 0;
    VerifyOrDie(decodedHeader.Decode(writableMsg->Start(), writableMsg->DataLength(), &headerSize) == CHIP_NO_ERROR);
    VerifyOrDie(decodedHeader.IsGroupSession());
    fx.sessionId = decodedHeader.GetSessionId();

    fx.validSeed.assign(writableMsg->Start(), writableMsg->Start() + writableMsg->DataLength());
}

Fixture & GetFixture()
{
    static std::once_flag once;
    std::call_once(once, [] {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
#if CHIP_CRYPTO_PSA
        // A fuzz binary links gmock_main, which does not init PSA; key derivation needs it.
        VerifyOrDie(psa_crypto_init() == PSA_SUCCESS);
#endif

        auto * fx = new Fixture();

        VerifyOrDie(fx->ctx.Init() == CHIP_NO_ERROR);

        VerifyOrDie(fx->opKeyStore.Init(&fx->fabricStorage) == CHIP_NO_ERROR);
        VerifyOrDie(fx->opCertStore.Init(&fx->fabricStorage) == CHIP_NO_ERROR);

        fx->provider.SetStorageDelegate(&fx->providerStorage);
        fx->provider.SetSessionKeystore(&fx->providerKeystore);
        VerifyOrDie(fx->provider.Init() == CHIP_NO_ERROR);
        Credentials::SetGroupDataProvider(&fx->provider);

        FabricTable::InitParams initParams;
        initParams.storage             = &fx->fabricStorage;
        initParams.operationalKeystore = &fx->opKeyStore;
        initParams.opCertStore         = &fx->opCertStore;
        VerifyOrDie(fx->fabricTable.Init(initParams) == CHIP_NO_ERROR);

        VerifyOrDie(fx->sessionManager.Init(&fx->ctx.GetSystemLayer(), &fx->ctx.GetTransportMgr(), &fx->messageCounterManager,
                                            &fx->deviceStorage, &fx->fabricTable, fx->sessionKeystore) == CHIP_NO_ERROR);
        fx->sessionManager.SetMessageDelegate(&fx->delegate);

        SetupGroupKeys(*fx);
        BuildValidSeed(*fx);

        Inet::IPAddress addr;
        VerifyOrDie(Inet::IPAddress::FromString("::1", addr));
        fx->peer = PeerAddress::UDP(addr, CHIP_PORT);

        gFixture = fx;

        // The fixture is leaked, but the file-static Inet EndPointManagers VerifyOrDie in
        // their destructors unless shut down first, which reads as a crash. atexit precedes them.
        std::atexit([] {
            if (gFixture != nullptr)
            {
                gFixture->sessionManager.Shutdown();
                gFixture->ctx.Shutdown();
            }
        });
    });
    return *gFixture;
}

// useRawDomain leaves the session id arbitrary; otherwise it is pinned to the installed
// key's hash so that inputs whose header shape passes reach a decrypt attempt.
void GroupDispatchDoesNotCrash(bool useRawDomain, bool testingEnabled, const std::vector<uint8_t> & bytes)
{
    Fixture & fx = GetFixture();

    ApplyTestingMode(fx, testingEnabled);

    std::vector<uint8_t> datagram = bytes;
    if (!useRawDomain && datagram.size() >= 3)
    {
        // Only the session id, at bytes 1-2. The flag bytes stay fuzzer-controlled, so most
        // inputs are still rejected on header shape before any key is tried; the seeds below
        // supply the well-formed shapes. Pinning the flags too costs more than it gains: it
        // measured 10 points lower, because the free bytes are what reach the reject arms.
        datagram[1] = static_cast<uint8_t>(fx.sessionId & 0xff);
        datagram[2] = static_cast<uint8_t>((fx.sessionId >> 8) & 0xff);
    }

    PacketBufferHandle msg = MakeBuf(datagram);
    if (msg.IsNull())
    {
        return;
    }

    fx.sessionManager.OnMessageReceived(fx.peer, std::move(msg));
}

// Seeds (evaluated lazily via the SeedProvider overload, so the fixture is stood up first).
std::vector<std::vector<uint8_t>> GroupSeeds()
{
    Fixture & fx = GetFixture();
    std::vector<std::vector<uint8_t>> seeds;

    // Programmatic valid seed: full privacy + valid MIC, drives the post-MIC region.
    if (!fx.validSeed.empty())
    {
        seeds.push_back(fx.validSeed);
    }

    // Written by field offset, not by hand-counted literal: PacketHeader::Encode lays the
    // fixed part out as [0] msgFlags, [1..2] sessionId, [3] secFlags, [4..7] counter,
    // [8..15] sourceNodeId, [16..17] destinationGroupId, and a group id placed anywhere but
    // 16..17 decodes as 0 and dies at the group-id compare.
    auto headerSeed = [&fx](uint8_t msgFlags, uint8_t secFlags, const std::vector<uint8_t> & tail) {
        std::vector<uint8_t> seed(18, 0);
        seed[0] = msgFlags;
        seed[1] = static_cast<uint8_t>(fx.sessionId & 0xff);
        seed[2] = static_cast<uint8_t>((fx.sessionId >> 8) & 0xff);
        seed[3] = secFlags;
        // counter and sourceNodeId stay zero; the mutator explores them.
        seed[16] = static_cast<uint8_t>(kGroupId & 0xff);
        seed[17] = static_cast<uint8_t>((kGroupId >> 8) & 0xff);
        seed.insert(seed.end(), tail.begin(), tail.end());
        return seed;
    };

    // Long enough for a payload header plus the 16-byte MIC.
    const std::vector<uint8_t> body = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
                                        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x01, 0x02 };

    // msgFlags 0x06 = sourceNodeId + destinationGroupId present.
    // secFlags: 0x01 group session, 0x80 privacy, 0x20 message extension.
    seeds.push_back(headerSeed(0x06, 0x81, body)); // the real shape: group session, privacy on
    seeds.push_back(headerSeed(0x06, 0x01, body)); // privacy off
    seeds.push_back(headerSeed(0x06, 0xa1, body)); // message-extension flag set
    seeds.push_back(headerSeed(0x06, 0x81, {}));   // fixed header only, no body or MIC

    // Truncated below the fixed header, for the length guards before any key lookup.
    seeds.push_back({ 0x06, static_cast<uint8_t>(fx.sessionId & 0xff), static_cast<uint8_t>((fx.sessionId >> 8) & 0xff), 0x81, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    return seeds;
}

FUZZ_TEST(FuzzSessionManagerGroupPW, GroupDispatchDoesNotCrash)
    .WithDomains(Arbitrary<bool>(), Arbitrary<bool>(),
                 // Cap input size: header + small payload + 16-byte MIC is well under 512.
                 VectorOf(Arbitrary<uint8_t>()).WithMaxSize(512).WithSeeds(&GroupSeeds));

// Re-MACs every input with the installed epoch key, so the MIC always verifies and the
// post-MIC continuation is reachable; a mutated seed cannot get there.
void GroupValidEncryptedDoesNotCrash(uint8_t payloadType, bool needsAck, bool testingEnabled, bool useSecondGroup,
                                     const std::vector<uint8_t> & payload)
{
    Fixture & fx = GetFixture();

    ApplyTestingMode(fx, testingEnabled);

    Transport::OutgoingGroupSession outgoingSession(useSecondGroup ? kGroupIdSecond : kGroupId, fx.fabricIndex);
    SessionHandle outgoingHandle(outgoingSession);
    SessionHolder outgoingHolder(outgoingHandle);

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::Id, payloadType);
    payloadHeader.SetNeedsAck(needsAck);

    System::PacketBufferHandle payloadBuf = MakeBuf(payload);
    if (payloadBuf.IsNull())
    {
        return;
    }

    EncryptedPacketBufferHandle prepared;
    if (fx.sessionManager.PrepareMessage(outgoingHolder.Get().Value(), payloadHeader, std::move(payloadBuf), prepared) !=
        CHIP_NO_ERROR)
    {
        return;
    }

    System::PacketBufferHandle wire = prepared.CastToWritable();
    if (wire.IsNull())
    {
        return;
    }

    PacketBufferHandle msg = MessagePacketBuffer::NewWithData(wire->Start(), wire->DataLength());
    if (msg.IsNull())
    {
        return;
    }
    fx.sessionManager.OnMessageReceived(fx.peer, std::move(msg));
}

FUZZ_TEST(FuzzSessionManagerGroupPW, GroupValidEncryptedDoesNotCrash)
    .WithDomains(Arbitrary<uint8_t>(), Arbitrary<bool>(), Arbitrary<bool>(), Arbitrary<bool>(),
                 VectorOf(Arbitrary<uint8_t>()).WithMaxSize(256));

// kUndefinedNodeId is included so the peer table's rejecting arm is reachable.
constexpr NodeId kManualSourceNodeIds[] = { 0x0000000011223344ULL, 0x0000000000000001ULL, kUndefinedNodeId, 0xFFFFFFFFFFFFFFFFULL };

// Built field by field because PrepareMessage fixes the source node id, counter and control
// flag internally, leaving the arms keyed on those unreachable through it. Privacy off keeps
// the frame verifiable; receive handles both forms.
CHIP_ERROR BuildManualGroupFrame(Fixture & fx, bool controlMsg, uint8_t sourceSelector, uint32_t counter, uint8_t payloadType,
                                 const std::vector<uint8_t> & payload, std::vector<uint8_t> & out)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    VerifyOrReturnError(groups != nullptr, CHIP_ERROR_INTERNAL);

    Crypto::SymmetricKeyContext * keyContext = groups->GetKeyContext(fx.fabricIndex, kGroupId);
    VerifyOrReturnError(keyContext != nullptr, CHIP_ERROR_INTERNAL);
    AutoRelease<Crypto::SymmetricKeyContext> keyContextOwner(keyContext);

    const NodeId sourceNodeId = kManualSourceNodeIds[sourceSelector % MATTER_ARRAY_SIZE(kManualSourceNodeIds)];

    PacketHeader packetHeader;
    packetHeader.SetSessionType(Header::SessionType::kGroupSession);
    packetHeader.SetSessionId(keyContext->GetKeyHash());
    packetHeader.SetMessageCounter(counter);
    packetHeader.SetSourceNodeId(sourceNodeId);
    // Group id only: Encode refuses a header carrying both destination kinds.
    packetHeader.SetDestinationGroupId(kGroupId);
    packetHeader.SetSecureSessionControlMsg(controlMsg);

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::Id, payloadType);

    System::PacketBufferHandle msg = MakeBuf(payload);
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);

    CryptoContext cryptoContext(keyContext);
    CryptoContext::NonceStorage nonce;
    ReturnErrorOnFailure(
        CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), packetHeader.GetMessageCounter(), sourceNodeId));
    ReturnErrorOnFailure(SecureMessageCodec::Encrypt(cryptoContext, nonce, payloadHeader, packetHeader, msg));
    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(msg));

    out.assign(msg->Start(), msg->Start() + msg->DataLength());
    return CHIP_NO_ERROR;
}

void GroupManualFrameDoesNotCrash(bool controlMsg, uint8_t sourceSelector, uint32_t counter, uint8_t payloadType,
                                  bool testingEnabled, const std::vector<uint8_t> & payload)
{
    Fixture & fx = GetFixture();

    ApplyTestingMode(fx, testingEnabled);

    std::vector<uint8_t> datagram;
    if (BuildManualGroupFrame(fx, controlMsg, sourceSelector, counter, payloadType, payload, datagram) != CHIP_NO_ERROR)
    {
        return;
    }

    PacketBufferHandle msg = MakeBuf(datagram);
    if (msg.IsNull())
    {
        return;
    }

    fx.sessionManager.OnMessageReceived(fx.peer, std::move(msg));
}

FUZZ_TEST(FuzzSessionManagerGroupPW, GroupManualFrameDoesNotCrash)
    .WithDomains(Arbitrary<bool>(), Arbitrary<uint8_t>(), Arbitrary<uint32_t>(), Arbitrary<uint8_t>(), Arbitrary<bool>(),
                 VectorOf(Arbitrary<uint8_t>()).WithMaxSize(256));

} // namespace
