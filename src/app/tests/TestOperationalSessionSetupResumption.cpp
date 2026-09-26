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
 *      Unit tests for whether OperationalSessionSetup keeps offering session resumption on the
 *      attempts that follow a CASE attempt whose Sigma1 went unanswered. A SessionResumptionStorage
 *      lookup is what puts a ResumptionID into Sigma1, so the tests count those lookups.
 */

#include <pw_unit_test/framework.h>

#include <array>

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
#include <system/RAIIMockClock.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;

namespace {

constexpr NodeId kPeerNodeId      = 0xC0FFEE0000000011ULL;
constexpr NodeId kOtherPeerNodeId = 0xC0FFEE0000000012ULL;
constexpr uint16_t kPeerPort      = 5540;

class RecordingSessionResumptionStorage : public SessionResumptionStorage
{
public:
    enum class Record
    {
        kUsable,
        kAbsent,
        kLookupFails,
    };

    RecordingSessionResumptionStorage() { mResumptionId.fill(0xa5); }

    CHIP_ERROR FindByScopedNodeId(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                  Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override
    {
        RecordOffer(node.GetNodeId());
        switch (mRecord)
        {
        case Record::kAbsent:
            return CHIP_ERROR_KEY_NOT_FOUND;
        case Record::kLookupFails:
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        case Record::kUsable:
            break;
        }
        resumptionId = mResumptionId;
        ReturnErrorOnFailure(sharedSecret.SetLength(Crypto::P256ECDHDerivedSecret::Capacity()));
        memset(sharedSecret.Bytes(), 0x5a, sharedSecret.Length());
        peerCATs = kUndefinedCATs;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FindByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node,
                                  Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR Save(const ScopedNodeId & node, ConstResumptionIdView resumptionId,
                    const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR DeleteAll(FabricIndex fabricIndex) override { return CHIP_NO_ERROR; }

    unsigned OfferCountForNode(NodeId nodeId) const
    {
        for (const auto & entry : mOffersByNode)
        {
            if (entry.mNodeId == nodeId)
            {
                return entry.mCount;
            }
        }
        return 0;
    }

    Record mRecord       = Record::kUsable;
    unsigned mOfferCount = 0;

private:
    void RecordOffer(NodeId nodeId)
    {
        mOfferCount++;
        for (auto & entry : mOffersByNode)
        {
            if (entry.mNodeId == nodeId || entry.mCount == 0)
            {
                entry.mNodeId = nodeId;
                entry.mCount++;
                return;
            }
        }
    }

    struct PerNodeOffers
    {
        NodeId mNodeId  = kUndefinedNodeId;
        unsigned mCount = 0;
    };

    std::array<PerNodeOffers, 4> mOffersByNode;
    ResumptionIdStorage mResumptionId;
};

class RecordingReleaseDelegate : public OperationalSessionReleaseDelegate
{
public:
    void ReleaseSession(OperationalSessionSetup * sessionSetup) override { mReleasedCount++; }

    unsigned mReleasedCount = 0;
};

class NoOpDnssdResolver : public Dnssd::Resolver
{
public:
    CHIP_ERROR Init(EndPointManager<UDPEndPoint> * endPointManager) override { return CHIP_NO_ERROR; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(Dnssd::OperationalResolveDelegate * delegate) override {}
    CHIP_ERROR ResolveNodeId(const PeerId & peerId) override { return CHIP_NO_ERROR; }
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
};

class TestOperationalSessionSetupResumption : public chip::Testing::AppContext
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

        // Dropping every message is what makes the peer silent rather than merely slow.
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
        CASEClientInitParams params     = InitParamsWithoutResumption();
        params.sessionResumptionStorage = &mResumptionStorage;
        return params;
    }

    CASEClientInitParams InitParamsWithoutResumption()
    {
        CASEClientInitParams params;
        params.sessionManager    = &GetSecureSessionManager();
        params.exchangeMgr       = &GetExchangeManager();
        params.fabricTable       = &GetFabricTable();
        params.groupDataProvider = &mGroupDataProvider;
        return params;
    }

    ScopedNodeId TestPeer() { return ScopedNodeId(kPeerNodeId, GetAliceFabricIndex()); }
    ScopedNodeId OtherTestPeer() { return ScopedNodeId(kOtherPeerNodeId, GetAliceFabricIndex()); }

    AddressResolve::ResolveResult ResolveResultFor(const char * address)
    {
        AddressResolve::ResolveResult result;
        IPAddress ipAddress;
        EXPECT_TRUE(IPAddress::FromString(address, ipAddress));
        result.address         = PeerAddress::UDP(ipAddress, kPeerPort);
        result.mrpRemoteConfig = GetDefaultMRPConfig();
        return result;
    }

    AddressResolve::ResolveResult ResolveResultForPeer() { return ResolveResultFor("fe80::1"); }

    void DeliverAddressAndExpectSigma1(OperationalSessionSetup & setup, const AddressResolve::ResolveResult & result)
    {
        const uint32_t sentBefore = GetLoopback().mSentMessageCount;
        setup.OnNodeAddressResolved(PeerId(), result);
        ASSERT_EQ(GetLoopback().mSentMessageCount, sentBefore + 1);
    }

    void RunAttemptUntilSigma1Sent(OperationalSessionSetup & setup, const char * address = "fe80::1")
    {
        setup.Connect(&mOnConnected, &mOnFailure);
        DeliverAddressAndExpectSigma1(setup, ResolveResultFor(address));
    }

    void RunAttemptFailingWith(OperationalSessionSetup & setup, CHIP_ERROR error, SessionEstablishmentStage stage)
    {
        RunAttemptUntilSigma1Sent(setup);
        setup.OnSessionEstablishmentError(error, stage);
    }

    void ExpectFailureKeepsOfferThenUnansweredSigma1DropsIt(CHIP_ERROR error, SessionEstablishmentStage stage)
    {
        OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

        RunAttemptFailingWith(setup, error, stage);

        RunAttemptUntilSigma1Sent(setup);
        ASSERT_EQ(mResumptionStorage.mOfferCount, 2u);
        setup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);

        RunAttemptUntilSigma1Sent(setup);
        EXPECT_EQ(mResumptionStorage.mOfferCount, 2u);
    }

    void ExpectUnansweredSigma1StopsConsultingStorage(RecordingSessionResumptionStorage::Record record)
    {
        mResumptionStorage.mRecord = record;

        OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
        ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

        RunAttemptUntilSigma1Sent(setup);
        EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
    }

    RecordingSessionResumptionStorage mResumptionStorage;
    RecordingReleaseDelegate mReleaseDelegate;
    CASEClientPool<4> mClientPool;
    Callback::Callback<OnDeviceConnected> mOnConnected{ OnConnected, this };
    Callback::Callback<OnDeviceConnectionFailure> mOnFailure{ OnConnectionFailure, this };

private:
    static void OnConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle) {}
    static void OnConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error) {}

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

    NoOpDnssdResolver mDnssdResolver;
    GroupDataProviderImpl mGroupDataProvider;
    TestPersistentStorageDelegate mIpkStorage;
};

TEST_F(TestOperationalSessionSetupResumption, FirstAttemptOffersResumption)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(setup);

    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
    EXPECT_EQ(mResumptionStorage.OfferCountForNode(kPeerNodeId), 1u);
}

TEST_F(TestOperationalSessionSetupResumption, BusyResponseToSigma1KeepsOfferingResumption)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_BUSY, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 2u);
}

TEST_F(TestOperationalSessionSetupResumption, TimeoutAfterTheHandshakeGotPastSigma1KeepsOfferingResumption)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma3);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 2u);
}

TEST_F(TestOperationalSessionSetupResumption, SecondAttemptAfterUnansweredSigma1DoesNotOfferResumption)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(setup);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);
    ASSERT_EQ(mResumptionStorage.OfferCountForNode(kPeerNodeId), 1u);

    setup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

TEST_F(TestOperationalSessionSetupResumption, ResumptionOfferStaysDroppedForEveryLaterAttempt)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

TEST_F(TestOperationalSessionSetupResumption, LaterInformativeFailureDoesNotRestoreTheResumptionOffer)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma3);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

TEST_F(TestOperationalSessionSetupResumption, BusyFailureAfterTheFallbackDoesNotRestoreTheResumptionOffer)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    RunAttemptFailingWith(setup, CHIP_ERROR_BUSY, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

TEST_F(TestOperationalSessionSetupResumption, UnansweredSigma1OnALaterAttemptAlsoDropsTheResumptionOffer)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma3);

    RunAttemptUntilSigma1Sent(setup);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 2u);
    setup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 2u);
}

TEST_F(TestOperationalSessionSetupResumption, RetryToADifferentResolvedAddressDoesNotOfferResumption)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(setup, "fe80::1");
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    setup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(setup, "fe80::2");
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

TEST_F(TestOperationalSessionSetupResumption, TimeoutAtUnknownStageKeepsOfferThenUnansweredSigma1DropsIt)
{
    ExpectFailureKeepsOfferThenUnansweredSigma1DropsIt(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kUnknown);
}

TEST_F(TestOperationalSessionSetupResumption, TimeoutBeforeKeyExchangeKeepsOfferThenUnansweredSigma1DropsIt)
{
    ExpectFailureKeepsOfferThenUnansweredSigma1DropsIt(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kNotInKeyExchange);
}

TEST_F(TestOperationalSessionSetupResumption, TimeoutAfterReceivingSigma1KeepsOfferThenUnansweredSigma1DropsIt)
{
    ExpectFailureKeepsOfferThenUnansweredSigma1DropsIt(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kReceivedSigma1);
}

TEST_F(TestOperationalSessionSetupResumption, TimeoutAfterSendingSigma2KeepsOfferThenUnansweredSigma1DropsIt)
{
    ExpectFailureKeepsOfferThenUnansweredSigma1DropsIt(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma2);
}

TEST_F(TestOperationalSessionSetupResumption, TimeoutAfterReceivingSigma2KeepsOfferThenUnansweredSigma1DropsIt)
{
    ExpectFailureKeepsOfferThenUnansweredSigma1DropsIt(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kReceivedSigma2);
}

TEST_F(TestOperationalSessionSetupResumption, TimeoutAfterReceivingSigma3KeepsOfferThenUnansweredSigma1DropsIt)
{
    ExpectFailureKeepsOfferThenUnansweredSigma1DropsIt(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kReceivedSigma3);
}

TEST_F(TestOperationalSessionSetupResumption, BusyAtSigma1KeepsOfferThenUnansweredSigma1DropsIt)
{
    ExpectFailureKeepsOfferThenUnansweredSigma1DropsIt(CHIP_ERROR_BUSY, SessionEstablishmentStage::kSentSigma1);
}

TEST_F(TestOperationalSessionSetupResumption, NonTimeoutFailureAtSigma1EndsTheSetupWithNoFurtherAttempt)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(setup);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    const uint32_t sentBeforeFailure = GetLoopback().mSentMessageCount;
    setup.OnSessionEstablishmentError(CHIP_ERROR_INTERNAL, SessionEstablishmentStage::kSentSigma1);
    EXPECT_EQ(mReleaseDelegate.mReleasedCount, 1u);

    setup.Connect(&mOnConnected, &mOnFailure);
    EXPECT_EQ(GetLoopback().mSentMessageCount, sentBeforeFailure);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

TEST_F(TestOperationalSessionSetupResumption, UnansweredSigma1StopsConsultingStorageThatHoldsNoRecord)
{
    ExpectUnansweredSigma1StopsConsultingStorage(RecordingSessionResumptionStorage::Record::kAbsent);
}

TEST_F(TestOperationalSessionSetupResumption, UnansweredSigma1StopsConsultingStorageWhoseLookupFails)
{
    ExpectUnansweredSigma1StopsConsultingStorage(RecordingSessionResumptionStorage::Record::kLookupFails);
}

TEST_F(TestOperationalSessionSetupResumption, UnansweredSigma1WithoutResumptionStorageStillRetries)
{
    OperationalSessionSetup setup(InitParamsWithoutResumption(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 0u);
}

TEST_F(TestOperationalSessionSetupResumption, EachPeerSetupTracksItsOwnUnansweredSigma1)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);
    OperationalSessionSetup otherSetup(InitParams(), &mClientPool, OtherTestPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(setup);
    RunAttemptUntilSigma1Sent(otherSetup);
    ASSERT_EQ(mResumptionStorage.OfferCountForNode(kPeerNodeId), 1u);
    ASSERT_EQ(mResumptionStorage.OfferCountForNode(kOtherPeerNodeId), 1u);

    setup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    otherSetup.OnSessionEstablishmentError(CHIP_ERROR_BUSY, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(setup);
    RunAttemptUntilSigma1Sent(otherSetup);

    EXPECT_EQ(mResumptionStorage.OfferCountForNode(kPeerNodeId), 1u);
    EXPECT_EQ(mResumptionStorage.OfferCountForNode(kOtherPeerNodeId), 2u);
}

TEST_F(TestOperationalSessionSetupResumption, ASetupForThePeerCreatedLaterOffersResumptionAgain)
{
    {
        OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
        ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);
    }

    OperationalSessionSetup laterSetup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(laterSetup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 2u);
}

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

TEST_F(TestOperationalSessionSetupResumption, SetupScheduledRetryAfterUnansweredSigma1DoesNotOfferResumption)
{
    // Installed before the setup arms its reattempt timer, so advancing it below fires that timer.
    System::Clock::Internal::RAIIMockClock clock;

    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);
    setup.UpdateAttemptCount(3);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    // Past any backoff the first reattempt can pick, short of the address-resolution timeout.
    clock.AdvanceMonotonic(System::Clock::Seconds16(30));
    DrainAndServiceIO();

    DeliverAddressAndExpectSigma1(setup, ResolveResultForPeer());
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

TEST_F(TestOperationalSessionSetupResumption, RetryAfterAnInterleavedDiscoveryFailureDoesNotOfferResumption)
{
    System::Clock::Internal::RAIIMockClock clock;

    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);
    setup.UpdateAttemptCount(3);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    clock.AdvanceMonotonic(System::Clock::Seconds16(30));
    DrainAndServiceIO();

    setup.OnNodeAddressResolutionFailed(PeerId(), CHIP_ERROR_TIMEOUT);

    DeliverAddressAndExpectSigma1(setup, ResolveResultForPeer());
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

TEST_F(TestOperationalSessionSetupResumption, RaisingTheAttemptCountDoesNotRestoreTheResumptionOffer)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    setup.UpdateAttemptCount(4);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

#else // !CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

TEST_F(TestOperationalSessionSetupResumption, ConsumerDrivenRetryAfterUnansweredSigma1DoesNotOfferResumption)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, TestPeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_EQ(mResumptionStorage.mOfferCount, 1u);

    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mOfferCount, 1u);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

} // namespace
