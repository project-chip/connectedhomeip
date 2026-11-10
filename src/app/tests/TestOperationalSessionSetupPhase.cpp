/*
 *    Copyright (c) 2026 Project CHIP Authors
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

// Tests OperationalSessionSetup::GetSetupPhase() across a real CASE handshake over the loopback transport.

#include <pw_unit_test/framework.h>

#include <app/CASEClientPool.h>
#include <app/OperationalSessionSetup.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/tests/CHIPCert_test_vectors.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/CASESession.h>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Crypto;
using namespace chip::TestCerts;
using namespace chip::Transport;

namespace chip {

class TestOperationalSessionSetupPhase : public Testing::LoopbackMessagingContext
{
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    void SetUp() override
    {
        ConfigInitializeNodes(false);
        Testing::LoopbackMessagingContext::SetUp();
    }

    void ServiceEvents();

    // TEST_F bodies run in a subclass, which friendship does not reach, so read the setup's state here.
    static CASEClient * CASEClientOf(OperationalSessionSetup & setup) { return setup.mCASEClient; }
    static bool IsWaitingForRetry(OperationalSessionSetup & setup)
    {
        return setup.mState == OperationalSessionSetup::State::WaitingForRetry;
    }
};

namespace {

// DNS-SD that accepts lookups and never answers, so the test hands the setup its address.
class NeverAnsweringDnssdResolver : public Dnssd::Resolver
{
public:
    CHIP_ERROR Init(Inet::EndPointManager<Inet::UDPEndPoint> *) override { return CHIP_NO_ERROR; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(Dnssd::OperationalResolveDelegate *) override {}
    CHIP_ERROR ResolveNodeId(const PeerId &) override { return CHIP_NO_ERROR; }
    void NodeIdResolutionNoLongerNeeded(const PeerId &) override {}
    CHIP_ERROR StartDiscovery(Dnssd::DiscoveryType, Dnssd::DiscoveryFilter, Dnssd::DiscoveryContext &) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR StopDiscovery(Dnssd::DiscoveryContext &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ReconfirmRecord(const char *, Inet::IPAddress, Inet::InterfaceId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
};

class SingleKeyOperationalKeystore : public OperationalKeystore
{
public:
    void Init(FabricIndex fabricIndex, Platform::UniquePtr<P256Keypair> keypair)
    {
        mFabricIndex = fabricIndex;
        mKeypair     = std::move(keypair);
    }
    void Shutdown()
    {
        mFabricIndex = kUndefinedFabricIndex;
        mKeypair     = nullptr;
    }

    bool HasPendingOpKeypair() const override { return false; }
    bool HasOpKeypairForFabric(FabricIndex) const override { return mFabricIndex != kUndefinedFabricIndex; }
    CHIP_ERROR NewOpKeypairForFabric(FabricIndex, MutableByteSpan &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ActivateOpKeypairForFabric(FabricIndex, const P256PublicKey &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR CommitOpKeypairForFabric(FabricIndex) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    void RevertPendingKeypair() override {}
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message, P256ECDSASignature & outSignature) const override
    {
        VerifyOrReturnError(mKeypair != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(fabricIndex == mFabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
        return mKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }
    P256Keypair * AllocateEphemeralKeypairForCASE() override { return Platform::New<P256Keypair>(); }
    void ReleaseEphemeralKeypair(P256Keypair * keypair) override { Platform::Delete<P256Keypair>(keypair); }

private:
    Platform::UniquePtr<P256Keypair> mKeypair;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
};

class NoopReleaseDelegate : public OperationalSessionReleaseDelegate
{
public:
    void ReleaseSession(OperationalSessionSetup *) override {}
};

class NoopPairingDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR, SessionEstablishmentStage) override {}
    void OnSessionEstablished(const SessionHandle &) override {}
};

constexpr NodeId kResponderNodeId = 0xDEDEDEDE00010001;

FabricTable gInitiatorFabrics;
FabricIndex gInitiatorFabricIndex;
GroupDataProviderImpl gInitiatorGroupDataProvider;
TestPersistentStorageDelegate gInitiatorStorage;
DefaultSessionKeystore gInitiatorSessionKeystore;
PersistentStorageOpCertStore gInitiatorOpCertStore;

FabricTable gResponderFabrics;
FabricIndex gResponderFabricIndex;
GroupDataProviderImpl gResponderGroupDataProvider;
TestPersistentStorageDelegate gResponderStorage;
SingleKeyOperationalKeystore gResponderOperationalKeystore;
DefaultSessionKeystore gResponderSessionKeystore;
PersistentStorageOpCertStore gResponderOpCertStore;

NeverAnsweringDnssdResolver gDnssdResolver;

CHIP_ERROR InitFabricTable(FabricTable & fabricTable, TestPersistentStorageDelegate * storage, OperationalKeystore * opKeystore,
                           PersistentStorageOpCertStore * opCertStore)
{
    ReturnErrorOnFailure(opCertStore->Init(storage));
    FabricTable::InitParams initParams;
    initParams.storage             = storage;
    initParams.operationalKeystore = opKeystore;
    initParams.opCertStore         = opCertStore;
    return fabricTable.Init(initParams);
}

CHIP_ERROR InitIpk(GroupDataProvider & groupDataProvider, const FabricInfo & fabricInfo)
{
    using KeySet         = GroupDataProvider::KeySet;
    using SecurityPolicy = GroupDataProvider::SecurityPolicy;

    KeySet ipkKeySet(GroupDataProvider::kIdentityProtectionKeySetId, SecurityPolicy::kTrustFirst, 1);
    ipkKeySet.epoch_keys[0].start_time = 0;
    memset(&ipkKeySet.epoch_keys[0].key, 0, sizeof(ipkKeySet.epoch_keys[0].key));

    uint8_t compressedId[sizeof(uint64_t)];
    MutableByteSpan compressedIdSpan(compressedId);
    ReturnErrorOnFailure(fabricInfo.GetCompressedFabricIdBytes(compressedIdSpan));
    return groupDataProvider.SetKeySet(fabricInfo.GetFabricIndex(), compressedIdSpan, ipkKeySet);
}

CHIP_ERROR SerializeKeypair(const ByteSpan & publicKey, const ByteSpan & privateKey, P256SerializedKeypair & out)
{
    memcpy(out.Bytes(), publicKey.data(), publicKey.size());
    memcpy(out.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
    return out.SetLength(publicKey.size() + privateKey.size());
}

CHIP_ERROR InitCredentials()
{
    gInitiatorStorage.ClearStorage();
    gInitiatorGroupDataProvider.SetStorageDelegate(&gInitiatorStorage);
    gInitiatorGroupDataProvider.SetSessionKeystore(&gInitiatorSessionKeystore);
    ReturnErrorOnFailure(gInitiatorGroupDataProvider.Init());
    ReturnErrorOnFailure(InitFabricTable(gInitiatorFabrics, &gInitiatorStorage, nullptr, &gInitiatorOpCertStore));
    {
        P256SerializedKeypair opKey;
        ReturnErrorOnFailure(SerializeKeypair(sTestCert_Node01_02_PublicKey, sTestCert_Node01_02_PrivateKey, opKey));
        ReturnErrorOnFailure(gInitiatorFabrics.AddNewFabricForTest(ByteSpan(sTestCert_Root01_Chip), ByteSpan{},
                                                                   ByteSpan(sTestCert_Node01_02_Chip),
                                                                   ByteSpan(opKey.ConstBytes(), opKey.Length()), &gInitiatorFabricIndex));
    }
    const FabricInfo * initiatorFabric = gInitiatorFabrics.FindFabricWithIndex(gInitiatorFabricIndex);
    VerifyOrReturnError(initiatorFabric != nullptr, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(InitIpk(gInitiatorGroupDataProvider, *initiatorFabric));

    gResponderStorage.ClearStorage();
    gResponderGroupDataProvider.SetStorageDelegate(&gResponderStorage);
    gResponderGroupDataProvider.SetSessionKeystore(&gResponderSessionKeystore);
    ReturnErrorOnFailure(gResponderGroupDataProvider.Init());
    {
        P256SerializedKeypair opKey;
        ReturnErrorOnFailure(SerializeKeypair(sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_PrivateKey, opKey));
        auto keypair = Platform::MakeUnique<P256Keypair>();
        VerifyOrReturnError(keypair, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(keypair->Deserialize(opKey));
        gResponderOperationalKeystore.Init(1, std::move(keypair));
        ReturnErrorOnFailure(
            InitFabricTable(gResponderFabrics, &gResponderStorage, &gResponderOperationalKeystore, &gResponderOpCertStore));
        ReturnErrorOnFailure(gResponderFabrics.AddNewFabricForTest(ByteSpan(sTestCert_Root01_Chip), ByteSpan(sTestCert_ICA01_Chip),
                                                                   ByteSpan(sTestCert_Node01_01_Chip), ByteSpan{},
                                                                   &gResponderFabricIndex));
    }
    const FabricInfo * responderFabric = gResponderFabrics.FindFabricWithIndex(gResponderFabricIndex);
    VerifyOrReturnError(responderFabric != nullptr, CHIP_ERROR_INTERNAL);
    return InitIpk(gResponderGroupDataProvider, *responderFabric);
}

} // namespace

void TestOperationalSessionSetupPhase::SetUpTestSuite()
{
    LoopbackMessagingContext::SetUpTestSuite();
    ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    ASSERT_EQ(InitCredentials(), CHIP_NO_ERROR);
    DeviceLayer::SetSystemLayerForTesting(&GetSystemLayer());
    Dnssd::Resolver::SetInstance(gDnssdResolver);
}

void TestOperationalSessionSetupPhase::TearDownTestSuite()
{
    AddressResolve::Resolver::Instance().Shutdown();
    DeviceLayer::SetSystemLayerForTesting(nullptr);
    gResponderOperationalKeystore.Shutdown();
    gInitiatorFabrics.DeleteAllFabrics();
    gResponderFabrics.DeleteAllFabrics();
    gInitiatorStorage.ClearStorage();
    gResponderStorage.ClearStorage();
    gInitiatorGroupDataProvider.Finish();
    gResponderGroupDataProvider.Finish();
    gInitiatorOpCertStore.Finish();
    gResponderOpCertStore.Finish();
    DeviceLayer::PlatformMgr().Shutdown();
    LoopbackMessagingContext::TearDownTestSuite();
}

void TestOperationalSessionSetupPhase::ServiceEvents()
{
    for (int i = 0; i < 3; ++i)
    {
        DrainAndServiceIO();
        EXPECT_SUCCESS(DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t) -> void { TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().StopEventLoopTask(); }, (intptr_t) nullptr));
        DeviceLayer::PlatformMgr().RunEventLoop();
    }
}

TEST_F(TestOperationalSessionSetupPhase, PhaseFollowsTheHandshake)
{
    ASSERT_EQ(AddressResolve::Resolver::Instance().Init(&GetSystemLayer()), CHIP_NO_ERROR);

    // The responder answers Sigma1 with Sigma2; the loopback then drops the initiator's Sigma3, so the initiator waits in
    // kSentSigma3.
    CASESession responder;
    NoopPairingDelegate responderDelegate;
    responder.SetGroupDataProvider(&gResponderGroupDataProvider);
    auto & loopback                            = GetLoopback();
    loopback.mNumMessagesToAllowBeforeDropping = 2;
    loopback.mNumMessagesToDrop                = 1;
    ASSERT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1,
                                                                            &responder),
              CHIP_NO_ERROR);
    ASSERT_EQ(responder.PrepareForSessionEstablishment(GetSecureSessionManager(), &gResponderFabrics, nullptr, nullptr,
                                                       &responderDelegate, ScopedNodeId(), NullOptional),
              CHIP_NO_ERROR);

    CASEClientInitParams params;
    params.sessionManager           = &GetSecureSessionManager();
    params.exchangeMgr              = &GetExchangeManager();
    params.fabricTable              = &gInitiatorFabrics;
    params.groupDataProvider        = &gInitiatorGroupDataProvider;
    params.sessionResumptionStorage = nullptr;
    params.mrpLocalConfig           = MakeOptional(GetDefaultMRPConfig());
    ASSERT_EQ(params.Validate(), CHIP_NO_ERROR);

    CASEClientPool<1> clientPool;
    NoopReleaseDelegate releaseDelegate;
    ScopedNodeId peer(kResponderNodeId, gInitiatorFabricIndex);
    auto * setup = Platform::New<OperationalSessionSetup>(params, &clientPool, peer, &releaseDelegate);
    ASSERT_NE(setup, nullptr);

    Callback::Callback<OnDeviceConnected> onConnected([](void *, Messaging::ExchangeManager &, const SessionHandle &) {}, nullptr);
    Callback::Callback<OnDeviceConnectionFailure> onFailure([](void *, const ScopedNodeId &, CHIP_ERROR) {}, nullptr);
    setup->Connect(&onConnected, &onFailure);
    EXPECT_EQ(setup->GetSetupPhase(), OperationalSessionSetup::SetupPhase::kResolvingAddress);

    AddressResolve::ResolveResult result;
    result.address         = GetBobAddress();
    result.mrpRemoteConfig = GetDefaultMRPConfig();
    setup->OnNodeAddressResolved(PeerId(gInitiatorFabrics.FindFabricWithIndex(gInitiatorFabricIndex)->GetCompressedFabricId(),
                                        kResponderNodeId),
                                 result);

    // Sigma1 is out and the responder has not answered yet.
    ASSERT_NE(CASEClientOf(*setup), nullptr);
    ASSERT_EQ(CASEClientOf(*setup)->GetCASESessionState(), CASESession::State::kSentSigma1);
    EXPECT_EQ(setup->GetSetupPhase(), OperationalSessionSetup::SetupPhase::kAwaitingSigma2);

    // The responder answers with Sigma2; the initiator sends Sigma3, which the loopback drops.
    ServiceEvents();
    ASSERT_NE(CASEClientOf(*setup), nullptr);
    ASSERT_EQ(CASEClientOf(*setup)->GetCASESessionState(), CASESession::State::kSentSigma3);
    EXPECT_EQ(setup->GetSetupPhase(), OperationalSessionSetup::SetupPhase::kOther);

    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    loopback.mNumMessagesToAllowBeforeDropping = 0;
    loopback.mNumMessagesToDrop                = 0;
    EXPECT_SUCCESS(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1));
    Platform::Delete(setup);
    GetExchangeManager().CloseAllContextsForDelegate(nullptr);
    ServiceEvents();
}

TEST_F(TestOperationalSessionSetupPhase, PhaseWhileWaitingForRetry)
{
    ASSERT_EQ(AddressResolve::Resolver::Instance().Init(&GetSystemLayer()), CHIP_NO_ERROR);

    // No responder: the Sigma1 goes unanswered.
    CASEClientInitParams params;
    params.sessionManager           = &GetSecureSessionManager();
    params.exchangeMgr              = &GetExchangeManager();
    params.fabricTable              = &gInitiatorFabrics;
    params.groupDataProvider        = &gInitiatorGroupDataProvider;
    params.sessionResumptionStorage = nullptr;
    params.mrpLocalConfig           = MakeOptional(GetDefaultMRPConfig());
    ASSERT_EQ(params.Validate(), CHIP_NO_ERROR);

    CASEClientPool<1> clientPool;
    NoopReleaseDelegate releaseDelegate;
    ScopedNodeId peer(kResponderNodeId, gInitiatorFabricIndex);
    auto * setup = Platform::New<OperationalSessionSetup>(params, &clientPool, peer, &releaseDelegate);
    ASSERT_NE(setup, nullptr);

    // Two attempts: the first lookup uses one, leaving one for a retry.
    setup->UpdateAttemptCount(2);
    Callback::Callback<OnDeviceConnected> onConnected([](void *, Messaging::ExchangeManager &, const SessionHandle &) {}, nullptr);
    Callback::Callback<OnDeviceConnectionFailure> onFailure([](void *, const ScopedNodeId &, CHIP_ERROR) {}, nullptr);
    setup->Connect(&onConnected, &onFailure);

    AddressResolve::ResolveResult result;
    result.address         = GetBobAddress();
    result.mrpRemoteConfig = GetDefaultMRPConfig();
    setup->OnNodeAddressResolved(PeerId(gInitiatorFabrics.FindFabricWithIndex(gInitiatorFabricIndex)->GetCompressedFabricId(),
                                        kResponderNodeId),
                                 result);
    ASSERT_NE(CASEClientOf(*setup), nullptr);
    ASSERT_EQ(CASEClientOf(*setup)->GetCASESessionState(), CASESession::State::kSentSigma1);

    // The attempt times out with an attempt left, so the setup waits to retry.
    setup->OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT, SessionEstablishmentStage::kSentSigma1);
    ASSERT_TRUE(IsWaitingForRetry(*setup));
    EXPECT_EQ(setup->GetSetupPhase(), OperationalSessionSetup::SetupPhase::kWaitingForRetry);

    Platform::Delete(setup);
    GetExchangeManager().CloseAllContextsForDelegate(nullptr);
    ServiceEvents();
}

} // namespace chip
