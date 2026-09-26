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
 *      Sequencing, repetition and restore tests for the Sigma1 session resumption offer.
 */

#include <pw_unit_test/framework.h>

#include <array>
#include <cstring>

#include <app/CASEClient.h>
#include <app/CASEClientPool.h>
#include <app/OperationalSessionSetup.h>
#include <app/tests/AppTestContext.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/secure_channel/SessionEstablishmentDelegate.h>
#include <protocols/secure_channel/SessionResumptionStorage.h>
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>
#include <system/RAIIMockClock.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;

namespace {

constexpr NodeId kFirstPeerNodeId         = 0xBADCAB0000000031ULL;
constexpr NodeId kSecondPeerNodeId        = 0xBADCAB0000000032ULL;
constexpr uint16_t kPeerPort              = 5540;
constexpr uint8_t kSeededResumptionIdByte = 0x5c;
constexpr uint8_t kSeededSecretByte       = 0x3b;

class CountingPersistedResumptionStorage : public SimpleSessionResumptionStorage
{
public:
    CHIP_ERROR FindByScopedNodeId(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                  Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override
    {
        mLookups++;
        RecordLookup(node);
        return SimpleSessionResumptionStorage::FindByScopedNodeId(node, resumptionId, sharedSecret, peerCATs);
    }

    CHIP_ERROR DeleteAll(FabricIndex fabricIndex) override
    {
        mDeleteAlls++;
        return SimpleSessionResumptionStorage::DeleteAll(fabricIndex);
    }

    unsigned LookupsFor(const ScopedNodeId & node) const
    {
        for (const auto & entry : mPerNode)
        {
            if (entry.mCount != 0 && entry.mNode == node)
            {
                return entry.mCount;
            }
        }
        return 0;
    }

    void ResetCounters()
    {
        mLookups    = 0;
        mDeleteAlls = 0;
        mPerNode.fill(PerNode{});
    }

    unsigned mLookups    = 0;
    unsigned mDeleteAlls = 0;

private:
    struct PerNode
    {
        ScopedNodeId mNode;
        unsigned mCount = 0;
    };

    void RecordLookup(const ScopedNodeId & node)
    {
        for (auto & entry : mPerNode)
        {
            if (entry.mCount == 0 || entry.mNode == node)
            {
                entry.mNode = node;
                entry.mCount++;
                return;
            }
        }
    }

    std::array<PerNode, 4> mPerNode;
};

class CountingReleaseDelegate : public OperationalSessionReleaseDelegate
{
public:
    void ReleaseSession(OperationalSessionSetup * sessionSetup) override { mReleasedCount++; }

    unsigned mReleasedCount = 0;
};

class SilentDnssdResolver : public Dnssd::Resolver
{
public:
    CHIP_ERROR Init(EndPointManager<UDPEndPoint> * endPointManager) override { return CHIP_NO_ERROR; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(Dnssd::OperationalResolveDelegate * delegate) override { mDelegate = delegate; }
    CHIP_ERROR ResolveNodeId(const PeerId & peerId) override
    {
        mResolveCalls++;
        return CHIP_NO_ERROR;
    }
    void NodeIdResolutionNoLongerNeeded(const PeerId & peerId) override {}
    CHIP_ERROR StartDiscovery(Dnssd::DiscoveryType type, Dnssd::DiscoveryFilter filter, Dnssd::DiscoveryContext & context) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR StopDiscovery(Dnssd::DiscoveryContext & context) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ReconfirmRecord(const char * hostname, IPAddress address, InterfaceId interfaceId) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    Dnssd::OperationalResolveDelegate * mDelegate = nullptr;
    unsigned mResolveCalls                        = 0;
};

class TestOperationalSessionSetupResumptionAdversarial2 : public chip::Testing::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();
        VerifyOrReturn(!HasFailure());

        Dnssd::Resolver::SetInstance(mDnssdResolver);
        ASSERT_EQ(AddressResolve::Resolver::Instance().Init(&GetSystemLayer()), CHIP_NO_ERROR);

        mIpkStorage.ClearStorage();
        mGroupDataProvider.SetStorageDelegate(&mIpkStorage);
        mGroupDataProvider.SetSessionKeystore(&GetSessionKeystore());
        ASSERT_EQ(mGroupDataProvider.Init(), CHIP_NO_ERROR);
        ASSERT_NE(GetAliceFabric(), nullptr);
        ASSERT_EQ(InstallTestIpk(*GetAliceFabric()), CHIP_NO_ERROR);

        mResumptionBacking.ClearStorage();
        ASSERT_EQ(mResumptionStorage.Init(&mResumptionBacking), CHIP_NO_ERROR);

        GetLoopback().Reset();
        GetLoopback().mNumMessagesToDrop = chip::Testing::LoopbackTransport::kUnlimitedMessageCount;
    }

    void TearDown() override
    {
        GetLoopback().Reset();
        mGroupDataProvider.Finish();
        AddressResolve::Resolver::Instance().Shutdown();
        Dnssd::Resolver::SetInstance(Dnssd::GetDefaultResolver());
        AppContext::TearDown();
    }

protected:
    CASEClientInitParams InitParams()
    {
        CASEClientInitParams params;
        params.sessionManager           = &GetSecureSessionManager();
        params.exchangeMgr              = &GetExchangeManager();
        params.fabricTable              = &GetFabricTable();
        params.groupDataProvider        = &mGroupDataProvider;
        params.sessionResumptionStorage = &mResumptionStorage;
        return params;
    }

    ScopedNodeId FirstPeer() { return ScopedNodeId(kFirstPeerNodeId, GetAliceFabricIndex()); }
    ScopedNodeId SecondPeer() { return ScopedNodeId(kSecondPeerNodeId, GetAliceFabricIndex()); }

    void SeedPersistedRecord(const ScopedNodeId & node)
    {
        SessionResumptionStorage::ResumptionIdStorage resumptionId;
        resumptionId.fill(kSeededResumptionIdByte);
        Crypto::P256ECDHDerivedSecret sharedSecret;
        ASSERT_EQ(sharedSecret.SetLength(Crypto::P256ECDHDerivedSecret::Capacity()), CHIP_NO_ERROR);
        memset(sharedSecret.Bytes(), kSeededSecretByte, sharedSecret.Length());
        ASSERT_EQ(mResumptionStorage.Save(node, SessionResumptionStorage::ConstResumptionIdView(resumptionId), sharedSecret,
                                          kUndefinedCATs),
                  CHIP_NO_ERROR);
        mResumptionStorage.ResetCounters();
    }

    void ExpectSeededRecordStillReadable(const ScopedNodeId & node)
    {
        SessionResumptionStorage::ResumptionIdStorage resumptionId;
        Crypto::P256ECDHDerivedSecret sharedSecret;
        CATValues peerCATs;
        ASSERT_EQ(mResumptionStorage.FindByScopedNodeId(node, resumptionId, sharedSecret, peerCATs), CHIP_NO_ERROR);
        SessionResumptionStorage::ResumptionIdStorage expected;
        expected.fill(kSeededResumptionIdByte);
        EXPECT_EQ(resumptionId, expected);
        ASSERT_EQ(sharedSecret.Length(), Crypto::P256ECDHDerivedSecret::Capacity());
        EXPECT_EQ(sharedSecret.Bytes()[0], kSeededSecretByte);
    }

    uint32_t Sigma1Count() const { return GetLoopback().mSentMessageCount; }

    AddressResolve::ResolveResult ResolveResultFor(const char * address)
    {
        AddressResolve::ResolveResult result;
        IPAddress ipAddress;
        EXPECT_TRUE(IPAddress::FromString(address, ipAddress));
        result.address         = PeerAddress::UDP(ipAddress, kPeerPort);
        result.mrpRemoteConfig = GetDefaultMRPConfig();
        return result;
    }

    void DeliverAddressAndExpectSigma1(OperationalSessionSetup & setup, const char * address = "fe80::1")
    {
        const uint32_t sentBefore = Sigma1Count();
        setup.OnNodeAddressResolved(PeerId(), ResolveResultFor(address));
        ASSERT_EQ(Sigma1Count(), sentBefore + 1);
    }

    void RunAttemptUntilSigma1Sent(OperationalSessionSetup & setup, const char * address = "fe80::1")
    {
        setup.Connect(&mOnConnected, &mOnFailure);
        DeliverAddressAndExpectSigma1(setup, address);
    }

    void RunAttemptFailingWith(OperationalSessionSetup & setup, CHIP_ERROR error, SessionEstablishmentStage stage)
    {
        RunAttemptUntilSigma1Sent(setup);
        setup.OnSessionEstablishmentError(error, stage);
    }

    void AdvanceUntilResolveCallsExceed(System::Clock::Internal::MockClock & clock, unsigned calls)
    {
        for (uint32_t elapsed = 0; mDnssdResolver.mResolveCalls == calls && elapsed <= mRetryTimeout.count(); elapsed++)
        {
            clock.AdvanceMonotonic(System::Clock::Seconds16(1));
            DrainAndServiceIO();
        }
        ASSERT_GT(mDnssdResolver.mResolveCalls, calls);
    }

    // Answered through the DNS-SD delegate so the lookup completes rather than outliving the attempt.
    void AnswerLookupAndExpectSigma1(System::Clock::Internal::MockClock & clock, const ScopedNodeId & node)
    {
        const uint32_t sentBefore = Sigma1Count();
        Dnssd::ResolvedNodeData nodeData;
        nodeData.operationalData.peerId =
            GetFabricTable().FindFabricWithIndex(node.GetFabricIndex())->GetPeerIdForNode(node.GetNodeId());
        ASSERT_TRUE(IPAddress::FromString("fe80::1", nodeData.resolutionData.ipAddress[0]));
        nodeData.resolutionData.numIPs = 1;
        nodeData.resolutionData.port   = kPeerPort;
        ASSERT_NE(mDnssdResolver.mDelegate, nullptr);
        mDnssdResolver.mDelegate->OnOperationalNodeResolved(nodeData);
        for (uint32_t elapsed = 0; Sigma1Count() == sentBefore && elapsed <= CHIP_CONFIG_ADDRESS_RESOLVE_MAX_LOOKUP_TIME_MS;
             elapsed += 100)
        {
            clock.AdvanceMonotonic(System::Clock::Milliseconds32(100));
            DrainAndServiceIO();
        }
        ASSERT_EQ(Sigma1Count(), sentBefore + 1);
    }

    void GoSilentAtSigma1(OperationalSessionSetup & setup)
    {
        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    }

    SilentDnssdResolver mDnssdResolver;
    CountingPersistedResumptionStorage mResumptionStorage;
    CountingReleaseDelegate mReleaseDelegate;
    CASEClientPool<4> mClientPool;
    Callback::Callback<OnDeviceConnected> mOnConnected{ OnConnected, this };
    Callback::Callback<OnDeviceConnectionFailure> mOnFailure{ OnConnectionFailure, this };
    Callback::Callback<OnDeviceConnectionRetry> mOnRetry{ OnRetry, this };
    System::Clock::Seconds16 mRetryTimeout = System::Clock::kZero;

private:
    static void OnConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle) {}
    static void OnConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error) {}
    static void OnRetry(void * context, const ScopedNodeId & peerId, CHIP_ERROR error, System::Clock::Seconds16 retryTimeout)
    {
        static_cast<TestOperationalSessionSetupResumptionAdversarial2 *>(context)->mRetryTimeout = retryTimeout;
    }

    CHIP_ERROR InstallTestIpk(const FabricInfo & fabricInfo)
    {
        GroupDataProvider::KeySet ipkKeySet(GroupDataProvider::kIdentityProtectionKeySetId,
                                            GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
        ipkKeySet.epoch_keys[0].start_time = 0;
        memset(&ipkKeySet.epoch_keys[0].key, 0, sizeof(ipkKeySet.epoch_keys[0].key));

        uint8_t compressedId[sizeof(uint64_t)];
        MutableByteSpan compressedIdSpan(compressedId);
        ReturnErrorOnFailure(fabricInfo.GetCompressedFabricIdBytes(compressedIdSpan));
        return mGroupDataProvider.SetKeySet(fabricInfo.GetFabricIndex(), compressedIdSpan, ipkKeySet);
    }

    GroupDataProviderImpl mGroupDataProvider;
    TestPersistentStorageDelegate mIpkStorage;
    TestPersistentStorageDelegate mResumptionBacking;
};

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, SilentRetryRunRunsAtLeastOneAttemptWithoutAResumptionOffer)
{
    SeedPersistedRecord(FirstPeer());

    OperationalSessionSetup setup(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(setup);
    const unsigned offersAtSilence = mResumptionStorage.mLookups;
    ASSERT_EQ(offersAtSilence, 1u);
    setup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);

    for (unsigned i = 0; i < 6; i++)
    {
        GoSilentAtSigma1(setup);
    }

    EXPECT_EQ(mResumptionStorage.mLookups, offersAtSilence);
    EXPECT_LT(mResumptionStorage.mLookups, Sigma1Count());
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, SilenceStopsReadingThePersistedRecordWithoutRemovingIt)
{
    SeedPersistedRecord(FirstPeer());

    OperationalSessionSetup setup(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);

    GoSilentAtSigma1(setup);
    const unsigned offersAtSilence = mResumptionStorage.mLookups;
    ASSERT_EQ(offersAtSilence, 1u);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mLookups, offersAtSilence);
    EXPECT_EQ(mResumptionStorage.mDeleteAlls, 0u);

    ExpectSeededRecordStillReadable(FirstPeer());
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, EachRestoredSetupReadsThePersistedRecordExactlyOnce)
{
    SeedPersistedRecord(FirstPeer());

    for (unsigned generation = 0; generation < 4; generation++)
    {
        SCOPED_TRACE(generation);
        const unsigned before = mResumptionStorage.mLookups;

        OperationalSessionSetup setup(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);
        GoSilentAtSigma1(setup);
        RunAttemptUntilSigma1Sent(setup);

        EXPECT_EQ(mResumptionStorage.mLookups - before, 1u);
    }

    EXPECT_LT(mResumptionStorage.mLookups, Sigma1Count());
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, TwoSetupsForOnePeerTrackTheSilenceIndependently)
{
    SeedPersistedRecord(FirstPeer());

    OperationalSessionSetup silent(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);
    OperationalSessionSetup answered(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(silent);
    RunAttemptUntilSigma1Sent(answered);
    const unsigned offersAfterFirstRound = mResumptionStorage.mLookups;
    ASSERT_EQ(offersAfterFirstRound, 2u);

    silent.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    answered.OnSessionEstablishmentError(CHIP_ERROR_BUSY, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(silent);
    RunAttemptUntilSigma1Sent(answered);

    EXPECT_EQ(mResumptionStorage.mLookups, offersAfterFirstRound + 1);
    EXPECT_EQ(mResumptionStorage.LookupsFor(FirstPeer()), offersAfterFirstRound + 1);
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, RoundRobinSilentSetupsEachDropTheirOwnOfferOnce)
{
    SeedPersistedRecord(FirstPeer());
    SeedPersistedRecord(SecondPeer());

    OperationalSessionSetup first(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);
    OperationalSessionSetup second(InitParams(), &mClientPool, SecondPeer(), &mReleaseDelegate);

    for (unsigned round = 0; round < 3; round++)
    {
        SCOPED_TRACE(round);
        GoSilentAtSigma1(first);
        GoSilentAtSigma1(second);
        EXPECT_EQ(mResumptionStorage.LookupsFor(FirstPeer()), 1u);
        EXPECT_EQ(mResumptionStorage.LookupsFor(SecondPeer()), 1u);
    }

    EXPECT_LT(mResumptionStorage.mLookups, Sigma1Count());
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, SilenceStopsConsultingAnEmptyPersistedStore)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);

    GoSilentAtSigma1(setup);
    const unsigned offersAtSilence = mResumptionStorage.mLookups;
    ASSERT_EQ(offersAtSilence, 1u);

    for (unsigned i = 0; i < 3; i++)
    {
        GoSilentAtSigma1(setup);
    }

    EXPECT_EQ(mResumptionStorage.mLookups, offersAtSilence);
}

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, AutomaticRetryRunReachesAnAttemptWithoutTheResumptionOffer)
{
    // Installed before the setup arms a reattempt timer, so advancing it below fires that timer.
    System::Clock::Internal::RAIIMockClock clock;

    SeedPersistedRecord(FirstPeer());

    OperationalSessionSetup setup(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);
    setup.UpdateAttemptCount(3);
    setup.AddRetryHandler(&mOnRetry);

    setup.Connect(&mOnConnected, &mOnFailure);
    AnswerLookupAndExpectSigma1(clock, FirstPeer());

    for (int reattempt = 1; reattempt <= 2; reattempt++)
    {
        SCOPED_TRACE(reattempt);
        const unsigned resolveCalls = mDnssdResolver.mResolveCalls;
        setup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
        AdvanceUntilResolveCallsExceed(clock, resolveCalls);
        AnswerLookupAndExpectSigma1(clock, FirstPeer());
    }

    ASSERT_EQ(Sigma1Count(), 3u);
    EXPECT_LT(mResumptionStorage.mLookups, Sigma1Count());
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, AnsweringPeerIsOfferedResumptionOnEveryAttempt)
{
    SeedPersistedRecord(FirstPeer());

    OperationalSessionSetup setup(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);

    for (unsigned attempt = 0; attempt < 4; attempt++)
    {
        SCOPED_TRACE(attempt);
        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma3);
        EXPECT_EQ(mResumptionStorage.mLookups, Sigma1Count());
    }
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, DroppingTheOfferStillSendsASigma1OnEveryLaterAttempt)
{
    SeedPersistedRecord(FirstPeer());

    OperationalSessionSetup setup(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);

    GoSilentAtSigma1(setup);
    ASSERT_EQ(Sigma1Count(), 1u);

    for (unsigned attempt = 0; attempt < 5; attempt++)
    {
        SCOPED_TRACE(attempt);
        GoSilentAtSigma1(setup);
        EXPECT_EQ(Sigma1Count(), attempt + 2);
    }
    EXPECT_EQ(Sigma1Count(), 6u);
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial2, PersistedRecordSurvivesASetupThatNeverWentSilent)
{
    SeedPersistedRecord(FirstPeer());

    OperationalSessionSetup setup(InitParams(), &mClientPool, FirstPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_BUSY, SessionEstablishmentStage::kSentSigma1);
    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kReceivedSigma2);
    RunAttemptUntilSigma1Sent(setup);

    EXPECT_EQ(mResumptionStorage.mLookups, Sigma1Count());
    EXPECT_EQ(mResumptionStorage.mDeleteAlls, 0u);
    ExpectSeededRecordStillReadable(FirstPeer());
}

} // namespace
