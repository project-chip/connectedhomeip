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
 *      Stateful FuzzTest harness for the Matter group-multicast peer-counter table.
 *
 *      The state that matters lives in the file-static gGroupPeerTable and only builds up
 *      across datagrams: FindOrAddPeer maintains a fixed-size LRU per fabric, so its
 *      insert, evict and most-recently-used paths depend on what earlier datagrams left
 *      behind. One fuzz input is therefore a SEQUENCE of records replayed in order.
 *
 *      FindOrAddPeer runs only after AES-CCM MIC verification succeeds, which mutated bytes
 *      cannot pass, so each record is encrypted with the real group key before being fed to
 *      SessionManager::OnMessageReceived. The encoder replicates the production encrypt
 *      sequence; it differs from PrepareMessage only in taking the source node id, message
 *      counter and control flag as parameters, which PrepareMessage fixes internally.
 *
 *      Real SessionManager, FabricTable, GroupDataProviderImpl, CryptoContext, libcrypto
 *      and the file-static gGroupPeerTable; nothing on the exercised path is stubbed.
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

// The single group id every fabric registers a key for. The epoch key is a valid
// configured key; the fuzzer varies the datagram's sourceNodeId and counter.
constexpr GroupId kGroupId       = 2;
constexpr uint16_t kTestKeysetId = 0x0123;

// Fabrics the harness attempts to install; the cert vectors cap how many succeed, so
// routing uses the achieved Fixture::fabricCount. Two or more exercise the fabric-search
// loop; one still drives the per-fabric LRU.
constexpr size_t kMaxFabrics = 4;

// Distinct epoch key per fabric so each yields a distinct derived group key.
const uint8_t kEpochKeys[kMaxFabrics][16] = {
    { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf },
    { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
    { 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf },
    { 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef },
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

// Built once and reused. gGroupPeerTable is file-static, so the per-input reset below is
// what makes an input self-contained and its result reproducible.
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

// Install one fabric from a distinct NOC asset, plus its group key. Each successful add
// yields a distinct fabric index.
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

// Encode a valid, privacy-protected group datagram with fabric `slot`'s real key. Mirrors
// the production encrypt sequence, differing from PrepareMessage only in taking the node id,
// counter and control flag as parameters rather than fixing them.
CHIP_ERROR EncodeGroupDatagram(Fixture & fx, size_t slot, NodeId nodeId, uint32_t counter, bool isControl,
                               std::vector<uint8_t> & out)
{
    const FabricIndex fabricIndex = fx.fabricIndices[slot];

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);
    // NeedsAck must stay false: a group message requesting an ack is dropped before
    // FindOrAddPeer.

    const uint8_t payload[]        = { 'h', '4' };
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
        // Distinct roots give distinct compressed fabric ids; NodeA2 adds a third fabric under
        // root A. Each successful add is one routable slot; a single fabric is a valid fallback.
        InstallFabric(*fx, 0, GetRootACertAsset(), GetIAA1CertAsset(), GetNodeA1CertAsset());
        InstallFabric(*fx, 1, GetRootBCertAsset(), GetIAB1CertAsset(), GetNodeB1CertAsset());
        InstallFabric(*fx, 2, GetRootACertAsset(), GetIAA1CertAsset(), GetNodeA2CertAsset());
        VerifyOrDie(fx->fabricCount >= 1);

        Inet::IPAddress addr;
        VerifyOrDie(Inet::IPAddress::FromString("::1", addr));
        fx->peer = PeerAddress::UDP(addr, CHIP_PORT);

        gFixture = fx;

        // The fixture is intentionally leaked, but the file-static Inet EndPointManagers
        // VerifyOrDie in their destructors unless the layers were shut down first -- without
        // this hook the run ends in a SIGABRT that reads as a crash.
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

// Base for the generated node IDs. 256 distinct values (via nodeSel) is enough to over-fill the
// 15/2 caps and force many evictions + MRU re-inserts.
constexpr NodeId kNodeBase = 0x0000000100000000ull;

// Reset gGroupPeerTable to empty so each fuzz input is self-contained and any crash reproduces
// standalone. SessionManager::FabricRemoved is public and clears that fabric's slot.
void ResetPeerTable(Fixture & fx)
{
    for (size_t i = 0; i < fx.fabricCount; i++)
    {
        fx.sessionManager.FabricRemoved(fx.fabricIndices[i]);
    }
}

// Drive a sequence of datagrams through the real receive function, accumulating state in
// gGroupPeerTable. The fuzzer controls the record count and every per-record field.
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

        PacketBufferHandle msg = MessagePacketBuffer::NewWithData(datagram.data(), datagram.size());
        if (msg.IsNull())
        {
            continue;
        }

        fx.sessionManager.OnMessageReceived(fx.peer, std::move(msg));
    }
}

// Programmatic seeds (record sequences), one per transition the table cares about, so the
// mutator starts with a representative of every fill/evict/re-insert/first-add path.
std::vector<std::vector<Record>> GroupCounterSeeds()
{
    std::vector<std::vector<Record>> seeds;

    auto data = [](uint8_t node) { return Record{ 0, node, false, 1, false }; };

    // 1. A single valid data-message record (baseline reach of FindOrAddPeer non-control branch).
    seeds.push_back({ data(1) });

    // 2. A control-message record. SecureGroupMessageDispatch requires !IsSecureSessionControlMsg()
    //    (MessageHeader.h IsValidGroupMsg), so this is rejected before FindOrAddPeer and the
    //    control table is never reached from here. Kept as the negative case for that guard.
    seeds.push_back({ Record{ 0, 1, true, 1, false } });

    // 3. > MAX_GROUP_DATA_PEERS (17) distinct data nodes, one fabric: fill -> evict past cap 15.
    {
        std::vector<Record> s;
        for (uint8_t n = 1; n <= 17; n++)
        {
            s.push_back(data(n));
        }
        seeds.push_back(std::move(s));
    }

    // 4. Several control records in a row: still the rejection guard, not the control cap.
    //    CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS is unreachable through this function.
    seeds.push_back({ Record{ 0, 1, true, 1, false }, Record{ 0, 2, true, 1, false }, Record{ 0, 3, true, 1, false } });

    // 5. Mix >=2 fabrics: drives the first-add branch for a second mGroupFabrics slot.
    seeds.push_back({ Record{ 0, 1, false, 1, false }, Record{ 1, 2, false, 1, false }, Record{ 2, 3, false, 1, false } });

    // 6. Fill the data cap, then re-send an already-present node id: drives the search-hit
    //    MRU-move ShiftAndInsert(list, i, ...) path.
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

    // 7. Replay-window walk on ONE node: adopt a base counter, then revisit positions around
    //    it. Covers FutureCounter, the in-window bitset test, equal-to-max and before-window.
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

// The outgoing side of the same translation unit: GroupOutgoingCounters::GetCounter /
// IncrementCounter are reached only from SessionManager::PrepareMessage (:224-225), which the
// hand-rolled EncodeGroupDatagram above deliberately bypasses in order to choose the counter.
// This case sends through the real PrepareMessage instead, then loops the result back in, so
// the send-side counter advances and a canonical frame also traverses the receive path.
void GroupSendThenReceiveDoesNotCrash(uint8_t fabricSel, uint8_t typeSel, const std::vector<uint8_t> & payload)
{
    Fixture & fx = GetFixture();

    const size_t slot = fabricSel % fx.fabricCount;

    Transport::OutgoingGroupSession outgoingSession(kGroupId, fx.fabricIndices[slot]);
    SessionHandle outgoingHandle(outgoingSession);
    SessionHolder outgoingHolder(outgoingHandle);

    // typeSel picks the send-side counter branch. The two SecureChannel counter-sync types are
    // what SessionManager::IsControlMessage recognises, and GetCounter/IncrementCounter run at
    // :224-225 before the IsValidGroupMsg() bail at :259, so the control branch is still covered
    // even though a control group send cannot complete.
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

FUZZ_TEST(FuzzSessionManagerGroupCounterPW, GroupSendThenReceiveDoesNotCrash)
    .WithDomains(Arbitrary<uint8_t>(), Arbitrary<uint8_t>(), VectorOf(Arbitrary<uint8_t>()).WithMaxSize(128));

// The peer table drives replay protection: each entry carries the PeerMessageCounter state
// for one sender, and the LRU maintenance moves those entries around on every insert. The
// two cases below drive GroupPeerTable directly, because the property worth checking is not
// "does it crash" but "does a sender's counter state stay bound to that sender" -- if an
// entry's counter were aliased to a neighbour by a shuffle, replays would be accepted with
// no memory error for a sanitizer to see.

constexpr FabricIndex kOracleFabric = 1;
constexpr NodeId kOracleNodeBase    = 0x0000000200000000ull;

// A committed counter must still be recognised as a replay after the victim's entry has been
// shuffled down the list by unrelated inserts.
void PeerCounterSurvivesLruChurn(uint8_t victimSel, uint8_t churn, uint32_t counterValue)
{
    Transport::GroupPeerTable table;

    const NodeId victim = kOracleNodeBase + victimSel;

    Transport::PeerMessageCounter * counter = nullptr;
    ASSERT_EQ(table.FindOrAddPeer(kOracleFabric, victim, false, counter), CHIP_NO_ERROR);
    ASSERT_NE(counter, nullptr);
    ASSERT_EQ(counter->VerifyOrTrustFirstGroup(counterValue), CHIP_NO_ERROR);
    counter->CommitGroup(counterValue);

    // Stay strictly under CHIP_CONFIG_MAX_GROUP_DATA_PEERS so the victim cannot be evicted;
    // eviction is the other case's subject.
    const uint8_t inserts = churn % (CHIP_CONFIG_MAX_GROUP_DATA_PEERS - 1);
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

// The mirror property. Whether a peer that no longer fits is admitted with fresh state or
// refused outright is the table's policy choice, so both outcomes are accepted here; what
// must hold either way is that the peer is never handed a counter carrying some *other*
// peer's history, which is what a mis-shuffled list would produce.
void PeerCounterCrowdedOutPeerKeepsNoForeignState(uint8_t victimSel, uint32_t victimCounter)
{
    // Far enough apart that neither counter can fall in the other's 32-entry window, so a
    // duplicate verdict below can only come from state that belongs to a different peer.
    const uint32_t otherCounter = victimCounter + 0x10000u;

    Transport::GroupPeerTable table;

    const NodeId victim = kOracleNodeBase + victimSel;

    Transport::PeerMessageCounter * counter = nullptr;
    ASSERT_EQ(table.FindOrAddPeer(kOracleFabric, victim, false, counter), CHIP_NO_ERROR);
    ASSERT_NE(counter, nullptr);
    ASSERT_EQ(counter->VerifyOrTrustFirstGroup(victimCounter), CHIP_NO_ERROR);
    counter->CommitGroup(victimCounter);

    // Fill every remaining slot, each peer committing a counter of its own.
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
