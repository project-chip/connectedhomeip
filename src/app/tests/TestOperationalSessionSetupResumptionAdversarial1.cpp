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
 *      State-matrix and boundary tests for the Sigma1 session resumption offer.
 */

#include <pw_unit_test/framework.h>

#include <utility>
#include <vector>

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

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;

namespace {

constexpr NodeId kPeerNodeId       = 0xFEEDFACE00000021ULL;
constexpr NodeId kSecondPeerNodeId = 0xFEEDFACE00000022ULL;
constexpr uint16_t kPeerPort       = 5540;

constexpr SessionEstablishmentStage kAllStages[] = {
    SessionEstablishmentStage::kUnknown,    SessionEstablishmentStage::kNotInKeyExchange,
    SessionEstablishmentStage::kSentSigma1, SessionEstablishmentStage::kReceivedSigma1,
    SessionEstablishmentStage::kSentSigma2, SessionEstablishmentStage::kReceivedSigma2,
    SessionEstablishmentStage::kSentSigma3, SessionEstablishmentStage::kReceivedSigma3,
};

class CountingSessionResumptionStorage : public SessionResumptionStorage
{
public:
    CountingSessionResumptionStorage() { mResumptionId.fill(0x5c); }

    CHIP_ERROR FindByScopedNodeId(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                  Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override
    {
        mTotalLookups++;
        CounterFor(node)++;
        resumptionId = mResumptionId;
        ReturnErrorOnFailure(sharedSecret.SetLength(Crypto::P256ECDHDerivedSecret::Capacity()));
        memset(sharedSecret.Bytes(), 0xc5, sharedSecret.Length());
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

    unsigned LookupsFor(const ScopedNodeId & node) const
    {
        for (const auto & entry : mLookups)
        {
            if (entry.first == node)
            {
                return entry.second;
            }
        }
        return 0;
    }

    unsigned mTotalLookups = 0;

private:
    unsigned & CounterFor(const ScopedNodeId & node)
    {
        for (auto & entry : mLookups)
        {
            if (entry.first == node)
            {
                return entry.second;
            }
        }
        mLookups.emplace_back(node, 0u);
        return mLookups.back().second;
    }

    std::vector<std::pair<ScopedNodeId, unsigned>> mLookups;
    ResumptionIdStorage mResumptionId;
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

class TestOperationalSessionSetupResumptionAdversarial1 : public chip::Testing::AppContext
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
        ASSERT_NE(GetBobFabric(), nullptr);
        ASSERT_EQ(InstallTestIpk(*GetAliceFabric()), CHIP_NO_ERROR);
        ASSERT_EQ(InstallTestIpk(*GetBobFabric()), CHIP_NO_ERROR);

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

    ScopedNodeId AlicePeer() { return ScopedNodeId(kPeerNodeId, GetAliceFabricIndex()); }
    ScopedNodeId AliceSecondPeer() { return ScopedNodeId(kSecondPeerNodeId, GetAliceFabricIndex()); }
    ScopedNodeId BobPeer() { return ScopedNodeId(kPeerNodeId, GetBobFabricIndex()); }

    AddressResolve::ResolveResult ResolveResultFor(const char * address)
    {
        AddressResolve::ResolveResult result;
        IPAddress ipAddress;
        EXPECT_TRUE(IPAddress::FromString(address, ipAddress));
        result.address         = PeerAddress::UDP(ipAddress, kPeerPort);
        result.mrpRemoteConfig = GetDefaultMRPConfig();
        return result;
    }

    void RunAttemptUntilSigma1Sent(OperationalSessionSetup & setup, const char * address = "fe80::1")
    {
        setup.Connect(&mOnConnected, &mOnFailure);
        const uint32_t sentBefore = GetLoopback().mSentMessageCount;
        setup.OnNodeAddressResolved(PeerId(), ResolveResultFor(address));
        ASSERT_EQ(GetLoopback().mSentMessageCount, sentBefore + 1);
    }

    void RunAttemptFailingWith(OperationalSessionSetup & setup, CHIP_ERROR error, SessionEstablishmentStage stage)
    {
        RunAttemptUntilSigma1Sent(setup);
        setup.OnSessionEstablishmentError(error, stage);
    }

    void ExpectStageKeepsOfferAcrossAFreshSetup(CHIP_ERROR error, SessionEstablishmentStage stage, unsigned expectedDelta)
    {
        const unsigned before = mResumptionStorage.mTotalLookups;

        OperationalSessionSetup setup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);
        RunAttemptFailingWith(setup, error, stage);
        RunAttemptUntilSigma1Sent(setup);

        EXPECT_EQ(mResumptionStorage.mTotalLookups - before, expectedDelta);
    }

    CountingSessionResumptionStorage mResumptionStorage;
    CountingReleaseDelegate mReleaseDelegate;
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

    SilentDnssdResolver mDnssdResolver;
    GroupDataProviderImpl mGroupDataProvider;
    TestPersistentStorageDelegate mIpkStorage;
};

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, TimeoutAtSentSigma1IsTheOnlyStageThatDropsTheOffer)
{
    for (auto stage : kAllStages)
    {
        SCOPED_TRACE(static_cast<int>(to_underlying(stage)));
        const unsigned expectedDelta = (stage == SessionEstablishmentStage::kSentSigma1) ? 1u : 2u;
        ExpectStageKeepsOfferAcrossAFreshSetup(CHIP_ERROR_TIMEOUT, stage, expectedDelta);
    }
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, OfferCountFreezesAtTheAttemptThatObservedTheSilence)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);

    for (unsigned i = 0; i < 3; i++)
    {
        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma3);
    }
    ASSERT_EQ(mResumptionStorage.mTotalLookups, 3u);

    RunAttemptUntilSigma1Sent(setup);
    const unsigned offersAtSilence = mResumptionStorage.mTotalLookups;
    ASSERT_EQ(offersAtSilence, 4u);
    setup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);

    for (unsigned i = 0; i < 3; i++)
    {
        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    }

    EXPECT_EQ(mResumptionStorage.mTotalLookups, offersAtSilence);
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, RepeatedSilentCyclesOfferResumptionExactlyOnce)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_EQ(mResumptionStorage.mTotalLookups, 1u);

    for (unsigned i = 0; i < 6; i++)
    {
        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
        EXPECT_EQ(mResumptionStorage.mTotalLookups, 1u);
    }
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, InterleavedInformativeFailuresAfterSilenceNeverReOffer)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    const unsigned offersAtSilence = mResumptionStorage.mTotalLookups;
    ASSERT_EQ(offersAtSilence, 1u);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma3);
    RunAttemptFailingWith(setup, CHIP_ERROR_BUSY, SessionEstablishmentStage::kSentSigma1);
    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kReceivedSigma2);
    RunAttemptFailingWith(setup, CHIP_ERROR_BUSY, SessionEstablishmentStage::kReceivedSigma3);
    RunAttemptUntilSigma1Sent(setup);

    EXPECT_EQ(mResumptionStorage.mTotalLookups, offersAtSilence);
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, SilenceDoesNotReachASiblingSetupMidHandshake)
{
    OperationalSessionSetup silent(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);
    OperationalSessionSetup inFlight(InitParams(), &mClientPool, AliceSecondPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(silent);
    RunAttemptUntilSigma1Sent(inFlight);
    ASSERT_EQ(mResumptionStorage.LookupsFor(AlicePeer()), 1u);
    ASSERT_EQ(mResumptionStorage.LookupsFor(AliceSecondPeer()), 1u);

    silent.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    RunAttemptUntilSigma1Sent(silent);
    EXPECT_EQ(mResumptionStorage.LookupsFor(AlicePeer()), 1u);

    inFlight.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma3);
    RunAttemptUntilSigma1Sent(inFlight);
    EXPECT_EQ(mResumptionStorage.LookupsFor(AliceSecondPeer()), 2u);
    EXPECT_EQ(mResumptionStorage.LookupsFor(AlicePeer()), 1u);
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, SameNodeIdOnAnotherFabricKeepsItsOwnOffer)
{
    OperationalSessionSetup aliceSetup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);
    OperationalSessionSetup bobSetup(InitParams(), &mClientPool, BobPeer(), &mReleaseDelegate);

    RunAttemptUntilSigma1Sent(aliceSetup);
    RunAttemptUntilSigma1Sent(bobSetup);
    ASSERT_EQ(mResumptionStorage.LookupsFor(AlicePeer()), 1u);
    ASSERT_EQ(mResumptionStorage.LookupsFor(BobPeer()), 1u);

    aliceSetup.OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    bobSetup.OnSessionEstablishmentError(CHIP_ERROR_BUSY, SessionEstablishmentStage::kSentSigma1);

    RunAttemptUntilSigma1Sent(aliceSetup);
    RunAttemptUntilSigma1Sent(bobSetup);

    EXPECT_EQ(mResumptionStorage.LookupsFor(AlicePeer()), 1u);
    EXPECT_EQ(mResumptionStorage.LookupsFor(BobPeer()), 2u);
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, AddressResolutionFailureAfterSilenceDoesNotRestoreTheOffer)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    const unsigned offersAtSilence = mResumptionStorage.mTotalLookups;
    ASSERT_EQ(offersAtSilence, 1u);

    setup.OnNodeAddressResolutionFailed(PeerId(), CHIP_ERROR_TIMEOUT);
    RunAttemptUntilSigma1Sent(setup);

    EXPECT_EQ(mResumptionStorage.mTotalLookups, offersAtSilence);
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, BusyAtEveryStageKeepsOfferingResumption)
{
    for (auto stage : kAllStages)
    {
        SCOPED_TRACE(static_cast<int>(to_underlying(stage)));
        ExpectStageKeepsOfferAcrossAFreshSetup(CHIP_ERROR_BUSY, stage, 2u);
    }
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, AnsweringPeerIsOfferedResumptionOnEveryAttempt)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);

    unsigned attempts = 0;
    for (unsigned i = 0; i < 6; i++)
    {
        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kReceivedSigma2);
        attempts++;
        EXPECT_EQ(mResumptionStorage.mTotalLookups, attempts);
    }
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, AddressResolutionTimeoutKeepsOfferingResumption)
{
    OperationalSessionSetup setup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);

    setup.Connect(&mOnConnected, &mOnFailure);
    setup.OnNodeAddressResolutionFailed(PeerId(), CHIP_ERROR_TIMEOUT);
    ASSERT_EQ(mResumptionStorage.mTotalLookups, 0u);

    RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kReceivedSigma2);
    ASSERT_EQ(mResumptionStorage.mTotalLookups, 1u);

    setup.OnNodeAddressResolutionFailed(PeerId(), CHIP_ERROR_TIMEOUT);
    RunAttemptUntilSigma1Sent(setup);
    EXPECT_EQ(mResumptionStorage.mTotalLookups, 2u);
}

TEST_F(TestOperationalSessionSetupResumptionAdversarial1, EachFreshSetupForThePeerOffersResumptionOnce)
{
    for (unsigned i = 0; i < 3; i++)
    {
        SCOPED_TRACE(i);
        OperationalSessionSetup setup(InitParams(), &mClientPool, AlicePeer(), &mReleaseDelegate);

        RunAttemptFailingWith(setup, CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
        EXPECT_EQ(mResumptionStorage.mTotalLookups, i + 1);
    }
}

} // namespace
