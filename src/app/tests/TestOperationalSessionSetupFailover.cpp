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

#include <algorithm>
#include <cstring>
#include <functional>
#include <initializer_list>

#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/OperationalSessionSetup.h>
#include <app/tests/AppTestContext.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>
#include <protocols/secure_channel/StatusReport.h>
#include <transport/raw/MessageHeader.h>

using namespace chip;
using namespace chip::Messaging;
using namespace chip::System::Clock::Literals;

namespace {

constexpr NodeId kPeerNodeId                                = 0x0000'0000'1234'5678ULL;
constexpr uint16_t kPeerPort                                = CHIP_PORT;
constexpr uint8_t kFullSigma1Transmissions                  = CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS + 1;
[[maybe_unused]] constexpr uint8_t kTransmissionsBeforeNext = CHIP_CONFIG_CASE_SIGMA1_TRANSMISSIONS_BEFORE_NEXT_ADDRESS;
constexpr System::Clock::Timeout kSettleSlack               = 2_s;

// The shortened first-attempt budget is off when the limit is 0 or not below the full budget; the tests of the
// shortening itself are compiled only when it is on.
#define SIGMA1_FIRST_ATTEMPT_SHORTENING_ENABLED                                                                                    \
    (CHIP_CONFIG_CASE_SIGMA1_TRANSMISSIONS_BEFORE_NEXT_ADDRESS > 0 &&                                                              \
     CHIP_CONFIG_CASE_SIGMA1_TRANSMISSIONS_BEFORE_NEXT_ADDRESS < CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS + 1)

// Advertised by the peer over DNS-SD. Idle differs from active so a deadline built on the wrong interval shows up.
const ReliableMessageProtocolConfig kPeerMrpConfig(60_ms32, 20_ms32);

Transport::PeerAddress PeerAt(const char * ip)
{
    Inet::IPAddress address;
    VerifyOrDie(Inet::IPAddress::FromString(ip, address));
    return Transport::PeerAddress::UDP(address, kPeerPort);
}

class FakeOperationalResolver : public Dnssd::Resolver
{
public:
    CHIP_ERROR Init(Inet::EndPointManager<Inet::UDPEndPoint> *) override { return CHIP_NO_ERROR; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(Dnssd::OperationalResolveDelegate * delegate) override { mDelegate = delegate; }
    CHIP_ERROR ResolveNodeId(const PeerId & peerId) override
    {
        mResolvingPeer = peerId;
        return CHIP_NO_ERROR;
    }
    void NodeIdResolutionNoLongerNeeded(const PeerId &) override {}
    CHIP_ERROR StartDiscovery(Dnssd::DiscoveryType, Dnssd::DiscoveryFilter, Dnssd::DiscoveryContext &) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR StopDiscovery(Dnssd::DiscoveryContext &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ReconfirmRecord(const char *, Inet::IPAddress, Inet::InterfaceId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    void Advertise(std::initializer_list<Transport::PeerAddress> addresses)
    {
        Dnssd::ResolvedNodeData nodeData;
        nodeData.operationalData.peerId     = mResolvingPeer;
        nodeData.resolutionData.interfaceId = Inet::InterfaceId::Null();
        nodeData.resolutionData.port        = kPeerPort;
        nodeData.resolutionData.mrpRetryIntervalIdle.emplace(kPeerMrpConfig.mIdleRetransTimeout);
        nodeData.resolutionData.mrpRetryIntervalActive.emplace(kPeerMrpConfig.mActiveRetransTimeout);
        for (const auto & address : addresses)
        {
            nodeData.resolutionData.ipAddress[nodeData.resolutionData.numIPs++] = address.GetIPAddress();
        }
        mDelegate->OnOperationalNodeResolved(nodeData);
    }

private:
    Dnssd::OperationalResolveDelegate * mDelegate = nullptr;
    PeerId mResolvingPeer;
};

// Counts the Sigma1 transmissions to each peer address and drops traffic to unreachable ones.
class PeerAddressProbe : public Testing::LoopbackTransportDelegate
{
public:
    static constexpr size_t kMaxPeers = 3;

    struct Peer
    {
        Transport::PeerAddress address;
        bool reachable = true;
        // Sigma1 transmissions lost on the way before the peer hears one.
        uint32_t sigma1sToLose         = 0;
        uint32_t sigma1s               = 0;
        uint32_t sigma1sCarryingMarker = 0;
        System::Clock::Timestamp firstSigma1Time;
    };

    // Sigma1 payload bytes whose presence is counted per peer.
    ByteSpan mSigma1Marker;

    void WillSendMessage(const Transport::PeerAddress & destination, const System::PacketBufferHandle & message) override
    {
        Peer * peer = Find(destination);
        mDropNext   = false;
        VerifyOrReturn(peer != nullptr && IsSigma1(message));
        if (Contains(message, mSigma1Marker))
        {
            peer->sigma1sCarryingMarker++;
        }
        if (peer->sigma1s++ == 0)
        {
            peer->firstSigma1Time = System::SystemClock().GetMonotonicTimestamp();
        }
        if (peer->sigma1sToLose > 0)
        {
            peer->sigma1sToLose--;
            mDropNext = true;
        }
    }

    bool ShouldDropMessage(const Transport::PeerAddress & destination) override
    {
        Peer * peer = Find(destination);
        return peer != nullptr && (!peer->reachable || mDropNext);
    }

    Peer mPeers[kMaxPeers];

private:
    Peer * Find(const Transport::PeerAddress & destination)
    {
        for (auto & peer : mPeers)
        {
            if (peer.address == destination)
            {
                return &peer;
            }
        }
        return nullptr;
    }

    static bool Contains(const System::PacketBufferHandle & message, ByteSpan marker)
    {
        VerifyOrReturnValue(!marker.empty() && message->DataLength() >= marker.size(), false);
        const uint8_t * data = message->Start();
        for (size_t i = 0; i + marker.size() <= message->DataLength(); i++)
        {
            if (memcmp(data + i, marker.data(), marker.size()) == 0)
            {
                return true;
            }
        }
        return false;
    }

    static bool IsSigma1(const System::PacketBufferHandle & message)
    {
        System::PacketBufferHandle copy = message.CloneData();
        PacketHeader packetHeader;
        PayloadHeader payloadHeader;
        VerifyOrReturnValue(!copy.IsNull() && packetHeader.DecodeAndConsume(copy) == CHIP_NO_ERROR, false);
        VerifyOrReturnValue(!packetHeader.IsEncrypted() && payloadHeader.DecodeAndConsume(copy) == CHIP_NO_ERROR, false);
        return payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::CASE_Sigma1);
    }

    bool mDropNext = false;
};

// Stands in for a responder reachable at an address: acks Sigma1 and then either stays silent or answers busy.
class Sigma1Responder : public UnsolicitedMessageHandler, public ExchangeDelegate
{
public:
    enum class Reply
    {
        kAckOnly,
        kBusy,
    };

    explicit Sigma1Responder(Reply reply) : mReply(reply) {}

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader &, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader &, System::PacketBufferHandle &&) override
    {
        mSigma1Received++;
        if (mReply == Reply::kBusy)
        {
            return ec->SendMessage(Protocols::SecureChannel::MsgType::StatusReport,
                                   Protocols::SecureChannel::StatusReport::MakeBusyStatusReportMessage(500_ms16));
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext *) override {}
    ExchangeMessageDispatch & GetMessageDispatch() override { return SessionEstablishmentExchangeDispatch::Instance(); }

    uint32_t mSigma1Received = 0;

private:
    Reply mReply;
};

class NoopReleaseDelegate : public OperationalSessionReleaseDelegate
{
public:
    void ReleaseSession(OperationalSessionSetup *) override {}
};

class CountingEstablishmentDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR error, SessionEstablishmentStage) override
    {
        mErrors++;
        mLastError = error;
        mErrorTime = System::SystemClock().GetMonotonicTimestamp();
    }

    uint32_t mErrors      = 0;
    CHIP_ERROR mLastError = CHIP_NO_ERROR;
    System::Clock::Timestamp mErrorTime;
};

class TestOperationalSessionSetupFailover : public Testing::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();
        SetMRPMode(MRPMode::kResponsive);

        mGroupDataProvider.SetStorageDelegate(&mStorage);
        mGroupDataProvider.SetSessionKeystore(&GetSessionKeystore());
        ASSERT_EQ(mGroupDataProvider.Init(), CHIP_NO_ERROR);
        const uint8_t ipk[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = {};
        uint8_t compressedFabricId[sizeof(uint64_t)];
        MutableByteSpan compressedFabricIdSpan(compressedFabricId);
        ASSERT_EQ(GetBobFabric()->GetCompressedFabricIdBytes(compressedFabricIdSpan), CHIP_NO_ERROR);
        ASSERT_EQ(
            Credentials::SetSingleIpkEpochKey(&mGroupDataProvider, GetBobFabricIndex(), ByteSpan(ipk), compressedFabricIdSpan),
            CHIP_NO_ERROR);

        mPreviousDnssd = &Dnssd::Resolver::Instance();
        Dnssd::Resolver::SetInstance(mDnssd);
        ASSERT_EQ(AddressResolve::Resolver::Instance().Init(&GetSystemLayer()), CHIP_NO_ERROR);

        mProbe.mPeers[0].address = PeerAt("fd00:0:1:1::1");
        mProbe.mPeers[1].address = PeerAt("fd00:0:1:1::2");
        mProbe.mPeers[2].address = PeerAt("fd00:0:1:1::3");
        GetLoopback().SetLoopbackTransportDelegate(&mProbe);

        mInitParams.sessionManager    = &GetSecureSessionManager();
        mInitParams.exchangeMgr       = &GetExchangeManager();
        mInitParams.fabricTable       = &GetFabricTable();
        mInitParams.groupDataProvider = &mGroupDataProvider;
    }

    void TearDown() override
    {
        DrainAndServiceIO();
        TEMPORARY_RETURN_IGNORED GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(
            Protocols::SecureChannel::MsgType::CASE_Sigma1);
        GetLoopback().SetLoopbackTransportDelegate(nullptr);
        AddressResolve::Resolver::Instance().Shutdown();
        Dnssd::Resolver::SetInstance(*mPreviousDnssd);
        mGroupDataProvider.Finish();
        SetMRPMode(MRPMode::kDefault);
        AppContext::TearDown();
    }

protected:
    static void OnConnected(void * context, ExchangeManager &, const SessionHandle &) {}
    static void OnFailure(void * context, const ScopedNodeId &, CHIP_ERROR error)
    {
        auto * self                  = static_cast<TestOperationalSessionSetupFailover *>(context);
        self->mFailure               = error;
        self->mFailureTime           = System::SystemClock().GetMonotonicTimestamp();
        self->mFailureCallbackCalled = true;
    }

    void ConnectTo(OperationalSessionSetup & setup, std::initializer_list<Transport::PeerAddress> addresses)
    {
        setup.Connect(&mOnConnected, &mOnFailure);
        mDnssd.Advertise(addresses);
    }

    void Respond(Sigma1Responder & responder)
    {
        ASSERT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1,
                                                                                &responder),
                  CHIP_NO_ERROR);
    }

    void DriveUntil(System::Clock::Timeout maxWait, std::function<bool()> done) { GetIOContext().DriveIOUntil(maxWait, done); }

    static System::Clock::Timeout FullSigma1Budget() { return CASESession::ComputeSigma1ResponseTimeout(kPeerMrpConfig); }

    // The deadline CASESession arms for the shortened first attempt: the ack windows of its first kTransmissionsBeforeNext
    // transmissions, on the larger of the peer's intervals.
    // Most Sigma1 transmissions a shortened attempt can send. At the upper limit, MRP's last retransmission can go out
    // before the deadline, so the full count is allowed there.
    [[maybe_unused]] static constexpr uint8_t kMaxShortenedSigma1Transmissions =
        (kTransmissionsBeforeNext >= CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS) ? kFullSigma1Transmissions : kFullSigma1Transmissions - 1;

    static System::Clock::Timeout ShortenedSigma1Deadline()
    {
        const System::Clock::Timeout base =
            std::max<System::Clock::Timeout>(kPeerMrpConfig.mActiveRetransTimeout, kPeerMrpConfig.mIdleRetransTimeout);
        System::Clock::Timeout deadline = System::Clock::kZero;
        for (uint8_t i = 0; i < kTransmissionsBeforeNext; i++)
        {
            deadline += ReliableMessageMgr::GetBackoff(base, i, /* computeMaxPossible */ true);
        }
        return deadline;
    }

    ScopedNodeId Peer() { return ScopedNodeId(kPeerNodeId, GetBobFabricIndex()); }

    // Runs a CASESession initiator directly, outside OperationalSessionSetup.
    void EstablishDirect(CASESession & session, CountingEstablishmentDelegate & delegate, const Transport::PeerAddress & peer,
                         const ReliableMessageProtocolConfig & peerMrpConfig = kPeerMrpConfig, bool peerActiveAtStart = false)
    {
        session.SetGroupDataProvider(&mGroupDataProvider);
        auto unauthenticated = GetSecureSessionManager().CreateUnauthenticatedSession(peer, peerMrpConfig);
        ASSERT_TRUE(unauthenticated.HasValue());
        if (peerActiveAtStart)
        {
            unauthenticated.Value()->AsUnauthenticatedSession()->MarkActiveRx();
        }
        ExchangeContext * exchange = GetExchangeManager().NewContext(unauthenticated.Value(), &session);
        ASSERT_NE(exchange, nullptr);
        ASSERT_EQ(session.EstablishSession(GetSecureSessionManager(), &GetFabricTable(), Peer(), exchange, nullptr, nullptr,
                                           &delegate, NullOptional),
                  CHIP_NO_ERROR);
    }

    TestPersistentStorageDelegate mStorage;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    FakeOperationalResolver mDnssd;
    Dnssd::Resolver * mPreviousDnssd = nullptr;
    PeerAddressProbe mProbe;
    CASEClientInitParams mInitParams;
    CASEClientPool<2> mClientPool;
    NoopReleaseDelegate mReleaseDelegate;
    Callback::Callback<OnDeviceConnected> mOnConnected{ OnConnected, this };
    Callback::Callback<OnDeviceConnectionFailure> mOnFailure{ OnFailure, this };
    CHIP_ERROR mFailure         = CHIP_NO_ERROR;
    bool mFailureCallbackCalled = false;
    System::Clock::Timestamp mFailureTime;
};

#if CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS >= 2

#if SIGMA1_FIRST_ATTEMPT_SHORTENING_ENABLED

TEST_F(TestOperationalSessionSetupFailover, DeadFirstAddressFallsThroughBeforeFullSigma1Budget)
{
    auto & first     = mProbe.mPeers[0];
    auto & second    = mProbe.mPeers[1];
    first.reachable  = false;
    second.reachable = false;
    {
        OperationalSessionSetup setup(mInitParams, &mClientPool, Peer(), &mReleaseDelegate);
        ConnectTo(setup, { first.address, second.address });

        DriveUntil(FullSigma1Budget() + kSettleSlack, [&] { return second.sigma1s > 0; });

        ASSERT_GT(first.sigma1s, 0u);
        ASSERT_GT(second.sigma1s, 0u);
        ChipLogProgress(Test, "Fell through after %u ms and %u Sigma1 transmissions; full Sigma1 budget %u ms",
                        static_cast<unsigned>((second.firstSigma1Time - first.firstSigma1Time).count()),
                        static_cast<unsigned>(first.sigma1s), static_cast<unsigned>(FullSigma1Budget().count()));
        EXPECT_LT(second.firstSigma1Time - first.firstSigma1Time, ShortenedSigma1Deadline() + kSettleSlack);
        EXPECT_GE(first.sigma1s, kTransmissionsBeforeNext);
        EXPECT_LE(first.sigma1s, kMaxShortenedSigma1Transmissions);
    }
}

TEST_F(TestOperationalSessionSetupFailover, ResumingDeadFirstAddressFallsThroughBeforeFullSigma1Budget)
{
    TestPersistentStorageDelegate resumptionBacking;
    SimpleSessionResumptionStorage resumptionStorage;
    ASSERT_EQ(resumptionStorage.Init(&resumptionBacking), CHIP_NO_ERROR);
    const uint8_t resumptionId[SessionResumptionStorage::kResumptionIdSize] = { 0xA5, 0x5A, 0xC3, 0x3C, 0x96, 0x69, 0x0F, 0xF0,
                                                                                0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
    mProbe.mSigma1Marker                                                    = ByteSpan(resumptionId);
    Crypto::P256ECDHDerivedSecret sharedSecret;
    ASSERT_EQ(sharedSecret.SetLength(sharedSecret.Capacity()), CHIP_NO_ERROR);
    ASSERT_EQ(
        resumptionStorage.Save(Peer(), SessionResumptionStorage::ConstResumptionIdView(resumptionId), sharedSecret, CATValues{}),
        CHIP_NO_ERROR);
    mInitParams.sessionResumptionStorage = &resumptionStorage;

    auto & first     = mProbe.mPeers[0];
    auto & second    = mProbe.mPeers[1];
    first.reachable  = false;
    second.reachable = false;
    {
        OperationalSessionSetup setup(mInitParams, &mClientPool, Peer(), &mReleaseDelegate);
        ConnectTo(setup, { first.address, second.address });

        DriveUntil(FullSigma1Budget() + kSettleSlack, [&] { return second.sigma1s > 0; });

        ASSERT_GT(second.sigma1s, 0u);
        EXPECT_EQ(first.sigma1sCarryingMarker, first.sigma1s);
        EXPECT_LT(second.firstSigma1Time - first.firstSigma1Time, ShortenedSigma1Deadline() + kSettleSlack);
        EXPECT_LE(first.sigma1s, kMaxShortenedSigma1Transmissions);
    }
    mProbe.mSigma1Marker = ByteSpan();
}

#if CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS >= 3

TEST_F(TestOperationalSessionSetupFailover, MiddleAddressIsShortenedAndLastKeepsFullSigma1Budget)
{
    auto & first     = mProbe.mPeers[0];
    auto & middle    = mProbe.mPeers[1];
    auto & last      = mProbe.mPeers[2];
    first.reachable  = false;
    middle.reachable = false;
    last.reachable   = false;
    {
        OperationalSessionSetup setup(mInitParams, &mClientPool, Peer(), &mReleaseDelegate);
        ConnectTo(setup, { first.address, middle.address, last.address });

        DriveUntil(3 * FullSigma1Budget() + kSettleSlack, [&] { return mFailureCallbackCalled; });

        EXPECT_LE(first.sigma1s, kMaxShortenedSigma1Transmissions);
        EXPECT_LE(middle.sigma1s, kMaxShortenedSigma1Transmissions);
        ASSERT_GT(last.sigma1s, 0u);
        EXPECT_LT(last.firstSigma1Time - first.firstSigma1Time, 2 * ShortenedSigma1Deadline() + kSettleSlack);
        ASSERT_TRUE(mFailureCallbackCalled);
        EXPECT_EQ(mFailure, CHIP_ERROR_TIMEOUT);
        EXPECT_EQ(last.sigma1s, kFullSigma1Transmissions);
        EXPECT_GE(mFailureTime - last.firstSigma1Time, FullSigma1Budget() * 9 / 10);
    }
}

#endif

#endif // SIGMA1_FIRST_ATTEMPT_SHORTENING_ENABLED

TEST_F(TestOperationalSessionSetupFailover, AcknowledgedFirstAddressKeepsFullSigma1Budget)
{
    Sigma1Responder responder(Sigma1Responder::Reply::kAckOnly);
    Respond(responder);
    auto & first     = mProbe.mPeers[0];
    auto & second    = mProbe.mPeers[1];
    second.reachable = false;
    {
        OperationalSessionSetup setup(mInitParams, &mClientPool, Peer(), &mReleaseDelegate);
        ConnectTo(setup, { first.address, second.address });

        DriveUntil(FullSigma1Budget() + kSettleSlack, [&] { return second.sigma1s > 0; });

        EXPECT_EQ(responder.mSigma1Received, 1u);
        EXPECT_EQ(first.sigma1s, 1u);
        ASSERT_GT(second.sigma1s, 0u);
        EXPECT_GE(second.firstSigma1Time - first.firstSigma1Time, FullSigma1Budget() * 9 / 10);
    }
}

#if SIGMA1_FIRST_ATTEMPT_SHORTENING_ENABLED

TEST_F(TestOperationalSessionSetupFailover, FirstAddressAcknowledgingItsLastAllowedTransmissionKeepsFullSigma1Budget)
{
    Sigma1Responder responder(Sigma1Responder::Reply::kAckOnly);
    Respond(responder);
    auto & first        = mProbe.mPeers[0];
    auto & second       = mProbe.mPeers[1];
    first.sigma1sToLose = kTransmissionsBeforeNext - 1;
    second.reachable    = false;
    {
        OperationalSessionSetup setup(mInitParams, &mClientPool, Peer(), &mReleaseDelegate);
        ConnectTo(setup, { first.address, second.address });

        DriveUntil(FullSigma1Budget() + kSettleSlack, [&] { return second.sigma1s > 0; });

        EXPECT_EQ(responder.mSigma1Received, 1u);
        EXPECT_EQ(first.sigma1s, kTransmissionsBeforeNext);
        ASSERT_GT(second.sigma1s, 0u);
        EXPECT_GE(second.firstSigma1Time - first.firstSigma1Time, FullSigma1Budget() * 9 / 10);
    }
}

#endif // SIGMA1_FIRST_ATTEMPT_SHORTENING_ENABLED

TEST_F(TestOperationalSessionSetupFailover, BusyFirstAddressFallsThroughAndLastAddressKeepsFullSigma1Budget)
{
    Sigma1Responder responder(Sigma1Responder::Reply::kBusy);
    Respond(responder);
    auto & first     = mProbe.mPeers[0];
    auto & second    = mProbe.mPeers[1];
    second.reachable = false;
    {
        OperationalSessionSetup setup(mInitParams, &mClientPool, Peer(), &mReleaseDelegate);
        ConnectTo(setup, { first.address, second.address });

        DriveUntil(FullSigma1Budget() + kSettleSlack, [&] { return mFailureCallbackCalled; });

        EXPECT_EQ(responder.mSigma1Received, 1u);
        EXPECT_EQ(first.sigma1s, 1u);
        EXPECT_EQ(second.sigma1s, kFullSigma1Transmissions);
        ASSERT_TRUE(mFailureCallbackCalled);
        EXPECT_GE(mFailureTime - second.firstSigma1Time, FullSigma1Budget() * 9 / 10);
    }
}

#else

// The resolver keeps one address, so the first never gets the shortened budget.
TEST_F(TestOperationalSessionSetupFailover, SingleResultSlotKeepsFullSigma1BudgetWithSeveralAddresses)
{
    auto & first     = mProbe.mPeers[0];
    auto & second    = mProbe.mPeers[1];
    first.reachable  = false;
    second.reachable = false;
    {
        OperationalSessionSetup setup(mInitParams, &mClientPool, Peer(), &mReleaseDelegate);
        ConnectTo(setup, { first.address, second.address });

        DriveUntil(FullSigma1Budget() + kSettleSlack, [&] { return mFailureCallbackCalled; });

        ASSERT_TRUE(mFailureCallbackCalled);
        EXPECT_EQ(mFailure, CHIP_ERROR_TIMEOUT);
        EXPECT_EQ(first.sigma1s, kFullSigma1Transmissions);
        EXPECT_EQ(second.sigma1s, 0u);
        EXPECT_GE(mFailureTime - first.firstSigma1Time, FullSigma1Budget() * 9 / 10);
    }
}

#endif

TEST_F(TestOperationalSessionSetupFailover, SingleDeadAddressKeepsFullSigma1Budget)
{
    auto & only    = mProbe.mPeers[0];
    only.reachable = false;
    {
        OperationalSessionSetup setup(mInitParams, &mClientPool, Peer(), &mReleaseDelegate);
        ConnectTo(setup, { only.address });

        DriveUntil(FullSigma1Budget() + kSettleSlack, [&] { return mFailureCallbackCalled; });

        ASSERT_TRUE(mFailureCallbackCalled);
        EXPECT_EQ(mFailure, CHIP_ERROR_TIMEOUT);
        EXPECT_EQ(only.sigma1s, kFullSigma1Transmissions);
        EXPECT_GE(mFailureTime - only.firstSigma1Time, FullSigma1Budget() * 9 / 10);
    }
}

TEST_F(TestOperationalSessionSetupFailover, Sigma1LimitOutsideRetransmitBudgetKeepsFullSigma1Budget)
{
    for (uint8_t limit : { uint8_t(0), uint8_t(CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS + 1) })
    {
        auto & peer    = mProbe.mPeers[0];
        peer.reachable = false;
        peer.sigma1s   = 0;
        CountingEstablishmentDelegate delegate;
        CASESession session;
        session.SetSigma1UnackedTransmissionLimit(limit);
        EstablishDirect(session, delegate, peer.address);

        DriveUntil(FullSigma1Budget() + kSettleSlack, [&] { return delegate.mErrors > 0; });

        EXPECT_EQ(delegate.mErrors, 1u);
        EXPECT_EQ(delegate.mLastError, CHIP_ERROR_TIMEOUT);
        EXPECT_EQ(peer.sigma1s, kFullSigma1Transmissions);
        EXPECT_GE(delegate.mErrorTime - peer.firstSigma1Time, FullSigma1Budget() * 9 / 10);
    }
}

// The peer is Active when Sigma1 first leaves and Idle by the time Sigma1 is retransmitted, so MRP waits the idle-based
// interval for the second ack. A deadline built on the Active interval alone would abort the attempt while the peer is
// still within its retransmission window. It needs a third Sigma1, so at least two retransmissions.
#if CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS >= 2
TEST_F(TestOperationalSessionSetupFailover, Sigma1AckDeadlineCoversIdleIntervalAfterPeerGoesIdle)
{
    // The active threshold ends before the first retransmission, which MRP waits the active interval for.
    const ReliableMessageProtocolConfig peerMrpConfig(400_ms32, 50_ms32, 30_ms16);
    auto & peer        = mProbe.mPeers[0];
    peer.sigma1sToLose = 2;
    Sigma1Responder responder(Sigma1Responder::Reply::kAckOnly);
    Respond(responder);
    CountingEstablishmentDelegate delegate;
    CASESession session;
    session.SetSigma1UnackedTransmissionLimit(2);
    EstablishDirect(session, delegate, peer.address, peerMrpConfig, /* peerActiveAtStart */ true);

    // The third transmission, the first the peer hears, goes out after the second idle-based wait.
    DriveUntil(3 * FullSigma1Budget(), [&] { return delegate.mErrors > 0 || responder.mSigma1Received > 0; });

    EXPECT_EQ(delegate.mErrors, 0u);
    EXPECT_EQ(responder.mSigma1Received, 1u);
    EXPECT_EQ(peer.sigma1s, 3u);
}
#endif // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS >= 2

// The deadline CASESession arms is wall-clock, but MRP sends the retransmissions from its own timer. When the event loop
// stalls, the deadline can run before MRP has sent the transmissions the limit promises, and must then wait for them
// rather than abandon the address.
//
// The stall is made by cancelling MRP's retransmit timer (ReliableMessageMgr::StopTimer) once Sigma1 is out and starting it
// again (ReliableMessageMgr::StartTimer) at a chosen time. The retransmission that fell due meanwhile then goes out late, as
// it would after a stalled loop, while the CASESession deadline timer is not touched and fires on schedule. The additional
// MRP backoff is not used for this: it lengthens the deadline and MRP's waits alike.
#if CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS >= 3
void ResumeMrpTimer(System::Layer *, void * context)
{
    static_cast<ReliableMessageMgr *>(context)->StartTimer();
}

TEST_F(TestOperationalSessionSetupFailover, Sigma1DeadlineWaitsForLimitTransmissionsWhenLoopStalls)
{
    constexpr uint8_t kLimit = 3;
    // Large enough that the waits below are far apart compared with timer scheduling noise.
    const ReliableMessageProtocolConfig peerMrpConfig(2000_ms32, 1000_ms32);
    const System::Clock::Timeout base =
        std::max<System::Clock::Timeout>(peerMrpConfig.mActiveRetransTimeout, peerMrpConfig.mIdleRetransTimeout);
    // Longest wait MRP can make after transmission i + 1 (hi), and a lower bound on the shortest (lo): jitter adds at most
    // 24.9% to it.
    auto hi = [&](uint8_t i) { return ReliableMessageMgr::GetBackoff(base, i, /* computeMaxPossible */ true); };
    auto lo = [&](uint8_t i) { return System::Clock::Timeout(hi(i).count() * 1024 / 1279); };
    // The first deadline: the longest waits of the first kLimit transmissions.
    const System::Clock::Timeout window      = hi(0) + hi(1) + hi(2);
    constexpr System::Clock::Timeout kMargin = 400_ms;
    // With one transmission out when the first deadline runs at `window`, the deadline waits another full `window`. MRP is
    // restarted so that transmission 3 goes out before that second deadline (its wait is at most hi(1)) and transmission 4,
    // another lo(1) + lo(2) later at the earliest, only after it.
    const System::Clock::Timeout resumeAt = 2 * window - hi(1) - kMargin;
    ASSERT_GT(resumeAt, window + kMargin);
    ASSERT_GE(lo(1) + lo(2), hi(1) + 2 * kMargin);

    auto & peer    = mProbe.mPeers[0];
    peer.reachable = false;
    CountingEstablishmentDelegate delegate;
    CASESession session;
    session.SetSigma1UnackedTransmissionLimit(kLimit);
    EstablishDirect(session, delegate, peer.address, peerMrpConfig);
    ASSERT_EQ(peer.sigma1s, 1u);

    ReliableMessageMgr * mrp = GetExchangeManager().GetReliableMessageMgr();
    mrp->StopTimer();
    const System::Clock::Timeout elapsed = System::SystemClock().GetMonotonicTimestamp() - peer.firstSigma1Time;
    ASSERT_EQ(GetSystemLayer().StartTimer(resumeAt - elapsed, ResumeMrpTimer, mrp), CHIP_NO_ERROR);

    // Until MRP is restarted nothing retransmits, whatever the CASESession deadline does.
    DriveUntil(resumeAt - kMargin / 2 - elapsed, [] { return false; });
    EXPECT_EQ(peer.sigma1s, 1u);

    DriveUntil(4 * window, [&] { return delegate.mErrors > 0; });

    EXPECT_EQ(delegate.mErrors, 1u);
    EXPECT_EQ(delegate.mLastError, CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(peer.sigma1s, kLimit);
    // The first deadline, with one transmission out, waited again instead of giving up: the attempt ends at the second.
    // Compared as plain milliseconds so a failure prints readable numbers.
    const uint64_t endedAfterMs = (delegate.mErrorTime - peer.firstSigma1Time).count();
    EXPECT_GE(endedAfterMs, static_cast<uint64_t>((2 * window - kMargin).count()));
    EXPECT_LT(endedAfterMs, static_cast<uint64_t>((2 * window + (hi(2) - hi(0)) / 2).count()));
    GetSystemLayer().CancelTimer(ResumeMrpTimer, mrp);
}
#endif // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS >= 3

TEST_F(TestOperationalSessionSetupFailover, DestroyedCASESessionLeavesNoSigma1Deadline)
{
    auto & peer    = mProbe.mPeers[0];
    peer.reachable = false;
    CountingEstablishmentDelegate delegate;
    {
        CASESession session;
        session.SetSigma1UnackedTransmissionLimit(1);
        EstablishDirect(session, delegate, peer.address);
        // Sigma1 left before destruction, so the deadline was armed.
        ASSERT_GT(peer.sigma1s, 0u);
    }

    // Under a memory checker a deadline left armed by the destroyed session fails here.
    DriveUntil(FullSigma1Budget() + kSettleSlack, [] { return false; });

    EXPECT_EQ(delegate.mErrors, 0u);
}

} // namespace
