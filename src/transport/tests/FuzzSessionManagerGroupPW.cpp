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
 *      FuzzTest harness for the Matter group-multicast decrypt-and-dispatch path.
 *
 *      Drives SessionManager::OnMessageReceived down its group branch
 *      (SecureGroupMessageDispatch): privacy AES-CTR deobfuscation, AES-CCM MIC
 *      verification, group header and counter parsing, and dispatch.
 *
 *      A real SessionManager, GroupDataProviderImpl, FabricTable and the file-static
 *      gGroupPeerTable are stood up with a valid epoch key; nothing on the parse,
 *      crypto or decode path is stubbed.
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

// Must precede SessionManager.h: enables EncryptedPacketBufferHandle::CastToWritable,
// used ONLY by the valid-seed generator (mirrors TestSessionManagerDispatch.cpp).
#define CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPCore.h>
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

#undef CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

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

// Same parameters as TestGroupPrepareMessagePrivacy. The key is a valid configured group
// key throughout; the fuzzer varies the datagram, not the key material.
constexpr GroupId kGroupId  = 2;
const uint8_t kEpochKey[16] = { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf };
constexpr uint16_t kTestKeysetId = 0x0123;

// A second group under its own keyset, so the trial-decryption loop runs more than once.
// kCacheAndSync is deliberately not used: GroupDataProviderImpl::SetKeySet rejects every
// policy but kTrustFirst, so no keyset carrying it can reach the receive path.
constexpr GroupId kGroupIdSecond   = 3;
constexpr uint16_t kKeysetIdSecond = 0x0124;
const uint8_t kEpochKeySecond[16]  = {
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf
};

// Dispatch needs a landing point. The delegate is the ExchangeManager boundary, past
// everything this harness measures, so a no-op removes no check.
class NoopDelegate : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader &, const PayloadHeader &, const SessionHandle &, DuplicateMessage,
                           System::PacketBufferHandle &&) override
    {}
};

// The Groupcast testing event sink. A null delegate would leave NotifyDelegate's dispatch
// arm unexercised.
class TestingSink : public chip::Groupcast::Testing::Delegate
{
public:
    void FlushGroupcastTestingEvent() override { mFlushes++; }

private:
    uint64_t mFlushes = 0;
};

// Built once and reused: gGroupPeerTable is file-static and persists regardless, so
// rebuilding the manager per input would cost time without isolating anything.
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

// Register a fabric and group key so IterateGroupSessions() yields a real group session.
// Mirrors SetupGroupKeys in TestSessionManagerDispatch.cpp.
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

// The testing state is a process-wide singleton, so set it explicitly on every input rather
// than only when enabling -- otherwise the mode leaks across iterations.
void ApplyTestingMode(Fixture & fx, bool enabled)
{
    auto & testing = chip::Groupcast::GetTesting();
    testing.SetDelegate(enabled ? &fx.testingSink : nullptr);
    testing.SetFabricIndex(fx.fabricIndex);
    testing.SetEnabled(enabled);
}

// One valid privacy-protected datagram via the real PrepareMessage round-trip, used as a
// seed and to learn the installed sessionId. Any mutation of it breaks the MIC.
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

        // The fixture is intentionally leaked, but the file-static Inet EndPointManagers
        // VerifyOrDie in their destructors unless the layers were shut down first -- without
        // this hook the run ends in a SIGABRT that reads as a crash. atexit runs before them.
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

// One datagram of arbitrary bytes. useRawDomain leaves the session id arbitrary, so most
// inputs are rejected before any key is tried; otherwise it is overwritten with the
// installed key's hash, which forces every input through privacy deobfuscation and a
// decrypt attempt.
void GroupDispatchDoesNotCrash(bool useRawDomain, bool testingEnabled, const std::vector<uint8_t> & bytes)
{
    Fixture & fx = GetFixture();

    ApplyTestingMode(fx, testingEnabled);

    std::vector<uint8_t> datagram = bytes;
    if (!useRawDomain && datagram.size() >= 3)
    {
        // PacketHeader layout: byte0 msgFlags, bytes 1-2 sessionId (little-endian), byte3 secFlags.
        datagram[1] = static_cast<uint8_t>(fx.sessionId & 0xff);
        datagram[2] = static_cast<uint8_t>((fx.sessionId >> 8) & 0xff);
    }

    PacketBufferHandle msg = MessagePacketBuffer::NewWithData(datagram.data(), datagram.size());
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

    const uint8_t lo = static_cast<uint8_t>(fx.sessionId & 0xff);
    const uint8_t hi = static_cast<uint8_t>((fx.sessionId >> 8) & 0xff);

    // Structured header-shape seeds, each pinned to the installed sessionId (Domain A shape),
    // giving the mutator a representative for each DSIZ/flag dispatch arm.
    // msgFlags byte0: version 0; 0x04 = SourceNodeId present, 0x02 = DestGroupId present.
    // secFlags byte3: 0x01 = group session, 0x80 = privacy, 0x20 = MsgExtension.

    // Source + DestGroup, group session, privacy on (mirrors the real shape), short body.
    seeds.push_back({ 0x06, lo,   hi,   0x81, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00 });
    // Same but privacy OFF.
    seeds.push_back({ 0x06, lo,   hi,   0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00 });
    // MsgExtension flag set with a small MX block (length prefix 0x0002 + 2 bytes).
    seeds.push_back({ 0x06, lo,   hi,   0xa1, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0xAB, 0xCD, 0x00, 0x00, 0x00,
                      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00 });
    // DSIZ reserved 0b11 (both node+group dest bits): byte0 = 0x06 | 0x01? DSIZ is bits 0-1:
    // 0x03 => both DestNode(0x01)+DestGroup(0x02). Plus SourceNodeId 0x04 => 0x07.
    seeds.push_back({ 0x07, lo,   hi,   0x81, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00 });
    // Truncated-to-16 buffer (mirrors TestGroupIncomingPrivacyBoundsCheck's shrink → :1126 guard).
    seeds.push_back({ 0x06, lo, hi, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    return seeds;
}

FUZZ_TEST(FuzzSessionManagerGroupPW, GroupDispatchDoesNotCrash)
    .WithDomains(Arbitrary<bool>(), Arbitrary<bool>(),
                 // Cap input size: header + small payload + 16-byte MIC is well under 512.
                 VectorOf(Arbitrary<uint8_t>()).WithMaxSize(512).WithSeeds(&GroupSeeds));

// Encrypt-per-iteration: build a group message from fuzzer-controlled inner content and encrypt
// it with the installed epoch key on EVERY input, so the AES-CCM MIC is always valid. The seeded
// valid datagram only reaches the post-MIC region on its single unmutated replay -- any mutation
// breaks the MIC -- so it cannot explore the post-MIC branches. This case re-MACs each input,
// exercising the post-MIC continuation: the group message-counter window (GroupPeerMessageCounter),
// payload-header decode, and the dispatch arms. Reachable by a group member holding the shared
// epoch key.
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

    System::PacketBufferHandle payloadBuf = MessagePacketBuffer::NewWithData(payload.data(), payload.size());
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

// Source node ids offered to the manual-frame case. kUndefinedNodeId is included because
// SessionManager.cpp:1331 rejects a decrypted frame whose node id the peer table will not
// accept, and that arm is otherwise unreachable.
constexpr NodeId kManualSourceNodeIds[] = { 0x0000000011223344ULL, 0x0000000000000001ULL, kUndefinedNodeId, 0xFFFFFFFFFFFFFFFFULL };

// Build a group datagram field by field instead of through PrepareMessage, then MAC it with
// the installed epoch key. PrepareMessage emits exactly one header shape -- destination group
// id, its own node id, its own monotonic counter, never the control-message flag, since
// IsValidGroupMsg() rejects that combination -- so the arms keyed on those fields cannot be
// reached through it however the payload is mutated. Privacy is left off: both forms are
// handled on receive and the unobfuscated form keeps the built frame verifiable.
bool BuildManualGroupFrame(Fixture & fx, bool controlMsg, bool destinationIsNode, uint8_t sourceSelector, uint32_t counter,
                           uint8_t payloadType, const std::vector<uint8_t> & payload, std::vector<uint8_t> & out)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    VerifyOrReturnValue(groups != nullptr, false);

    Crypto::SymmetricKeyContext * keyContext = groups->GetKeyContext(fx.fabricIndex, kGroupId);
    VerifyOrReturnValue(keyContext != nullptr, false);

    const NodeId sourceNodeId = kManualSourceNodeIds[sourceSelector % MATTER_ARRAY_SIZE(kManualSourceNodeIds)];

    PacketHeader packetHeader;
    packetHeader.SetSessionType(Header::SessionType::kGroupSession);
    packetHeader.SetSessionId(keyContext->GetKeyHash());
    packetHeader.SetMessageCounter(counter);
    packetHeader.SetSourceNodeId(sourceNodeId);
    // SecureGroupMessageDispatch returns at :1178 unless a destination group id is present,
    // while IsValidMCSPMsg() additionally requires a destination node id -- so the MCSP arm
    // is only reachable by a frame carrying both.
    packetHeader.SetDestinationGroupId(kGroupId);
    if (destinationIsNode)
    {
        packetHeader.SetDestinationNodeId(sourceNodeId);
    }
    packetHeader.SetSecureSessionControlMsg(controlMsg);

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::Id, payloadType);

    System::PacketBufferHandle msg = MessagePacketBuffer::NewWithData(payload.data(), payload.size());
    if (msg.IsNull())
    {
        keyContext->Release();
        return false;
    }

    CryptoContext cryptoContext(keyContext);
    CryptoContext::NonceStorage nonce;
    bool ok = CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), packetHeader.GetMessageCounter(), sourceNodeId) ==
            CHIP_NO_ERROR &&
        SecureMessageCodec::Encrypt(cryptoContext, nonce, payloadHeader, packetHeader, msg) == CHIP_NO_ERROR &&
        packetHeader.EncodeBeforeData(msg) == CHIP_NO_ERROR;

    keyContext->Release();
    VerifyOrReturnValue(ok, false);

    out.assign(msg->Start(), msg->Start() + msg->DataLength());
    return true;
}

void GroupManualFrameDoesNotCrash(bool controlMsg, bool destinationIsNode, uint8_t sourceSelector, uint32_t counter,
                                  uint8_t payloadType, bool testingEnabled, const std::vector<uint8_t> & payload)
{
    Fixture & fx = GetFixture();

    ApplyTestingMode(fx, testingEnabled);

    std::vector<uint8_t> datagram;
    if (!BuildManualGroupFrame(fx, controlMsg, destinationIsNode, sourceSelector, counter, payloadType, payload, datagram))
    {
        return;
    }

    PacketBufferHandle msg = MessagePacketBuffer::NewWithData(datagram.data(), datagram.size());
    if (msg.IsNull())
    {
        return;
    }

    fx.sessionManager.OnMessageReceived(fx.peer, std::move(msg));
}

FUZZ_TEST(FuzzSessionManagerGroupPW, GroupManualFrameDoesNotCrash)
    .WithDomains(Arbitrary<bool>(), Arbitrary<bool>(), Arbitrary<uint8_t>(), Arbitrary<uint32_t>(), Arbitrary<uint8_t>(),
                 Arbitrary<bool>(), VectorOf(Arbitrary<uint8_t>()).WithMaxSize(256));

} // namespace
