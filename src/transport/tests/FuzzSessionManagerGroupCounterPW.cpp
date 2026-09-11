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
 *      Stateful FuzzTest harness for the group peer-counter table.
 *
 *      FindOrAddPeer maintains a fixed-size LRU whose insert, evict and MRU paths depend on
 *      what earlier datagrams left behind, so one input is a sequence of records. It runs
 *      only after AES-CCM verification, which mutated bytes cannot pass, so each record is
 *      encrypted with the real group key first.
 *
 *      Real SessionManager, FabricTable, GroupDataProviderImpl, CryptoContext, libcrypto
 *      and gGroupPeerTable; nothing on the exercised path is stubbed.
 */

#include <array>
#include <cstdint>
#include <cstdlib>
#include <mutex>
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
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/CryptoContext.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/GroupSession.h>
#include <transport/SecureMessageCodec.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/tests/LoopbackTransportManager.h>

namespace {

using namespace chip;
using namespace chip::Transport;
using namespace chip::Credentials;
using namespace fuzztest;

using chip::System::PacketBufferHandle;

using GroupInfo = GroupDataProvider::GroupInfo;
using GroupKey  = GroupDataProvider::GroupKey;
using KeySet    = GroupDataProvider::KeySet;

// The key stays a valid configured key; the fuzzer varies sourceNodeId and counter.
constexpr GroupId kGroupId       = 2;
constexpr uint16_t kTestKeysetId = 0x0123;

// Routing uses Fixture::fabricCount, and more than one is what exercises the fabric-search
// loop, so a short install has to be loud rather than silently narrowing the harness.
constexpr size_t kMaxFabrics       = 4;
constexpr size_t kInstalledFabrics = 3;

// Distinct epoch key per fabric so each yields a distinct derived group key.
const uint8_t kEpochKeys[kMaxFabrics][16] = {
    { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf },
    { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
    { 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf },
    { 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef },
};

// The delegate is the ExchangeManager boundary, past everything measured here, so a
// no-op removes no check.
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

class NoopDelegate : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader &, const PayloadHeader &, const SessionHandle &, DuplicateMessage,
                           System::PacketBufferHandle &&) override
    {}
};

// Built once. gGroupPeerTable is file-static, so the per-input reset below is what makes
// an input reproducible on its own.
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

    std::array<FabricIndex, kMaxFabrics> fabricIndices{};
    size_t fabricCount = 0;

    PeerAddress peer;
};

Fixture * gFixture = nullptr;

// Each successful add yields a distinct fabric index.
bool InstallFabric(Fixture & fx, size_t slot, const TestCerts::UnitTestCertAsset & root, const TestCerts::UnitTestCertAsset & icac,
                   const TestCerts::UnitTestCertAsset & noc)
{
    FabricIndex fabricIndex = kUndefinedFabricIndex;
    if (fx.fabricTable.AddNewFabricForTestIgnoringCollisions(root.mCert, icac.mCert, noc.mCert, noc.mKey, &fabricIndex) !=
        CHIP_NO_ERROR)
    {
        return false;
    }

    uint8_t compressedFabricBuf[sizeof(uint64_t)];
    MutableByteSpan compressedFabricSpan(compressedFabricBuf);
    const FabricInfo * info = fx.fabricTable.FindFabricWithIndex(fabricIndex);
    if (info == nullptr || info->GetCompressedFabricIdBytes(compressedFabricSpan) != CHIP_NO_ERROR)
    {
        return false;
    }

    KeySet keySet(kTestKeysetId, GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
    memcpy(keySet.epoch_keys[0].key, kEpochKeys[slot], 16);
    keySet.epoch_keys[0].start_time = 0;
    GroupKey groupKey(kGroupId, kTestKeysetId);
    GroupInfo groupInfo(kGroupId, "Group");

    VerifyOrReturnValue(fx.provider.SetKeySet(fabricIndex, compressedFabricSpan, keySet) == CHIP_NO_ERROR, false);
    VerifyOrReturnValue(fx.provider.SetGroupKeyAt(fabricIndex, 0, groupKey) == CHIP_NO_ERROR, false);
    VerifyOrReturnValue(fx.provider.SetGroupInfoAt(fabricIndex, 0, groupInfo) == CHIP_NO_ERROR, false);

    fx.fabricIndices[fx.fabricCount++] = fabricIndex;
    return true;
}

// Mirrors the production encrypt sequence, but takes the node id, counter and control flag
// as parameters; PrepareMessage fixes all three internally.
CHIP_ERROR EncodeGroupDatagram(Fixture & fx, size_t slot, NodeId nodeId, uint32_t counter, bool isControl,
                               std::vector<uint8_t> & out)
{
    const FabricIndex fabricIndex = fx.fabricIndices[slot];

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);
    // Must stay false: a group message requesting an ack is dropped before FindOrAddPeer.

    const uint8_t payload[]        = { 0x01, 0x02 };
    System::PacketBufferHandle msg = MessagePacketBuffer::NewWithData(payload, sizeof(payload));
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);

    PacketHeader packetHeader;
    packetHeader.SetDestinationGroupId(kGroupId);
    packetHeader.SetMessageCounter(counter);
    packetHeader.SetSessionType(Header::SessionType::kGroupSession);
    packetHeader.SetFlags(Header::SecFlagValues::kPrivacyFlag);
    packetHeader.SetSourceNodeId(nodeId);
    // The secFlags C-bit, which selects the control or data peer list.
    packetHeader.SetSecureSessionControlMsg(isControl);

    auto * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturnError(groups != nullptr, CHIP_ERROR_INTERNAL);
    Crypto::SymmetricKeyContext * keyContext = groups->GetKeyContext(fabricIndex, kGroupId);
    VerifyOrReturnError(keyContext != nullptr, CHIP_ERROR_INTERNAL);
    AutoRelease<Crypto::SymmetricKeyContext> keyContextOwner(keyContext);

    packetHeader.SetSessionId(keyContext->GetKeyHash());
    CryptoContext cryptoContext(keyContext);

    CryptoContext::NonceStorage nonce;
    ReturnErrorOnFailure(
        CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), packetHeader.GetMessageCounter(), nodeId));
    ReturnErrorOnFailure(SecureMessageCodec::Encrypt(cryptoContext, nonce, payloadHeader, packetHeader, msg));

    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(msg));

    // Privacy-encrypt the header fields, as the production send path does.
    VerifyOrReturnError(msg->TotalLength() == msg->DataLength(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    uint8_t * data     = msg->Start();
    size_t len         = msg->TotalLength();
    uint16_t footerLen = packetHeader.MICTagLength();
    VerifyOrReturnError(footerLen <= len, CHIP_ERROR_INTERNAL);

    uint16_t taglen = 0;
    MessageAuthenticationCode mac;
    ReturnErrorOnFailure(mac.Decode(packetHeader, &data[len - footerLen], footerLen, &taglen));
    VerifyOrReturnError(taglen == footerLen, CHIP_ERROR_INTERNAL);

    uint8_t * privacyHeader = packetHeader.PrivacyHeader(msg->Start());
    size_t privacyLength    = packetHeader.PrivacyHeaderLength();
    ReturnErrorOnFailure(cryptoContext.PrivacyEncrypt(privacyHeader, privacyLength, privacyHeader, packetHeader, mac));

    out.assign(msg->Start(), msg->Start() + msg->DataLength());
    return CHIP_NO_ERROR;
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

        using namespace chip::TestCerts;
        // Distinct roots give distinct compressed fabric ids.
        InstallFabric(*fx, 0, GetRootACertAsset(), GetIAA1CertAsset(), GetNodeA1CertAsset());
        InstallFabric(*fx, 1, GetRootBCertAsset(), GetIAB1CertAsset(), GetNodeB1CertAsset());
        InstallFabric(*fx, 2, GetRootACertAsset(), GetIAA1CertAsset(), GetNodeA2CertAsset());
        VerifyOrDie(fx->fabricCount == kInstalledFabrics);

        Inet::IPAddress addr;
        VerifyOrDie(Inet::IPAddress::FromString("::1", addr));
        fx->peer = PeerAddress::UDP(addr, CHIP_PORT);

        gFixture = fx;

        // The fixture is leaked, but the file-static Inet EndPointManagers VerifyOrDie in
        // their destructors unless shut down first, which reads as a crash.
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

// One fuzzer record: routes a datagram to a fabric slot and picks the varied fields.
struct Record
{
    uint8_t fabricSel;  // % fabricCount -> which fabric key encrypts the datagram
    uint8_t nodeSel;    // -> nodeId = kNodeBase + nodeSel (forced != kUndefinedNodeId)
    bool isControl;     // sets the packet control-msg flag; such frames are rejected pre-dispatch
    uint32_t counter;   // selects the replay-window position; see AnyGroupCounter
    bool undefinedNode; // send sourceNodeId 0 -> FindOrAddPeer's CHIP_ERROR_INVALID_ARGUMENT arm
};

// 256 distinct node ids via nodeSel is enough to over-fill the caps and force evictions.
constexpr NodeId kNodeBase = 0x0000000100000000ull;

// Empties gGroupPeerTable so a crash reproduces from its input alone.
void ResetPeerTable(Fixture & fx)
{
    for (size_t i = 0; i < fx.fabricCount; i++)
    {
        fx.sessionManager.FabricRemoved(fx.fabricIndices[i]);
    }
}

// State accumulates across the sequence; the fuzzer controls the count and every field.
void GroupPeerTableDoesNotCorrupt(const std::vector<Record> & records)
{
    Fixture & fx = GetFixture();
    ResetPeerTable(fx);

    for (const Record & r : records)
    {
        const size_t slot   = r.fabricSel % fx.fabricCount;
        const NodeId nodeId = r.undefinedNode ? kUndefinedNodeId : (kNodeBase + r.nodeSel);

        std::vector<uint8_t> datagram;
        if (EncodeGroupDatagram(fx, slot, nodeId, r.counter, r.isControl, datagram) != CHIP_NO_ERROR)
        {
            continue;
        }

        PacketBufferHandle msg = MakeBuf(datagram);
        if (msg.IsNull())
        {
            continue;
        }

        fx.sessionManager.OnMessageReceived(fx.peer, std::move(msg));
    }
}

// One seed per table transition, so the mutator starts from a representative of each.
std::vector<std::vector<Record>> GroupCounterSeeds()
{
    std::vector<std::vector<Record>> seeds;

    auto data = [](uint8_t node) { return Record{ 0, node, false, 1, false }; };

    // Baseline: one data record.
    seeds.push_back({ data(1) });

    // Control frames fail IsValidGroupMsg before FindOrAddPeer, so this covers that guard,
    // not the control table.
    seeds.push_back({ Record{ 0, 1, true, 1, false } });

    // Past the data cap on one fabric: fill, then evict.
    {
        std::vector<Record> s;
        for (uint8_t n = 1; n <= 17; n++)
        {
            s.push_back(data(n));
        }
        seeds.push_back(std::move(s));
    }

    // Still the rejection guard: the control cap is unreachable through this function.
    seeds.push_back({ Record{ 0, 1, true, 1, false }, Record{ 0, 2, true, 1, false }, Record{ 0, 3, true, 1, false } });

    // Two fabrics: the first-add branch for a second slot.
    seeds.push_back({ Record{ 0, 1, false, 1, false }, Record{ 1, 2, false, 1, false }, Record{ 2, 3, false, 1, false } });

    // Re-sending a present node id takes the search-hit MRU-move path.
    {
        std::vector<Record> s;
        for (uint8_t n = 1; n <= 15; n++)
        {
            s.push_back(data(n));
        }
        s.push_back(data(1)); // re-send present node (MRU move)
        s.push_back(data(8)); // re-send present node (MRU move from middle)
        seeds.push_back(std::move(s));
    }

    // Walks one node's replay window: future, in-window, equal-to-max and behind.
    {
        const uint8_t node = 5;
        std::vector<Record> s;
        for (uint32_t c : { 100u, 101u, 100u, 90u, 90u, 69u, 68u, 140u, 141u, 100u })
        {
            s.push_back(Record{ 0, node, false, c, false });
        }
        seeds.push_back(std::move(s));
    }

    // 8. A jump past the window width (shift > 32) forces the window reset rather than a shift.
    seeds.push_back({ Record{ 0, 6, false, 10u, false }, Record{ 0, 6, false, 200u, false }, Record{ 0, 6, false, 199u, false } });

    // 9. sourceNodeId 0 -> the FindOrAddPeer invalid-argument arm.
    seeds.push_back({ Record{ 0, 1, false, 1u, true } });

    return seeds;
}

// Record domain: fabricSel/nodeSel/isControl/counter, each field fuzzer-controlled. The fuzzer
// picks the record count (bounded to keep per-input cost low). The 1-byte nodeSel concentrates
// entropy on transition structure rather than ID uniqueness.
// CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE is 32, so a uniform uint32 lands inside the window
// of the trust-first counter with probability ~2^-27. A flat domain therefore never reaches
// PeerMessageCounter's InWindow / duplicate arms, only the FutureCounter one. Mixing in a
// small range makes consecutive records for one node walk the window instead.
auto AnyGroupCounter()
{
    return OneOf(InRange<uint32_t>(1, 128), Arbitrary<uint32_t>());
}

auto RecordDomain()
{
    return StructOf<Record>(Arbitrary<uint8_t>(), // fabricSel
                            Arbitrary<uint8_t>(), // nodeSel
                            Arbitrary<bool>(),    // isControl
                            AnyGroupCounter(),    // counter
                            Arbitrary<bool>());   // undefinedNode
}

FUZZ_TEST(FuzzSessionManagerGroupCounterPW, GroupPeerTableDoesNotCorrupt)
    .WithDomains(VectorOf(RecordDomain()).WithMaxSize(64).WithSeeds(&GroupCounterSeeds));

// GroupOutgoingCounters::GetCounter and IncrementCounter are reached only from
// PrepareMessage, which EncodeGroupDatagram above bypasses in order to choose the counter.
// This case sends through the real PrepareMessage and loops the result back in.
void GroupSendThenReceiveDoesNotCrash(uint8_t fabricSel, uint8_t typeSel, const std::vector<uint8_t> & payload)
{
    Fixture & fx = GetFixture();

    const size_t slot = fabricSel % fx.fabricCount;

    Transport::OutgoingGroupSession outgoingSession(kGroupId, fx.fabricIndices[slot]);
    SessionHandle outgoingHandle(outgoingSession);
    SessionHolder outgoingHolder(outgoingHandle);

    // typeSel picks the send-side counter branch: the two SecureChannel counter-sync types are
    // what IsControlMessage recognises. The counters advance before the IsValidGroupMsg bail,
    // so the control branch is covered even though a control group send cannot complete.
    PayloadHeader payloadHeader;
    switch (typeSel % 3)
    {
    case 0:
        payloadHeader.SetMessageType(chip::Protocols::SecureChannel::MsgType::MsgCounterSyncReq);
        break;
    case 1:
        payloadHeader.SetMessageType(chip::Protocols::SecureChannel::MsgType::MsgCounterSyncRsp);
        break;
    default:
        payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);
        break;
    }

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

FUZZ_TEST(FuzzSessionManagerGroupCounterPW, GroupSendThenReceiveDoesNotCrash)
    .WithDomains(Arbitrary<uint8_t>(), Arbitrary<uint8_t>(), VectorOf(Arbitrary<uint8_t>()).WithMaxSize(128));

// These two drive GroupPeerTable directly: the property is that a sender's counter state
// stays bound to that sender across LRU shuffles. Aliasing it to a neighbour would accept
// replays with no memory error for a sanitizer to catch.

constexpr FabricIndex kOracleFabric = 1;
constexpr NodeId kOracleNodeBase    = 0x0000000200000000ull;

// A committed counter must still read as a replay after unrelated inserts shuffle its entry.
void PeerCounterSurvivesLruChurn(uint8_t victimSel, uint8_t churn, uint32_t counterValue)
{
    Transport::GroupPeerTable table;

    const NodeId victim = kOracleNodeBase + victimSel;

    Transport::PeerMessageCounter * counter = nullptr;
    ASSERT_EQ(table.FindOrAddPeer(kOracleFabric, victim, false, counter), CHIP_NO_ERROR);
    ASSERT_NE(counter, nullptr);
    ASSERT_EQ(counter->VerifyOrTrustFirstGroup(counterValue), CHIP_NO_ERROR);
    counter->CommitGroup(counterValue);

    // Up to a full table, leaving the victim at the least-recently-used end but still
    // present: the boundary where a ShiftAndInsert off-by-one would show.
    const uint8_t inserts = churn % CHIP_CONFIG_MAX_GROUP_DATA_PEERS;
    for (uint8_t i = 0; i < inserts; i++)
    {
        const NodeId other                      = kOracleNodeBase + static_cast<NodeId>(victimSel) + 1 + i;
        Transport::PeerMessageCounter * ignored = nullptr;
        ASSERT_EQ(table.FindOrAddPeer(kOracleFabric, other, false, ignored), CHIP_NO_ERROR);
    }

    Transport::PeerMessageCounter * again = nullptr;
    ASSERT_EQ(table.FindOrAddPeer(kOracleFabric, victim, false, again), CHIP_NO_ERROR);
    ASSERT_NE(again, nullptr);
    EXPECT_EQ(again->VerifyOrTrustFirstGroup(counterValue), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
}

FUZZ_TEST(FuzzSessionManagerGroupCounterPW, PeerCounterSurvivesLruChurn)
    .WithDomains(Arbitrary<uint8_t>(), Arbitrary<uint8_t>(), InRange<uint32_t>(1, 0xFFFFFFFEu));

// Admitting a crowded-out peer with fresh state or refusing it outright is a policy choice,
// so both are accepted; either way it must never be handed another peer's counter history.
void PeerCounterCrowdedOutPeerKeepsNoForeignState(uint8_t victimSel, uint32_t victimCounter)
{
    // Far enough apart to fall outside each other's 32-entry window, so a duplicate verdict
    // can only come from another peer's state.
    const uint32_t otherCounter = victimCounter + 0x10000u;

    Transport::GroupPeerTable table;

    const NodeId victim = kOracleNodeBase + victimSel;

    Transport::PeerMessageCounter * counter = nullptr;
    ASSERT_EQ(table.FindOrAddPeer(kOracleFabric, victim, false, counter), CHIP_NO_ERROR);
    ASSERT_NE(counter, nullptr);
    ASSERT_EQ(counter->VerifyOrTrustFirstGroup(victimCounter), CHIP_NO_ERROR);
    counter->CommitGroup(victimCounter);

    // Fill the remaining slots, each peer committing its own counter.
    for (uint8_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        const NodeId other                        = kOracleNodeBase + static_cast<NodeId>(victimSel) + 1 + i;
        Transport::PeerMessageCounter * otherPeer = nullptr;
        if (table.FindOrAddPeer(kOracleFabric, other, false, otherPeer) != CHIP_NO_ERROR)
        {
            break;
        }
        ASSERT_NE(otherPeer, nullptr);
        if (otherPeer->VerifyOrTrustFirstGroup(otherCounter) == CHIP_NO_ERROR)
        {
            otherPeer->CommitGroup(otherCounter);
        }
    }

    Transport::PeerMessageCounter * again = nullptr;
    const CHIP_ERROR err                  = table.FindOrAddPeer(kOracleFabric, victim, false, again);
    if (err != CHIP_NO_ERROR)
    {
        return; // Refused rather than re-admitted; nothing was handed out to check.
    }
    ASSERT_NE(again, nullptr);

    // otherCounter belongs to the peers that filled the table, never to the victim.
    EXPECT_NE(again->VerifyOrTrustFirstGroup(otherCounter), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
}

FUZZ_TEST(FuzzSessionManagerGroupCounterPW, PeerCounterCrowdedOutPeerKeepsNoForeignState)
    .WithDomains(Arbitrary<uint8_t>(), InRange<uint32_t>(1, 0xFFFEFFFEu));

} // namespace
