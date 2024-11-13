/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements unit tests for the CASESession implementation.
 */

#include <stdarg.h>

#include <pw_unit_test/framework.h>

#include <credentials/CHIPCert.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/DefaultSessionKeystore.h>
#include <errno.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/CASESession.h>

#include "credentials/tests/CHIPCert_test_vectors.h"

using namespace chip;
using namespace Credentials;
using namespace TestCerts;

using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;
using namespace chip::Crypto;

namespace chip {
class TestCASESecurePairingDelegate;

class TestCASESession : public Test::LoopbackMessagingContext
{
public:
    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite();
    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite();

    virtual void SetUp() override
    {
        ConfigInitializeNodes(false);
        chip::Test::LoopbackMessagingContext::SetUp();
    }

    void ServiceEvents();
    void SecurePairingHandshakeTestCommon(SessionManager & sessionManager, CASESession & pairingCommissioner,
                                          TestCASESecurePairingDelegate & delegateCommissioner);

    void SimulateUpdateNOCInvalidatePendingEstablishment();
};

void TestCASESession::ServiceEvents()
{
    // Takes a few rounds of this because handling IO messages may schedule work,
    // and scheduled work may queue messages for sending...
    for (int i = 0; i < 3; ++i)
    {
        DrainAndServiceIO();

        chip::DeviceLayer::PlatformMgr().ScheduleWork(
            [](intptr_t) -> void { chip::DeviceLayer::PlatformMgr().StopEventLoopTask(); }, (intptr_t) nullptr);
        chip::DeviceLayer::PlatformMgr().RunEventLoop();
    }
}

class TemporarySessionManager
{
public:
    TemporarySessionManager(TestCASESession & ctx) : mCtx(ctx)
    {
        EXPECT_EQ(CHIP_NO_ERROR,
                  mSessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &ctx.GetMessageCounterManager(), &mStorage,
                                       &ctx.GetFabricTable(), ctx.GetSessionKeystore()));
        // The setup here is really weird: we are using one session manager for
        // the actual messages we send (the PASE handshake, so the
        // unauthenticated sessions) and a different one for allocating the PASE
        // sessions.  Since our Init() set us up as the thing to handle messages
        // on the transport manager, undo that.
        mCtx.GetTransportMgr().SetSessionManager(&mCtx.GetSecureSessionManager());
    }

    ~TemporarySessionManager()
    {
        mSessionManager.Shutdown();
        // Reset the session manager on the transport again, just in case
        // shutdown messed with it.
        mCtx.GetTransportMgr().SetSessionManager(&mCtx.GetSecureSessionManager());
    }

    operator SessionManager &() { return mSessionManager; }

private:
    TestCASESession & mCtx;
    TestPersistentStorageDelegate mStorage;
    SessionManager mSessionManager;
};

CHIP_ERROR InitFabricTable(chip::FabricTable & fabricTable, chip::TestPersistentStorageDelegate * testStorage,
                           chip::Crypto::OperationalKeystore * opKeyStore,
                           chip::Credentials::PersistentStorageOpCertStore * opCertStore)
{
    ReturnErrorOnFailure(opCertStore->Init(testStorage));

    chip::FabricTable::InitParams initParams;
    initParams.storage             = testStorage;
    initParams.operationalKeystore = opKeyStore;
    initParams.opCertStore         = opCertStore;

    return fabricTable.Init(initParams);
}

class TestCASESecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR error) override
    {
        mNumPairingErrors++;
        if (error == CHIP_ERROR_BUSY)
        {
            mNumBusyResponses++;
        }
    }

    void OnSessionEstablished(const SessionHandle & session) override
    {
        mSession.Grab(session);
        mNumPairingComplete++;
    }

    SessionHolder & GetSessionHolder() { return mSession; }

    SessionHolder mSession;

    // TODO: Rename mNumPairing* to mNumEstablishment*
    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
    uint32_t mNumBusyResponses   = 0;
};

class TestOperationalKeystore : public chip::Crypto::OperationalKeystore
{
public:
    void Init(FabricIndex fabricIndex, Platform::UniquePtr<P256Keypair> keypair)
    {
        mSingleFabricIndex = fabricIndex;
        mKeypair           = std::move(keypair);
    }
    void Shutdown()
    {
        mSingleFabricIndex = kUndefinedFabricIndex;
        mKeypair           = nullptr;
    }

    bool HasPendingOpKeypair() const override { return false; }
    bool HasOpKeypairForFabric(FabricIndex fabricIndex) const override { return mSingleFabricIndex != kUndefinedFabricIndex; }

    CHIP_ERROR NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR ActivateOpKeypairForFabric(FabricIndex fabricIndex, const Crypto::P256PublicKey & nocPublicKey) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommitOpKeypairForFabric(FabricIndex fabricIndex) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    void RevertPendingKeypair() override {}

    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 Crypto::P256ECDSASignature & outSignature) const override
    {
        VerifyOrReturnError(mKeypair != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(fabricIndex == mSingleFabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
        return mKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE() override { return Platform::New<Crypto::P256Keypair>(); }

    void ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair) override { Platform::Delete<Crypto::P256Keypair>(keypair); }

protected:
    Platform::UniquePtr<P256Keypair> mKeypair;
    FabricIndex mSingleFabricIndex = kUndefinedFabricIndex;
};

#if CHIP_CONFIG_SLOW_CRYPTO
constexpr uint32_t sTestCaseMessageCount           = 8;
constexpr uint32_t sTestCaseResumptionMessageCount = 6;
#else  // CHIP_CONFIG_SLOW_CRYPTO
constexpr uint32_t sTestCaseMessageCount           = 5;
constexpr uint32_t sTestCaseResumptionMessageCount = 4;
#endif // CHIP_CONFIG_SLOW_CRYPTO

FabricTable gCommissionerFabrics;
FabricIndex gCommissionerFabricIndex;
GroupDataProviderImpl gCommissionerGroupDataProvider;
TestPersistentStorageDelegate gCommissionerStorageDelegate;
Crypto::DefaultSessionKeystore gCommissionerSessionKeystore;

FabricTable gDeviceFabrics;
FabricIndex gDeviceFabricIndex;
GroupDataProviderImpl gDeviceGroupDataProvider;
TestPersistentStorageDelegate gDeviceStorageDelegate;
TestOperationalKeystore gDeviceOperationalKeystore;
Crypto::DefaultSessionKeystore gDeviceSessionKeystore;

Credentials::PersistentStorageOpCertStore gCommissionerOpCertStore;
Credentials::PersistentStorageOpCertStore gDeviceOpCertStore;

CASEServer gPairingServer;

NodeId Node01_01 = 0xDEDEDEDE00010001;
NodeId Node01_02 = 0xDEDEDEDE00010002;

CHIP_ERROR InitTestIpk(GroupDataProvider & groupDataProvider, const FabricInfo & fabricInfo, size_t numIpks)
{
    VerifyOrReturnError((numIpks > 0) && (numIpks <= 3), CHIP_ERROR_INVALID_ARGUMENT);
    using KeySet         = chip::Credentials::GroupDataProvider::KeySet;
    using SecurityPolicy = chip::Credentials::GroupDataProvider::SecurityPolicy;

    KeySet ipkKeySet(GroupDataProvider::kIdentityProtectionKeySetId, SecurityPolicy::kTrustFirst, static_cast<uint8_t>(numIpks));

    for (size_t ipkIndex = 0; ipkIndex < numIpks; ++ipkIndex)
    {
        // Set start time to 0, 1000, 2000, etc
        ipkKeySet.epoch_keys[ipkIndex].start_time = static_cast<uint64_t>(ipkIndex * 1000);
        // Set IPK Epoch key to 00.....00, 01....01, 02.....02, etc
        memset(&ipkKeySet.epoch_keys[ipkIndex].key, static_cast<int>(ipkIndex), sizeof(ipkKeySet.epoch_keys[ipkIndex].key));
    }

    uint8_t compressedId[sizeof(uint64_t)];
    MutableByteSpan compressedIdSpan(compressedId);
    ReturnErrorOnFailure(fabricInfo.GetCompressedFabricIdBytes(compressedIdSpan));
    return groupDataProvider.SetKeySet(fabricInfo.GetFabricIndex(), compressedIdSpan, ipkKeySet);
}

CHIP_ERROR InitCredentialSets()
{
    gCommissionerStorageDelegate.ClearStorage();
    gCommissionerGroupDataProvider.SetStorageDelegate(&gCommissionerStorageDelegate);
    gCommissionerGroupDataProvider.SetSessionKeystore(&gCommissionerSessionKeystore);
    ReturnErrorOnFailure(gCommissionerGroupDataProvider.Init());

    FabricInfo commissionerFabric;
    {
        P256SerializedKeypair opKeysSerialized;

        // TODO: Rename gCommissioner* to gInitiator*
        memcpy(opKeysSerialized.Bytes(), sTestCert_Node01_02_PublicKey.data(), sTestCert_Node01_02_PublicKey.size());
        memcpy(opKeysSerialized.Bytes() + sTestCert_Node01_02_PublicKey.size(), sTestCert_Node01_02_PrivateKey.data(),
               sTestCert_Node01_02_PrivateKey.size());

        ReturnErrorOnFailure(
            opKeysSerialized.SetLength(sTestCert_Node01_02_PublicKey.size() + sTestCert_Node01_02_PrivateKey.size()));

        chip::ByteSpan rcacSpan(sTestCert_Root01_Chip);
        chip::ByteSpan icacSpan(sTestCert_ICA01_Chip);
        chip::ByteSpan nocSpan(sTestCert_Node01_02_Chip);
        chip::ByteSpan opKeySpan(opKeysSerialized.ConstBytes(), opKeysSerialized.Length());

        ReturnErrorOnFailure(
            gCommissionerFabrics.AddNewFabricForTest(rcacSpan, icacSpan, nocSpan, opKeySpan, &gCommissionerFabricIndex));
    }

    const FabricInfo * newFabric = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    VerifyOrReturnError(newFabric != nullptr, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(InitTestIpk(gCommissionerGroupDataProvider, *newFabric, /* numIpks= */ 1));

    gDeviceStorageDelegate.ClearStorage();
    gDeviceGroupDataProvider.SetStorageDelegate(&gDeviceStorageDelegate);
    gDeviceGroupDataProvider.SetSessionKeystore(&gDeviceSessionKeystore);
    ReturnErrorOnFailure(gDeviceGroupDataProvider.Init());
    FabricInfo deviceFabric;

    {
        P256SerializedKeypair opKeysSerialized;

        auto deviceOpKey = Platform::MakeUnique<Crypto::P256Keypair>();
        memcpy(opKeysSerialized.Bytes(), sTestCert_Node01_01_PublicKey.data(), sTestCert_Node01_01_PublicKey.size());
        memcpy(opKeysSerialized.Bytes() + sTestCert_Node01_01_PublicKey.size(), sTestCert_Node01_01_PrivateKey.data(),
               sTestCert_Node01_01_PrivateKey.size());

        ReturnErrorOnFailure(
            opKeysSerialized.SetLength(sTestCert_Node01_01_PublicKey.size() + sTestCert_Node01_01_PrivateKey.size()));

        ReturnErrorOnFailure(deviceOpKey->Deserialize(opKeysSerialized));

        // Use an injected operational key for device
        gDeviceOperationalKeystore.Init(1, std::move(deviceOpKey));

        ReturnErrorOnFailure(
            InitFabricTable(gDeviceFabrics, &gDeviceStorageDelegate, &gDeviceOperationalKeystore, &gDeviceOpCertStore));

        chip::ByteSpan rcacSpan(sTestCert_Root01_Chip);
        chip::ByteSpan icacSpan(sTestCert_ICA01_Chip);
        chip::ByteSpan nocSpan(sTestCert_Node01_01_Chip);

        ReturnErrorOnFailure(gDeviceFabrics.AddNewFabricForTest(rcacSpan, icacSpan, nocSpan, ByteSpan{}, &gDeviceFabricIndex));
    }

    // TODO: Validate more cases of number of IPKs on both sides
    newFabric = gDeviceFabrics.FindFabricWithIndex(gDeviceFabricIndex);
    VerifyOrReturnError(newFabric != nullptr, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(InitTestIpk(gDeviceGroupDataProvider, *newFabric, /* numIpks= */ 1));

    return CHIP_NO_ERROR;
}

void TestCASESession::SetUpTestSuite()
{
    LoopbackMessagingContext::SetUpTestSuite();

    ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

    ASSERT_EQ(
        InitFabricTable(gCommissionerFabrics, &gCommissionerStorageDelegate, /* opKeyStore = */ nullptr, &gCommissionerOpCertStore),
        CHIP_NO_ERROR);

    ASSERT_EQ(InitCredentialSets(), CHIP_NO_ERROR);

    chip::DeviceLayer::SetSystemLayerForTesting(&GetSystemLayer());
}

void TestCASESession::TearDownTestSuite()
{
    chip::DeviceLayer::SetSystemLayerForTesting(nullptr);
    gDeviceOperationalKeystore.Shutdown();
    gPairingServer.Shutdown();
    gCommissionerStorageDelegate.ClearStorage();
    gDeviceStorageDelegate.ClearStorage();
    gCommissionerFabrics.DeleteAllFabrics();
    gDeviceFabrics.DeleteAllFabrics();
    chip::DeviceLayer::PlatformMgr().Shutdown();
    LoopbackMessagingContext::TearDownTestSuite();
}

TEST_F(TestCASESession, SecurePairingWaitTest)
{
    TemporarySessionManager sessionManager(*this);
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    // Making this static to reduce stack usage, as some platforms have limits on stack size.
    static FabricTable fabrics;
    CASESession caseSession;

    EXPECT_EQ(caseSession.GetSecureSessionType(), SecureSession::Type::kCASE);

    caseSession.SetGroupDataProvider(&gDeviceGroupDataProvider);
    EXPECT_EQ(caseSession.PrepareForSessionEstablishment(sessionManager, nullptr, nullptr, nullptr, nullptr, ScopedNodeId(),
                                                         Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(caseSession.PrepareForSessionEstablishment(sessionManager, nullptr, nullptr, nullptr, &delegate, ScopedNodeId(),
                                                         Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(caseSession.PrepareForSessionEstablishment(sessionManager, &fabrics, nullptr, nullptr, &delegate, ScopedNodeId(),
                                                         Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_NO_ERROR);

    // Calling Clear() here since ASAN will have an issue if FabricTable destructor is called before CASESession's
    // destructor. We could reorder FabricTable and CaseSession, but this makes it a little more clear what we are
    // doing here.
    caseSession.Clear();
}

TEST_F(TestCASESession, SecurePairingStartTest)
{
    TemporarySessionManager sessionManager(*this);
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;
    pairing.SetGroupDataProvider(&gCommissionerGroupDataProvider);

    ExchangeContext * context = NewUnauthenticatedExchangeToBob(&pairing);

    EXPECT_NE(pairing.EstablishSession(sessionManager, nullptr, ScopedNodeId{ Node01_01, gCommissionerFabricIndex }, nullptr,
                                       nullptr, nullptr, nullptr, Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_NO_ERROR);
    ServiceEvents();

    EXPECT_NE(pairing.EstablishSession(sessionManager, &gCommissionerFabrics, ScopedNodeId{ Node01_01, gCommissionerFabricIndex },
                                       nullptr, nullptr, nullptr, nullptr, Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_NO_ERROR);
    ServiceEvents();

    EXPECT_EQ(pairing.EstablishSession(sessionManager, &gCommissionerFabrics, ScopedNodeId{ Node01_01, gCommissionerFabricIndex },
                                       context, nullptr, nullptr, &delegate, Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_NO_ERROR);
    ServiceEvents();

    auto & loopback = GetLoopback();
    // There should have been two message sent: Sigma1 and an ack.
    EXPECT_EQ(loopback.mSentMessageCount, 2u);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    loopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    CASESession pairing1;
    pairing1.SetGroupDataProvider(&gCommissionerGroupDataProvider);

    loopback.mSentMessageCount = 0;
    loopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;
    ExchangeContext * context1 = NewUnauthenticatedExchangeToBob(&pairing1);

    EXPECT_EQ(pairing1.EstablishSession(sessionManager, &gCommissionerFabrics, ScopedNodeId{ Node01_01, gCommissionerFabricIndex },
                                        context1, nullptr, nullptr, &delegate, Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_ERROR_BAD_REQUEST);
    ServiceEvents();

    loopback.mMessageSendError = CHIP_NO_ERROR;
}

void TestCASESession::SecurePairingHandshakeTestCommon(SessionManager & sessionManager, CASESession & pairingCommissioner,
                                                       TestCASESecurePairingDelegate & delegateCommissioner)
{
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegateAccessory;
    CASESession pairingAccessory;
    ReliableMessageProtocolConfig verySleepyAccessoryRmpConfig(
        System::Clock::Milliseconds32(360000), System::Clock::Milliseconds32(100000), System::Clock::Milliseconds16(300));
    ReliableMessageProtocolConfig nonSleepyCommissionerRmpConfig(
        System::Clock::Milliseconds32(5000), System::Clock::Milliseconds32(300), System::Clock::Milliseconds16(4000));

    auto & loopback            = GetLoopback();
    loopback.mSentMessageCount = 0;

    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1,
                                                                            &pairingAccessory),
              CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    pairingAccessory.SetGroupDataProvider(&gDeviceGroupDataProvider);
    EXPECT_EQ(pairingAccessory.PrepareForSessionEstablishment(sessionManager, &gDeviceFabrics, nullptr, nullptr, &delegateAccessory,
                                                              ScopedNodeId(), MakeOptional(verySleepyAccessoryRmpConfig)),
              CHIP_NO_ERROR);
    EXPECT_EQ(pairingCommissioner.EstablishSession(
                  sessionManager, &gCommissionerFabrics, ScopedNodeId{ Node01_01, gCommissionerFabricIndex }, contextCommissioner,
                  nullptr, nullptr, &delegateCommissioner, MakeOptional(nonSleepyCommissionerRmpConfig)),
              CHIP_NO_ERROR);
    ServiceEvents();

    EXPECT_EQ(loopback.mSentMessageCount, sTestCaseMessageCount);
    EXPECT_EQ(delegateAccessory.mNumPairingComplete, 1u);
    EXPECT_EQ(delegateCommissioner.mNumPairingComplete, 1u);
    EXPECT_EQ(delegateAccessory.mNumPairingErrors, 0u);
    EXPECT_EQ(delegateCommissioner.mNumPairingErrors, 0u);
    EXPECT_EQ(pairingAccessory.GetRemoteMRPConfig().mIdleRetransTimeout, System::Clock::Milliseconds32(5000));
    EXPECT_EQ(pairingAccessory.GetRemoteMRPConfig().mActiveRetransTimeout, System::Clock::Milliseconds32(300));
    EXPECT_EQ(pairingAccessory.GetRemoteMRPConfig().mActiveThresholdTime, System::Clock::Milliseconds16(4000));
    EXPECT_EQ(pairingCommissioner.GetRemoteMRPConfig().mIdleRetransTimeout, System::Clock::Milliseconds32(360000));
    EXPECT_EQ(pairingCommissioner.GetRemoteMRPConfig().mActiveRetransTimeout, System::Clock::Milliseconds32(100000));
    EXPECT_EQ(pairingCommissioner.GetRemoteMRPConfig().mActiveThresholdTime, System::Clock::Milliseconds16(300));
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    // Confirming that FabricTable sending a notification that fabric was updated doesn't affect
    // already established connections.
    //
    // This is compiled for host tests which is enough test coverage
    gCommissionerFabrics.SendUpdateFabricNotificationForTest(gCommissionerFabricIndex);
    gDeviceFabrics.SendUpdateFabricNotificationForTest(gDeviceFabricIndex);
    EXPECT_EQ(loopback.mSentMessageCount, sTestCaseMessageCount);
    EXPECT_EQ(delegateAccessory.mNumPairingComplete, 1u);
    EXPECT_EQ(delegateCommissioner.mNumPairingComplete, 1u);
    EXPECT_EQ(delegateAccessory.mNumPairingErrors, 0u);
    EXPECT_EQ(delegateCommissioner.mNumPairingErrors, 0u);
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
}

TEST_F(TestCASESession, SecurePairingHandshakeTest)
{
    TemporarySessionManager sessionManager(*this);
    TestCASESecurePairingDelegate delegateCommissioner;
    CASESession pairingCommissioner;
    pairingCommissioner.SetGroupDataProvider(&gCommissionerGroupDataProvider);
    SecurePairingHandshakeTestCommon(sessionManager, pairingCommissioner, delegateCommissioner);
}

TEST_F(TestCASESession, SecurePairingHandshakeServerTest)
{
    // TODO: Add cases for mismatching IPK config between initiator/responder

    TestCASESecurePairingDelegate delegateCommissioner;

    auto * pairingCommissioner = chip::Platform::New<CASESession>();
    pairingCommissioner->SetGroupDataProvider(&gCommissionerGroupDataProvider);

    auto & loopback            = GetLoopback();
    loopback.mSentMessageCount = 0;

    // Use the same session manager on both CASE client and server sides to validate that both
    // components may work simultaneously on a single device.
    EXPECT_EQ(gPairingServer.ListenForSessionEstablishment(&GetExchangeManager(), &GetSecureSessionManager(), &gDeviceFabrics,
                                                           nullptr, nullptr, &gDeviceGroupDataProvider),
              CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = NewUnauthenticatedExchangeToBob(pairingCommissioner);

    EXPECT_EQ(pairingCommissioner->EstablishSession(
                  GetSecureSessionManager(), &gCommissionerFabrics, ScopedNodeId{ Node01_01, gCommissionerFabricIndex },
                  contextCommissioner, nullptr, nullptr, &delegateCommissioner, Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_NO_ERROR);
    ServiceEvents();

    EXPECT_EQ(loopback.mSentMessageCount, sTestCaseMessageCount);
    EXPECT_EQ(delegateCommissioner.mNumPairingComplete, 1u);

    // Validate that secure session is created
    SessionHolder & holder = delegateCommissioner.GetSessionHolder();
    EXPECT_TRUE(bool(holder));

    EXPECT_EQ(holder->GetPeer(), (chip::ScopedNodeId{ Node01_01, gCommissionerFabricIndex }));

    auto * pairingCommissioner1 = chip::Platform::New<CASESession>();
    pairingCommissioner1->SetGroupDataProvider(&gCommissionerGroupDataProvider);
    ExchangeContext * contextCommissioner1 = NewUnauthenticatedExchangeToBob(pairingCommissioner1);

    EXPECT_EQ(pairingCommissioner1->EstablishSession(GetSecureSessionManager(), &gCommissionerFabrics,
                                                     ScopedNodeId{ Node01_01, gCommissionerFabricIndex }, contextCommissioner1,
                                                     nullptr, nullptr, &delegateCommissioner,
                                                     Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_NO_ERROR);

    ServiceEvents();

    chip::Platform::Delete(pairingCommissioner);
    chip::Platform::Delete(pairingCommissioner1);

    gPairingServer.Shutdown();
}

TEST_F(TestCASESession, ClientReceivesBusyTest)
{
    TemporarySessionManager sessionManager(*this);
    TestCASESecurePairingDelegate delegateCommissioner1, delegateCommissioner2;
    CASESession pairingCommissioner1, pairingCommissioner2;

    pairingCommissioner1.SetGroupDataProvider(&gCommissionerGroupDataProvider);
    pairingCommissioner2.SetGroupDataProvider(&gCommissionerGroupDataProvider);

    auto & loopback            = GetLoopback();
    loopback.mSentMessageCount = 0;

    EXPECT_EQ(gPairingServer.ListenForSessionEstablishment(&GetExchangeManager(), &GetSecureSessionManager(), &gDeviceFabrics,
                                                           nullptr, nullptr, &gDeviceGroupDataProvider),
              CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner1 = NewUnauthenticatedExchangeToBob(&pairingCommissioner1);
    ExchangeContext * contextCommissioner2 = NewUnauthenticatedExchangeToBob(&pairingCommissioner2);

    EXPECT_EQ(pairingCommissioner1.EstablishSession(sessionManager, &gCommissionerFabrics,
                                                    ScopedNodeId{ Node01_01, gCommissionerFabricIndex }, contextCommissioner1,
                                                    nullptr, nullptr, &delegateCommissioner1, NullOptional),
              CHIP_NO_ERROR);
    EXPECT_EQ(pairingCommissioner2.EstablishSession(sessionManager, &gCommissionerFabrics,
                                                    ScopedNodeId{ Node01_01, gCommissionerFabricIndex }, contextCommissioner2,
                                                    nullptr, nullptr, &delegateCommissioner2, NullOptional),
              CHIP_NO_ERROR);

    ServiceEvents();

    // We should have one full handshake and one Sigma1 + Busy + ack.  If that
    // ever changes (e.g. because our server starts supporting multiple parallel
    // handshakes), this test needs to be fixed so that the server is still
    // responding BUSY to the client.
    EXPECT_EQ(loopback.mSentMessageCount, sTestCaseMessageCount + 3);
    EXPECT_EQ(delegateCommissioner1.mNumPairingComplete, 1u);
    EXPECT_EQ(delegateCommissioner2.mNumPairingComplete, 0u);

    EXPECT_EQ(delegateCommissioner1.mNumPairingErrors, 0u);
    EXPECT_EQ(delegateCommissioner2.mNumPairingErrors, 1u);

    EXPECT_EQ(delegateCommissioner1.mNumBusyResponses, 0u);
    EXPECT_EQ(delegateCommissioner2.mNumBusyResponses, 1u);

    gPairingServer.Shutdown();
}

struct Sigma1Params
{
    // Purposefully not using constants like kSigmaParamRandomNumberSize that
    // the code uses, so we have a cross-check.
    static constexpr size_t initiatorRandomLen    = 32;
    static constexpr uint16_t initiatorSessionId  = 0;
    static constexpr size_t destinationIdLen      = 32;
    static constexpr size_t initiatorEphPubKeyLen = 65;
    static constexpr size_t resumptionIdLen       = 0; // Nonzero means include it.
    static constexpr size_t initiatorResumeMICLen = 0; // Nonzero means include it.

    static constexpr uint8_t initiatorRandomTag    = 1;
    static constexpr uint8_t initiatorSessionIdTag = 2;
    static constexpr uint8_t destinationIdTag      = 3;
    static constexpr uint8_t initiatorEphPubKeyTag = 4;
    static constexpr uint8_t resumptionIdTag       = 6;
    static constexpr uint8_t initiatorResumeMICTag = 7;
    static constexpr TLV::Tag NumToTag(uint8_t num) { return TLV::ContextTag(num); }

    static constexpr bool includeStructEnd = true;

    static constexpr bool expectSuccess = true;
};

TEST_F(TestCASESession, DestinationIdTest)
{
    // Validate example test vector from CASE section of spec

    const uint8_t kRootPubKeyFromSpec[Crypto::CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES] = {
        0x04, 0x4a, 0x9f, 0x42, 0xb1, 0xca, 0x48, 0x40, 0xd3, 0x72, 0x92, 0xbb, 0xc7, 0xf6, 0xa7, 0xe1, 0x1e,
        0x22, 0x20, 0x0c, 0x97, 0x6f, 0xc9, 0x00, 0xdb, 0xc9, 0x8a, 0x7a, 0x38, 0x3a, 0x64, 0x1c, 0xb8, 0x25,
        0x4a, 0x2e, 0x56, 0xd4, 0xe2, 0x95, 0xa8, 0x47, 0x94, 0x3b, 0x4e, 0x38, 0x97, 0xc4, 0xa7, 0x73, 0xe9,
        0x30, 0x27, 0x7b, 0x4d, 0x9f, 0xbe, 0xde, 0x8a, 0x05, 0x26, 0x86, 0xbf, 0xac, 0xfa
    };

    const uint8_t kIpkOperationalGroupKeyFromSpec[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = {
        0x9b, 0xc6, 0x1c, 0xd9, 0xc6, 0x2a, 0x2d, 0xf6, 0xd6, 0x4d, 0xfc, 0xaa, 0x9d, 0xc4, 0x72, 0xd4
    };

    const uint8_t kInitiatorRandomFromSpec[Sigma1Params::initiatorRandomLen] = { 0x7e, 0x17, 0x12, 0x31, 0x56, 0x8d, 0xfa, 0x17,
                                                                                 0x20, 0x6b, 0x3a, 0xcc, 0xf8, 0xfa, 0xec, 0x2f,
                                                                                 0x4d, 0x21, 0xb5, 0x80, 0x11, 0x31, 0x96, 0xf4,
                                                                                 0x7c, 0x7c, 0x4d, 0xeb, 0x81, 0x0a, 0x73, 0xdc };

    const uint8_t kExpectedDestinationIdFromSpec[Crypto::kSHA256_Hash_Length] = { 0xdc, 0x35, 0xdd, 0x5f, 0xc9, 0x13, 0x4c, 0xc5,
                                                                                  0x54, 0x45, 0x38, 0xc9, 0xc3, 0xfc, 0x42, 0x97,
                                                                                  0xc1, 0xec, 0x33, 0x70, 0xc8, 0x39, 0x13, 0x6a,
                                                                                  0x80, 0xe1, 0x07, 0x96, 0x45, 0x1d, 0x4c, 0x53 };

    const FabricId kFabricIdFromSpec = 0x2906C908D115D362;
    const NodeId kNodeIdFromSpec     = 0xCD5544AA7B13EF14;

    uint8_t destinationIdBuf[Crypto::kSHA256_Hash_Length] = { 0 };
    MutableByteSpan destinationIdSpan(destinationIdBuf);

    // Test exact example
    CHIP_ERROR err =
        GenerateCaseDestinationId(ByteSpan(kIpkOperationalGroupKeyFromSpec), ByteSpan(kInitiatorRandomFromSpec),
                                  ByteSpan(kRootPubKeyFromSpec), kFabricIdFromSpec, kNodeIdFromSpec, destinationIdSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(destinationIdSpan.size(), sizeof(destinationIdBuf));
    EXPECT_TRUE(destinationIdSpan.data_equal(ByteSpan(kExpectedDestinationIdFromSpec)));

    memset(destinationIdSpan.data(), 0, destinationIdSpan.size());

    // Test changing input: should yield different
    EXPECT_EQ(GenerateCaseDestinationId(ByteSpan(kIpkOperationalGroupKeyFromSpec), ByteSpan(kInitiatorRandomFromSpec),
                                        ByteSpan(kRootPubKeyFromSpec), kFabricIdFromSpec,
                                        kNodeIdFromSpec + 1, // <--- Change node ID
                                        destinationIdSpan),
              CHIP_NO_ERROR);
    EXPECT_EQ(destinationIdSpan.size(), sizeof(destinationIdBuf));
    EXPECT_FALSE(destinationIdSpan.data_equal(ByteSpan(kExpectedDestinationIdFromSpec)));
}

template <typename Params>
static CHIP_ERROR EncodeSigma1(MutableByteSpan & buf)
{
    using namespace TLV;

    TLVWriter writer;
    writer.Init(buf);

    TLVType containerType;
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, containerType));
    uint8_t initiatorRandom[Params::initiatorRandomLen] = { 1 };
    ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::initiatorRandomTag), ByteSpan(initiatorRandom)));

    ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::initiatorSessionIdTag), Params::initiatorSessionId));

    uint8_t destinationId[Params::destinationIdLen] = { 2 };
    ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::destinationIdTag), ByteSpan(destinationId)));

    uint8_t initiatorEphPubKey[Params::initiatorEphPubKeyLen] = { 3 };
    ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::initiatorEphPubKeyTag), ByteSpan(initiatorEphPubKey)));

    // I wish we had "if constexpr" support here, so the compiler would know
    // resumptionIdLen is nonzero inside the block....
    if (Params::resumptionIdLen != 0)
    {
        uint8_t resumptionId[Params::resumptionIdLen];

        // to fix _FORTIFY_SOURCE issue, _FORTIFY_SOURCE=2 by default on Android
        (&memset)(resumptionId, 4, Params::resumptionIdLen);
        ReturnErrorOnFailure(
            writer.Put(Params::NumToTag(Params::resumptionIdTag), ByteSpan(resumptionId, Params::resumptionIdLen)));
    }

    if (Params::initiatorResumeMICLen != 0)
    {
        uint8_t initiatorResumeMIC[Params::initiatorResumeMICLen];
        // to fix _FORTIFY_SOURCE issue, _FORTIFY_SOURCE=2 by default on Android
        (&memset)(initiatorResumeMIC, 5, Params::initiatorResumeMICLen);
        ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::initiatorResumeMICTag),
                                        ByteSpan(initiatorResumeMIC, Params::initiatorResumeMICLen)));
    }

    if (Params::includeStructEnd)
    {
        ReturnErrorOnFailure(writer.EndContainer(containerType));
    }

    buf.reduce_size(writer.GetLengthWritten());
    return CHIP_NO_ERROR;
}

// A macro, so we can tell which test failed based on line number.
#define TestSigma1Parsing(mem, bufferSize, params)                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        MutableByteSpan buf(mem.Get(), bufferSize);                                                                                \
        EXPECT_EQ(EncodeSigma1<params>(buf), CHIP_NO_ERROR);                                                                       \
                                                                                                                                   \
        TLV::ContiguousBufferTLVReader reader;                                                                                     \
        reader.Init(buf);                                                                                                          \
                                                                                                                                   \
        ByteSpan initiatorRandom;                                                                                                  \
        uint16_t initiatorSessionId;                                                                                               \
        ByteSpan destinationId;                                                                                                    \
        ByteSpan initiatorEphPubKey;                                                                                               \
        bool resumptionRequested;                                                                                                  \
        ByteSpan resumptionId;                                                                                                     \
        ByteSpan initiatorResumeMIC;                                                                                               \
        CASESession session;                                                                                                       \
        EXPECT_EQ(session.ParseSigma1(reader, initiatorRandom, initiatorSessionId, destinationId, initiatorEphPubKey,              \
                                      resumptionRequested, resumptionId, initiatorResumeMIC) == CHIP_NO_ERROR,                     \
                  params::expectSuccess);                                                                                          \
        if (params::expectSuccess)                                                                                                 \
        {                                                                                                                          \
            EXPECT_EQ(resumptionRequested, params::resumptionIdLen != 0 && params::initiatorResumeMICLen != 0);                    \
            /* Add other verification tests here as desired */                                                                     \
        }                                                                                                                          \
    } while (0)

struct BadSigma1ParamsBase : public Sigma1Params
{
    static constexpr bool expectSuccess = false;
};

struct Sigma1NoStructEnd : public BadSigma1ParamsBase
{
    static constexpr bool includeStructEnd = false;
};

struct Sigma1WrongTags : public BadSigma1ParamsBase
{
    static constexpr TLV::Tag NumToTag(uint8_t num) { return TLV::ProfileTag(0, num); }
};

struct Sigma1TooLongRandom : public BadSigma1ParamsBase
{
    static constexpr size_t initiatorRandomLen = 33;
};

struct Sigma1TooShortRandom : public BadSigma1ParamsBase
{
    static constexpr size_t initiatorRandomLen = 31;
};

struct Sigma1TooLongDest : public BadSigma1ParamsBase
{
    static constexpr size_t destinationIdLen = 33;
};

struct Sigma1TooShortDest : public BadSigma1ParamsBase
{
    static constexpr size_t destinationIdLen = 31;
};

struct Sigma1TooLongPubkey : public BadSigma1ParamsBase
{
    static constexpr size_t initiatorEphPubKeyLen = 66;
};

struct Sigma1TooShortPubkey : public BadSigma1ParamsBase
{
    static constexpr size_t initiatorEphPubKeyLen = 64;
};

struct Sigma1WithResumption : public Sigma1Params
{
    static constexpr size_t resumptionIdLen       = 16;
    static constexpr size_t initiatorResumeMICLen = 16;
};

struct Sigma1TooLongResumptionId : public Sigma1WithResumption
{
    static constexpr size_t resumptionIdLen = 17;
    static constexpr bool expectSuccess     = false;
};

struct Sigma1TooShortResumptionId : public BadSigma1ParamsBase
{
    static constexpr size_t resumptionIdLen = 15;
    static constexpr bool expectSuccess     = false;
};

struct Sigma1TooLongResumeMIC : public Sigma1WithResumption
{
    static constexpr size_t resumptionIdLen = 17;
    static constexpr bool expectSuccess     = false;
};

struct Sigma1TooShortResumeMIC : public Sigma1WithResumption
{
    static constexpr size_t initiatorResumeMICLen = 15;
    static constexpr bool expectSuccess           = false;
};

struct Sigma1SessionIdMax : public Sigma1Params
{
    static constexpr uint32_t initiatorSessionId = UINT16_MAX;
};

struct Sigma1SessionIdTooBig : public BadSigma1ParamsBase
{
    static constexpr uint32_t initiatorSessionId = UINT16_MAX + 1;
};

TEST_F(TestCASESession, Sigma1ParsingTest)
{
    // 1280 bytes must be enough by definition.
    constexpr size_t bufferSize = 1280;
    chip::Platform::ScopedMemoryBuffer<uint8_t> mem;
    EXPECT_TRUE(mem.Calloc(bufferSize));

    TestSigma1Parsing(mem, bufferSize, Sigma1Params);
    TestSigma1Parsing(mem, bufferSize, Sigma1NoStructEnd);
    TestSigma1Parsing(mem, bufferSize, Sigma1WrongTags);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooLongRandom);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooShortRandom);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooLongDest);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooShortDest);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooLongPubkey);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooShortPubkey);
    TestSigma1Parsing(mem, bufferSize, Sigma1WithResumption);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooLongResumptionId);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooShortResumptionId);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooLongResumeMIC);
    TestSigma1Parsing(mem, bufferSize, Sigma1TooShortResumeMIC);
    TestSigma1Parsing(mem, bufferSize, Sigma1SessionIdMax);
    TestSigma1Parsing(mem, bufferSize, Sigma1SessionIdTooBig);
}

struct SessionResumptionTestStorage : SessionResumptionStorage
{
    SessionResumptionTestStorage(CHIP_ERROR findMethodReturnCode, ScopedNodeId peerNodeId, ResumptionIdStorage * resumptionId,
                                 Crypto::P256ECDHDerivedSecret * sharedSecret) :
        mFindMethodReturnCode(findMethodReturnCode),
        mPeerNodeId(peerNodeId), mResumptionId(resumptionId), mSharedSecret(sharedSecret)
    {}
    SessionResumptionTestStorage(CHIP_ERROR findMethodReturnCode) : mFindMethodReturnCode(findMethodReturnCode) {}
    CHIP_ERROR FindByScopedNodeId(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                  Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override
    {
        if (mResumptionId != nullptr)
        {
            memcpy(resumptionId.data(), mResumptionId->data(), mResumptionId->size());
        }
        if (mSharedSecret != nullptr)
        {
            memcpy(sharedSecret.Bytes(), mSharedSecret->Bytes(), mSharedSecret->Length());
            sharedSecret.SetLength(mSharedSecret->Length());
        }
        peerCATs = CATValues{};
        return mFindMethodReturnCode;
    }
    CHIP_ERROR FindByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node,
                                  Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override
    {
        node = mPeerNodeId;
        if (mSharedSecret != nullptr)
        {
            memcpy(sharedSecret.Bytes(), mSharedSecret->Bytes(), mSharedSecret->Length());
            sharedSecret.SetLength(mSharedSecret->Length());
        }
        peerCATs = CATValues{};
        return mFindMethodReturnCode;
    }
    CHIP_ERROR Save(const ScopedNodeId & node, ConstResumptionIdView resumptionId,
                    const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR DeleteAll(const FabricIndex fabricIndex) override { return CHIP_NO_ERROR; }
    CHIP_ERROR mFindMethodReturnCode;
    ScopedNodeId mPeerNodeId;
    ResumptionIdStorage * mResumptionId           = nullptr;
    Crypto::P256ECDHDerivedSecret * mSharedSecret = nullptr;
};

TEST_F(TestCASESession, SessionResumptionStorage)
{
    // Test the SessionResumptionStorage external interface.
    //
    // Our build should accept any storage delegate injected that implements
    // this.  And if our delegate provides usable session resumption
    // information, session resumption should succeed.  In the case that the
    // delegate cannot provide the information needed for session resumption, or
    // if the peers have mismatched session resumption information, we should
    // fall back to CASE.

    TestCASESecurePairingDelegate delegateCommissioner;
    chip::SessionResumptionStorage::ResumptionIdStorage resumptionIdA;
    chip::SessionResumptionStorage::ResumptionIdStorage resumptionIdB;
    chip::Crypto::P256ECDHDerivedSecret sharedSecretA;
    chip::Crypto::P256ECDHDerivedSecret sharedSecretB;

    // Create our fabric-scoped node IDs.
    const FabricInfo * fabricInfo = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    ASSERT_NE(fabricInfo, nullptr);
    ScopedNodeId initiator = fabricInfo->GetScopedNodeIdForNode(Node01_02);
    ScopedNodeId responder = fabricInfo->GetScopedNodeIdForNode(Node01_01);

    // Generate a resumption IDs.
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(resumptionIdA.data(), resumptionIdA.size()), CHIP_NO_ERROR);
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(resumptionIdB.data(), resumptionIdB.size()), CHIP_NO_ERROR);

    // Generate a shared secrets.
    sharedSecretA.SetLength(sharedSecretA.Capacity());
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(sharedSecretA.Bytes(), sharedSecretA.Length()), CHIP_NO_ERROR);
    sharedSecretB.SetLength(sharedSecretB.Capacity());
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(sharedSecretB.Bytes(), sharedSecretB.Length()), CHIP_NO_ERROR);

    struct
    {
        SessionResumptionTestStorage initiatorStorage;
        SessionResumptionTestStorage responderStorage;
        uint32_t expectedSentMessageCount;
    } testVectors[] = {
        // Both peers have a matching session resumption record.
        // This should succeed.
        {
            .initiatorStorage = SessionResumptionTestStorage(CHIP_NO_ERROR, responder, &resumptionIdA, &sharedSecretA),
            .responderStorage = SessionResumptionTestStorage(CHIP_NO_ERROR, initiator, &resumptionIdA, &sharedSecretA),
            .expectedSentMessageCount =
                sTestCaseResumptionMessageCount, // we expect this number of sent messages with successful session resumption
        },
        // Peers have mismatched session resumption records.
        // This should succeed with fall back to CASE.
        {
            .initiatorStorage         = SessionResumptionTestStorage(CHIP_NO_ERROR, responder, &resumptionIdA, &sharedSecretA),
            .responderStorage         = SessionResumptionTestStorage(CHIP_ERROR_KEY_NOT_FOUND),
            .expectedSentMessageCount = sTestCaseMessageCount, // we expect this number of sent message when we fall back to CASE
        },
        // Peers both have record of the same resumption ID, but a different shared secret.
        // This should succeed with fall back to CASE.
        {
            .initiatorStorage         = SessionResumptionTestStorage(CHIP_NO_ERROR, responder, &resumptionIdA, &sharedSecretA),
            .responderStorage         = SessionResumptionTestStorage(CHIP_NO_ERROR, initiator, &resumptionIdA, &sharedSecretB),
            .expectedSentMessageCount = sTestCaseMessageCount, // we expect this number of sent message when we fall back to CASE
        },
        // Neither peer has a session resumption record.
        // This should succeed - no attempt at session resumption will be made.
        {
            .initiatorStorage = SessionResumptionTestStorage(CHIP_ERROR_KEY_NOT_FOUND),
            .responderStorage = SessionResumptionTestStorage(CHIP_ERROR_KEY_NOT_FOUND),
            .expectedSentMessageCount =
                sTestCaseMessageCount, // we expect this number of sent messages if we do not attempt session resumption
        },
    };

    auto & loopback = GetLoopback();
    for (size_t i = 0; i < sizeof(testVectors) / sizeof(testVectors[0]); ++i)
    {
        auto * pairingCommissioner = chip::Platform::New<CASESession>();
        pairingCommissioner->SetGroupDataProvider(&gCommissionerGroupDataProvider);
        loopback.mSentMessageCount = 0;
        EXPECT_EQ(gPairingServer.ListenForSessionEstablishment(&GetExchangeManager(), &GetSecureSessionManager(), &gDeviceFabrics,
                                                               &testVectors[i].responderStorage, nullptr,
                                                               &gDeviceGroupDataProvider),
                  CHIP_NO_ERROR);
        ExchangeContext * contextCommissioner = NewUnauthenticatedExchangeToBob(pairingCommissioner);
        auto establishmentReturnVal           = pairingCommissioner->EstablishSession(
            GetSecureSessionManager(), &gCommissionerFabrics, ScopedNodeId{ Node01_01, gCommissionerFabricIndex },
            contextCommissioner, &testVectors[i].initiatorStorage, nullptr, &delegateCommissioner,
            Optional<ReliableMessageProtocolConfig>::Missing());
        ServiceEvents();
        EXPECT_EQ(establishmentReturnVal, CHIP_NO_ERROR);
        EXPECT_EQ(loopback.mSentMessageCount, testVectors[i].expectedSentMessageCount);
        EXPECT_EQ(delegateCommissioner.mNumPairingComplete, i + 1);
        SessionHolder & holder = delegateCommissioner.GetSessionHolder();
        EXPECT_TRUE(bool(holder));
        EXPECT_EQ(holder->GetPeer(), fabricInfo->GetScopedNodeIdForNode(Node01_01));
        chip::Platform::Delete(pairingCommissioner);
        gPairingServer.Shutdown();
    }
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
TEST_F_FROM_FIXTURE(TestCASESession, SimulateUpdateNOCInvalidatePendingEstablishment)
{
    TemporarySessionManager sessionManager(*this);
    TestCASESecurePairingDelegate delegateCommissioner;
    CASESession pairingCommissioner;
    pairingCommissioner.SetGroupDataProvider(&gCommissionerGroupDataProvider);

    TestCASESecurePairingDelegate delegateAccessory;
    CASESession pairingAccessory;

    auto & loopback            = GetLoopback();
    loopback.mSentMessageCount = 0;

    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1,
                                                                            &pairingAccessory),
              CHIP_NO_ERROR);

    // In order for all the test iterations below, we need to stop the CASE sigma handshake in the middle such
    // that the CASE session is in the process of being established.
    pairingCommissioner.SetStopSigmaHandshakeAt(MakeOptional(CASESession::State::kSentSigma1));

    ExchangeContext * contextCommissioner = NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    pairingAccessory.SetGroupDataProvider(&gDeviceGroupDataProvider);
    EXPECT_EQ(pairingAccessory.PrepareForSessionEstablishment(sessionManager, &gDeviceFabrics, nullptr, nullptr, &delegateAccessory,
                                                              ScopedNodeId(), Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_NO_ERROR);

    gDeviceFabrics.SendUpdateFabricNotificationForTest(gDeviceFabricIndex);
    ServiceEvents();
    EXPECT_EQ(delegateAccessory.mNumPairingErrors, 0u);

    EXPECT_EQ(pairingCommissioner.EstablishSession(
                  sessionManager, &gCommissionerFabrics, ScopedNodeId{ Node01_01, gCommissionerFabricIndex }, contextCommissioner,
                  nullptr, nullptr, &delegateCommissioner, Optional<ReliableMessageProtocolConfig>::Missing()),
              CHIP_NO_ERROR);
    ServiceEvents();

    // At this point the CASESession is in the process of establishing. Confirm that there are no errors and there are session
    // has not been established.
    EXPECT_EQ(delegateAccessory.mNumPairingComplete, 0u);
    EXPECT_EQ(delegateCommissioner.mNumPairingComplete, 0u);
    EXPECT_EQ(delegateAccessory.mNumPairingErrors, 0u);
    EXPECT_EQ(delegateCommissioner.mNumPairingErrors, 0u);

    // Simulating an update to the Fabric NOC for gCommissionerFabrics fabric table.
    // Confirm that CASESession on commisioner side has reported an error.
    gCommissionerFabrics.SendUpdateFabricNotificationForTest(gCommissionerFabricIndex);
    ServiceEvents();
    EXPECT_EQ(delegateAccessory.mNumPairingErrors, 0u);
    EXPECT_EQ(delegateCommissioner.mNumPairingErrors, 1u);

    // Simulating an update to the Fabric NOC for gDeviceFabrics fabric table.
    // Confirm that CASESession on accessory side has reported an error.
    gDeviceFabrics.SendUpdateFabricNotificationForTest(gDeviceFabricIndex);
    ServiceEvents();
    EXPECT_EQ(delegateAccessory.mNumPairingErrors, 1u);
    EXPECT_EQ(delegateCommissioner.mNumPairingErrors, 1u);

    // Sanity check that pairing did not complete.
    EXPECT_EQ(delegateAccessory.mNumPairingComplete, 0u);
    EXPECT_EQ(delegateCommissioner.mNumPairingComplete, 0u);
}
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

class ExpectErrorExchangeDelegate : public ExchangeDelegate
{
public:
    ExpectErrorExchangeDelegate(uint16_t expectedProtocolCode) : mExpectedProtocolCode(expectedProtocolCode) {}

private:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buf) override
    {
        using namespace SecureChannel;

        EXPECT_TRUE(payloadHeader.HasMessageType(MsgType::StatusReport));

        SecureChannel::StatusReport statusReport;
        EXPECT_EQ(statusReport.Parse(std::move(buf)), CHIP_NO_ERROR);

        EXPECT_EQ(statusReport.GetProtocolId(), SecureChannel::Id);
        EXPECT_EQ(statusReport.GetGeneralCode(), GeneralStatusCode::kFailure);
        EXPECT_EQ(statusReport.GetProtocolCode(), mExpectedProtocolCode);
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    Messaging::ExchangeMessageDispatch & GetMessageDispatch() override { return SessionEstablishmentExchangeDispatch::Instance(); }

    uint16_t mExpectedProtocolCode;
};

TEST_F(TestCASESession, Sigma1BadDestinationIdTest)
{
    using SecureChannel::MsgType;

    SessionManager & sessionManager = GetSecureSessionManager();

    constexpr size_t bufferSize     = 600;
    System::PacketBufferHandle data = chip::System::PacketBufferHandle::New(bufferSize);
    ASSERT_FALSE(data.IsNull());

    MutableByteSpan buf(data->Start(), data->AvailableDataLength());
    // This uses a bogus destination id that is not going to match anything in practice.
    EXPECT_EQ(EncodeSigma1<Sigma1Params>(buf), CHIP_NO_ERROR);
    data->SetDataLength(static_cast<uint16_t>(buf.size()));

    Optional<SessionHandle> session = sessionManager.CreateUnauthenticatedSession(GetAliceAddress(), GetDefaultMRPConfig());
    EXPECT_TRUE(session.HasValue());

    TestCASESecurePairingDelegate caseDelegate;
    CASESession caseSession;
    caseSession.SetGroupDataProvider(&gDeviceGroupDataProvider);
    EXPECT_EQ(caseSession.PrepareForSessionEstablishment(sessionManager, &gDeviceFabrics, nullptr, nullptr, &caseDelegate,
                                                         ScopedNodeId(), NullOptional),
              CHIP_NO_ERROR);

    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(MsgType::CASE_Sigma1, &caseSession), CHIP_NO_ERROR);

    ExpectErrorExchangeDelegate delegate(SecureChannel::kProtocolCodeNoSharedRoot);
    ExchangeContext * exchange = GetExchangeManager().NewContext(session.Value(), &delegate);
    ASSERT_NE(exchange, nullptr);

    EXPECT_EQ(exchange->SendMessage(MsgType::CASE_Sigma1, std::move(data), SendMessageFlags::kExpectResponse), CHIP_NO_ERROR);

    ServiceEvents();

    EXPECT_EQ(caseDelegate.mNumPairingErrors, 1u);
    EXPECT_EQ(caseDelegate.mNumPairingComplete, 0u);

    GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(MsgType::CASE_Sigma1);
    caseSession.Clear();
}

} // namespace chip
