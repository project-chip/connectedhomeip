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

#include <pw_unit_test/framework.h>

#include <type_traits>
#include <vector>

#include <controller/python/matter/case_capture/CASEHandshakeMetricsBackend.h>
#include <controller/python/matter/case_capture/CompletedCASEHandshakeQueue.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/CHIPEncoding.h>

namespace {

using chip::ByteSpan;
using chip::NodeId;
using chip::PacketHeader;
using chip::PayloadHeader;
using chip::PeerId;
using chip::Protocols::SecureChannel::MsgType;
using chip::python::CASEHandshakeMetricsBackend;
using chip::python::CASEHandshakeRecordedField;
using chip::python::CompletedCASEHandshakes;
using chip::Transport::PeerAddress;

// This node's ephemeral initiator id and the exchange, which together identify a handshake.
constexpr NodeId kLocalNodeId = 0x1122334455667788ULL;
constexpr NodeId kPeerNodeId  = 0x000000000000A1A1ULL;
constexpr uint16_t kExchange  = 0x4242;

// Deep enough that no test can lose a record to the queue being full.
constexpr uint32_t kQueueDepth = 64;
// Wait is only ever called here on a queue that already holds what the test published, so it
// never has to block; the timeout only stops a broken test from hanging.
constexpr uint32_t kDrainTimeoutMs = 100;

// StatusReport bodies are generalCode (uint16), protocolId (uint32), protocolCode (uint16), all
// little endian.
constexpr uint16_t kGeneralCodeSuccess           = 0;
constexpr uint16_t kProtocolCodeSuccess          = 0;
constexpr uint16_t kGeneralCodeFailure           = 1;
constexpr uint16_t kProtocolCodeInvalidParameter = 2;
constexpr uint16_t kGeneralCodeBusy              = 8;
constexpr uint16_t kProtocolCodeBusy             = 4;

class StatusReportBody
{
public:
    StatusReportBody(uint16_t generalCode, uint16_t protocolCode)
    {
        chip::Encoding::LittleEndian::Put16(&mBytes[0], generalCode);
        chip::Encoding::LittleEndian::Put32(&mBytes[2], chip::Protocols::SecureChannel::Id.ToFullyQualifiedSpecForm());
        chip::Encoding::LittleEndian::Put16(&mBytes[6], protocolCode);
    }

    ByteSpan Span() const { return ByteSpan(mBytes, sizeof(mBytes)); }

private:
    uint8_t mBytes[8] = {};
};

// Some interface, whatever an interface handle happens to be on this platform: an index on POSIX,
// a pointer to a netif on LwIP. Only whether an interface is stated matters to these tests, never
// which one it names, and the value is never dereferenced. Two overloads rather than `if
// constexpr`, because in a plain function both branches would still have to compile everywhere.
template <typename PlatformType, std::enable_if_t<std::is_pointer<PlatformType>::value, int> = 0>
PlatformType SomeInterfaceHandle()
{
    return reinterpret_cast<PlatformType>(sizeof(void *));
}

template <typename PlatformType, std::enable_if_t<!std::is_pointer<PlatformType>::value, int> = 0>
PlatformType SomeInterfaceHandle()
{
    return static_cast<PlatformType>(1);
}

chip::Inet::InterfaceId SomeInterface()
{
    return chip::Inet::InterfaceId(SomeInterfaceHandle<chip::Inet::InterfaceId::PlatformType>());
}

PeerAddress AddressWithInterface(const char * literal, uint16_t port, chip::Inet::InterfaceId interface)
{
    chip::Inet::IPAddress address;
    VerifyOrDie(chip::Inet::IPAddress::FromString(literal, address));
    return PeerAddress::UDP(address, port, interface);
}

PeerAddress Address(const char * literal, uint16_t port = 5540)
{
    return AddressWithInterface(literal, port, chip::Inet::InterfaceId::Null());
}

class TestCASEHandshakeMetricsBackend : public ::testing::Test
{
protected:
    void SetUp() override
    {
        CompletedCASEHandshakes().Open(kQueueDepth);
        mBackend.Reset();
    }

    void TearDown() override { CompletedCASEHandshakes().Close(); }

    // --- driving the tracing hooks the way SessionManager would ----------------------------

    void SendSigma(MsgType opcode, uint16_t exchangeId = kExchange, NodeId localId = kLocalNodeId, ByteSpan payload = ByteSpan())
    {
        PayloadHeader payloadHeader;
        payloadHeader.SetMessageType(chip::Protocols::SecureChannel::Id, chip::to_underlying(opcode)).SetExchangeID(exchangeId);
        PacketHeader packetHeader;
        packetHeader.SetSourceNodeId(localId);

        chip::Tracing::MessageSendInfo info{ chip::Tracing::OutgoingMessageType::kUnauthenticated, &payloadHeader, &packetHeader,
                                             payload, payload.size() };
        mBackend.LogMessageSend(info);
    }

    void ReceiveSigma(MsgType opcode, const PeerAddress & from, uint16_t exchangeId = kExchange, NodeId localId = kLocalNodeId,
                      ByteSpan payload = ByteSpan())
    {
        PayloadHeader payloadHeader;
        payloadHeader.SetMessageType(chip::Protocols::SecureChannel::Id, chip::to_underlying(opcode)).SetExchangeID(exchangeId);
        PacketHeader packetHeader;
        packetHeader.SetDestinationNodeId(localId);

        chip::Tracing::MessageReceivedInfo info{ chip::Tracing::IncomingMessageType::kUnauthenticated,
                                                 &payloadHeader,
                                                 &packetHeader,
                                                 nullptr,
                                                 &from,
                                                 payload,
                                                 payload.size() };
        mBackend.LogMessageReceived(info);
    }

    void ResolveNode(NodeId nodeId, const PeerAddress & address)
    {
        const PeerId peer = PeerId().SetCompressedFabricId(1).SetNodeId(nodeId);

        chip::AddressResolve::NodeLookupRequest request(peer);
        chip::Tracing::NodeLookupInfo lookup{ &request };
        mBackend.LogNodeLookup(lookup);

        chip::AddressResolve::ResolveResult result;
        result.address = address;
        chip::Tracing::NodeDiscoveredInfo discovered{ chip::Tracing::DiscoveryInfoType::kResolutionDone, &peer, &result };
        mBackend.LogNodeDiscovered(discovered);
    }

    // Everything the backend has published since the last drain.
    std::vector<PychipCASEHandshakeMetricsRecord> Drain()
    {
        std::vector<PychipCASEHandshakeMetricsRecord> records;
        PychipCASEHandshakeMetricsRecord record{};
        uint32_t dropped = 0;
        while (CompletedCASEHandshakes().Wait(record, kDrainTimeoutMs, dropped))
        {
            records.push_back(record);
        }
        return records;
    }

    CASEHandshakeMetricsBackend mBackend;
};

TEST_F(TestCASEHandshakeMetricsBackend, FullHandshakeIsPublishedOnceWithEveryPhase)
{
    const StatusReportBody success(kGeneralCodeSuccess, kProtocolCodeSuccess);

    SendSigma(MsgType::CASE_Sigma1);
    ReceiveSigma(MsgType::CASE_Sigma2, Address("fd11::1"));
    SendSigma(MsgType::CASE_Sigma3);
    ReceiveSigma(MsgType::StatusReport, Address("fd11::1"), kExchange, kLocalNodeId, success.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u);
    const auto & record = records[0];
    EXPECT_EQ(record.exchangeId, kExchange);
    EXPECT_TRUE(record.recordedFields.Has(CASEHandshakeRecordedField::kSigma1Sent));
    EXPECT_TRUE(record.recordedFields.Has(CASEHandshakeRecordedField::kSigma2Received));
    EXPECT_TRUE(record.recordedFields.Has(CASEHandshakeRecordedField::kSigma3Sent));
    EXPECT_TRUE(record.recordedFields.Has(CASEHandshakeRecordedField::kStatusReportReceived));
    EXPECT_TRUE(record.recordedFields.Has(CASEHandshakeRecordedField::kStatusReportCodes));
    EXPECT_EQ(record.statusReportGeneralCode, kGeneralCodeSuccess);
    EXPECT_EQ(record.statusReportProtocolCode, kProtocolCodeSuccess);
    EXPECT_FALSE(record.recordedFields.Has(CASEHandshakeRecordedField::kThisNodeRejectedPeer));
}

TEST_F(TestCASEHandshakeMetricsBackend, RejectedSigma3KeepsItsCodes)
{
    // A rejection is faster than an acceptance, so the duration alone would read as a good
    // result. The codes are what tell the two apart.
    const StatusReportBody rejection(kGeneralCodeFailure, kProtocolCodeInvalidParameter);

    SendSigma(MsgType::CASE_Sigma1);
    ReceiveSigma(MsgType::CASE_Sigma2, Address("fd11::1"));
    SendSigma(MsgType::CASE_Sigma3);
    ReceiveSigma(MsgType::StatusReport, Address("fd11::1"), kExchange, kLocalNodeId, rejection.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u);
    EXPECT_TRUE(records[0].recordedFields.Has(CASEHandshakeRecordedField::kStatusReportCodes));
    EXPECT_EQ(records[0].statusReportGeneralCode, kGeneralCodeFailure);
    EXPECT_EQ(records[0].statusReportProtocolCode, kProtocolCodeInvalidParameter);
}

TEST_F(TestCASEHandshakeMetricsBackend, BusyAnswerToSigma1IsStillPublished)
{
    // A responder already busy with another CASE answers Sigma1 with BUSY, so the handshake ends
    // before Sigma2. Waiting for Sigma3 would leave the record in flight and the caller told
    // nothing at all about a failed handshake.
    const StatusReportBody busy(kGeneralCodeBusy, kProtocolCodeBusy);

    SendSigma(MsgType::CASE_Sigma1);
    ReceiveSigma(MsgType::StatusReport, Address("fd11::1"), kExchange, kLocalNodeId, busy.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u);
    const auto & record = records[0];
    EXPECT_TRUE(record.recordedFields.Has(CASEHandshakeRecordedField::kStatusReportReceived));
    EXPECT_EQ(record.statusReportGeneralCode, kGeneralCodeBusy);
    EXPECT_EQ(record.statusReportProtocolCode, kProtocolCodeBusy);
    // The phases that never happened must not claim a time.
    EXPECT_FALSE(record.recordedFields.Has(CASEHandshakeRecordedField::kSigma2Received));
    EXPECT_FALSE(record.recordedFields.Has(CASEHandshakeRecordedField::kSigma3Sent));
}

TEST_F(TestCASEHandshakeMetricsBackend, Sigma2ResumeAloneDoesNotConcludeTheHandshake)
{
    // The receive hook runs before CASESession has validated the resumption MIC, so the message
    // arriving is not yet proof that resumption succeeded.
    SendSigma(MsgType::CASE_Sigma1);
    ReceiveSigma(MsgType::CASE_Sigma2Resume, Address("fd11::1"));

    EXPECT_TRUE(Drain().empty()) << "a resumption must not be published before it is acknowledged";
}

TEST_F(TestCASEHandshakeMetricsBackend, ResumptionIsPublishedOnThisNodesAcknowledgement)
{
    const StatusReportBody success(kGeneralCodeSuccess, kProtocolCodeSuccess);

    SendSigma(MsgType::CASE_Sigma1);
    ReceiveSigma(MsgType::CASE_Sigma2Resume, Address("fd11::1"));
    SendSigma(MsgType::StatusReport, kExchange, kLocalNodeId, success.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u);
    EXPECT_TRUE(records[0].recordedFields.Has(CASEHandshakeRecordedField::kSigma2ResumeReceived));
    EXPECT_FALSE(records[0].recordedFields.Has(CASEHandshakeRecordedField::kThisNodeRejectedPeer));
}

TEST_F(TestCASEHandshakeMetricsBackend, ResumptionThisNodeTurnedDownIsReportedAsARejection)
{
    const StatusReportBody rejection(kGeneralCodeFailure, kProtocolCodeInvalidParameter);

    SendSigma(MsgType::CASE_Sigma1);
    ReceiveSigma(MsgType::CASE_Sigma2Resume, Address("fd11::1"));
    SendSigma(MsgType::StatusReport, kExchange, kLocalNodeId, rejection.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u);
    EXPECT_TRUE(records[0].recordedFields.Has(CASEHandshakeRecordedField::kSigma2ResumeReceived));
    EXPECT_TRUE(records[0].recordedFields.Has(CASEHandshakeRecordedField::kThisNodeRejectedPeer));
    EXPECT_EQ(records[0].statusReportGeneralCode, kGeneralCodeFailure);
}

TEST_F(TestCASEHandshakeMetricsBackend, ConcurrentHandshakesDoNotInterleave)
{
    constexpr uint16_t kOtherExchange = 0x4343;
    const StatusReportBody success(kGeneralCodeSuccess, kProtocolCodeSuccess);

    SendSigma(MsgType::CASE_Sigma1, kExchange);
    SendSigma(MsgType::CASE_Sigma1, kOtherExchange);
    // Answered in the opposite order from the order they were started.
    ReceiveSigma(MsgType::CASE_Sigma2, Address("fd11::2"), kOtherExchange);
    ReceiveSigma(MsgType::CASE_Sigma2, Address("fd11::1"), kExchange);
    SendSigma(MsgType::CASE_Sigma3, kOtherExchange);
    ReceiveSigma(MsgType::StatusReport, Address("fd11::2"), kOtherExchange, kLocalNodeId, success.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u) << "only the handshake that finished should be published";
    EXPECT_EQ(records[0].exchangeId, kOtherExchange);
}

TEST_F(TestCASEHandshakeMetricsBackend, DiscoverySpanIsAttachedFromThePeerAddress)
{
    const PeerAddress peer = Address("fd11::9");
    const StatusReportBody success(kGeneralCodeSuccess, kProtocolCodeSuccess);

    ResolveNode(kPeerNodeId, peer);
    SendSigma(MsgType::CASE_Sigma1);
    ReceiveSigma(MsgType::CASE_Sigma2, peer);
    SendSigma(MsgType::CASE_Sigma3);
    ReceiveSigma(MsgType::StatusReport, peer, kExchange, kLocalNodeId, success.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u);
    EXPECT_TRUE(records[0].recordedFields.Has(CASEHandshakeRecordedField::kDeviceDiscovery));
    EXPECT_EQ(records[0].peerNodeId, kPeerNodeId);
}

TEST_F(TestCASEHandshakeMetricsBackend, DiscoveryMatchesWhenOnlyOneSideNamesTheInterface)
{
    // Address resolution clears the interface for anything other than an IPv6 link-local address,
    // while an inbound packet always carries the interface it arrived on. Comparing the two whole
    // addresses would never match for a peer reached through a router.
    const PeerAddress resolved = AddressWithInterface("fd11::7", 5540, chip::Inet::InterfaceId::Null());
    const PeerAddress observed = AddressWithInterface("fd11::7", 5540, SomeInterface());
    const StatusReportBody success(kGeneralCodeSuccess, kProtocolCodeSuccess);

    ResolveNode(kPeerNodeId, resolved);
    SendSigma(MsgType::CASE_Sigma1);
    ReceiveSigma(MsgType::CASE_Sigma2, observed);
    SendSigma(MsgType::CASE_Sigma3);
    ReceiveSigma(MsgType::StatusReport, observed, kExchange, kLocalNodeId, success.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u);
    EXPECT_TRUE(records[0].recordedFields.Has(CASEHandshakeRecordedField::kDeviceDiscovery));
    EXPECT_EQ(records[0].peerNodeId, kPeerNodeId);
}

TEST_F(TestCASEHandshakeMetricsBackend, NewestLookupWinsWhenTwoNodesShareAnAddress)
{
    // A device commissioned again under a new node id keeps the address it had, so the older
    // lookup is still in the table. Taking the first match would name this handshake with the
    // previous node id and report no discovery, because that span was already claimed.
    constexpr NodeId kFirstNodeId  = 0x00000000AAAAAAAAULL;
    constexpr NodeId kSecondNodeId = 0x00000000BBBBBBBBULL;
    const PeerAddress peer         = Address("fd11::5");
    const StatusReportBody success(kGeneralCodeSuccess, kProtocolCodeSuccess);

    // First handshake, which claims the first lookup's span.
    ResolveNode(kFirstNodeId, peer);
    SendSigma(MsgType::CASE_Sigma1, kExchange);
    ReceiveSigma(MsgType::CASE_Sigma2, peer, kExchange);
    SendSigma(MsgType::CASE_Sigma3, kExchange);
    ReceiveSigma(MsgType::StatusReport, peer, kExchange, kLocalNodeId, success.Span());
    ASSERT_EQ(Drain().size(), 1u);

    // Second handshake to the same address under a different node id.
    constexpr uint16_t kSecondExchange = 0x4444;
    ResolveNode(kSecondNodeId, peer);
    SendSigma(MsgType::CASE_Sigma1, kSecondExchange);
    ReceiveSigma(MsgType::CASE_Sigma2, peer, kSecondExchange);
    SendSigma(MsgType::CASE_Sigma3, kSecondExchange);
    ReceiveSigma(MsgType::StatusReport, peer, kSecondExchange, kLocalNodeId, success.Span());

    const auto records = Drain();
    ASSERT_EQ(records.size(), 1u);
    EXPECT_TRUE(records[0].recordedFields.Has(CASEHandshakeRecordedField::kDeviceDiscovery))
        << "the second handshake should get its own lookup's span, not nothing";
    EXPECT_EQ(records[0].peerNodeId, kSecondNodeId) << "the peer should be named from the newest lookup";
}

TEST_F(TestCASEHandshakeMetricsBackend, HandshakesThatNeverConcludeAreCountedOnceTheSlotsRunOut)
{
    // Every slot is filled with a handshake that gets no reply, then one more is started. The one
    // given up has to be counted, or a run would report fewer notifications than establishments
    // with nothing to explain the difference.
    EXPECT_EQ(mBackend.AbandonedCASEHandshakeCount(), 0u);

    for (uint16_t i = 0; i <= chip::python::kCASEHandshakeMetricsMaxInFlight; i++)
    {
        SendSigma(MsgType::CASE_Sigma1, static_cast<uint16_t>(0x5000 + i));
    }

    EXPECT_EQ(mBackend.AbandonedCASEHandshakeCount(), 1u);
    EXPECT_TRUE(Drain().empty()) << "a handshake that never concluded must not be published";
}

TEST_F(TestCASEHandshakeMetricsBackend, ResetForgetsWhatWasInFlight)
{
    SendSigma(MsgType::CASE_Sigma1);
    mBackend.Reset();

    // The record for that exchange is gone, so its Sigma2 has nothing to attach to.
    ReceiveSigma(MsgType::CASE_Sigma2, Address("fd11::1"));
    SendSigma(MsgType::CASE_Sigma3);

    EXPECT_TRUE(Drain().empty());
    EXPECT_EQ(mBackend.AbandonedCASEHandshakeCount(), 0u) << "Reset should clear the count as well";
}

} // namespace
