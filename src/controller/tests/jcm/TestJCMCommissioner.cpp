/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/Context.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/test-interaction-model-api.h>
#include <controller/CommissioningDelegate.h>
#include <controller/jcm/AutoCommissioner.h>
#include <controller/jcm/DeviceCommissioner.h>
#include <controller/tests/data_model/DataModelFixtures.h>
#include <credentials/CHIPCert.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <pw_unit_test/framework.h>
#include <transport/SecureSession.h>
#include <transport/SecureSessionTable.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Controller;
using namespace chip::Crypto;
using namespace chip::Credentials;
using namespace chip::Messaging;
using namespace chip::Platform;
using namespace chip::Testing;
using namespace chip::TestCerts;
using namespace chip::Transport;

namespace chip {
namespace Controller {
namespace JCM {
using namespace ::chip::Credentials::JCM;

// Mock function for linking
void InitDataModelHandler() {}

class MockTrustVerificationDelegate : public TrustVerificationDelegate
{
public:
    void OnProgressUpdate(TrustVerificationStateMachine & stateMachine, TrustVerificationStage stage, TrustVerificationInfo & info,
                          TrustVerificationError error) override
    {
        mProgressUpdates++;
        mLastStage              = stage;
        mLastError              = error;
        mRemoteAdminTrustedRoot = info.adminRCAC.Span();
    }

    void OnAskUserForConsent(TrustVerificationStateMachine & stateMachine, TrustVerificationInfo & info) override
    {
        mAskedForConsent = true;
        mLastVendorId    = info.adminVendorId;
        stateMachine.ContinueAfterUserConsent(mShouldConsent);
    }

    CHIP_ERROR OnLookupOperationalTrustAnchor(VendorId vendorID, CertificateKeyId & subjectKeyId,
                                              ByteSpan & globallyTrustedRootSpan)
    {
        mLookedUpOperationalTrustAnchor = true;
        globallyTrustedRootSpan         = mRemoteAdminTrustedRoot;

        return CHIP_NO_ERROR;
    }

    int mProgressUpdates                 = 0;
    TrustVerificationStage mLastStage    = TrustVerificationStage::kIdle;
    TrustVerificationError mLastError    = TrustVerificationError::kSuccess;
    bool mAskedForConsent                = false;
    bool mLookedUpOperationalTrustAnchor = false;
    bool mShouldConsent                  = true;
    CHIP_ERROR mVerifyVendorIdError      = CHIP_NO_ERROR;
    VendorId mLastVendorId               = VendorId::Common;
    ByteSpan mRemoteAdminTrustedRoot;
};

class MockClusterStateCache : public ClusterStateCache
{
public:
    MockClusterStateCache() : ClusterStateCache(mClusterStateCacheCallback) {}

    class MockClusterStateCacheCallback : public ClusterStateCache::Callback
    {
        void OnDone(ReadClient *) override {}
        void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override {}
    };

    CHIP_ERROR SetUp(const FabricTable & fabricTable, const FabricIndex fabricIndex, const NodeId nodeId)
    {
        const FabricInfo * fabricInfo = fabricTable.FindFabricWithIndex(fabricIndex);

        // Setup JF Administrator cluster attributes
        ConcreteAttributePath adminFabricIndexPath(1, JointFabricAdministrator::Id,
                                                   JointFabricAdministrator::Attributes::AdministratorFabricIndex::Id);
        ReturnErrorOnFailure(SetAttribute(adminFabricIndexPath, static_cast<FabricIndex>(fabricIndex)));

        // Setup trusted root certificates
        uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
        MutableByteSpan rcacSpan{ rcacBuf };
        ReturnErrorOnFailure(fabricTable.FetchRootCert(fabricIndex, rcacSpan));
        chip::ByteSpan rcacCertsData[] = { rcacSpan };
        DataModel::List<chip::ByteSpan> rcacCerts;
        rcacCerts = rcacCertsData;
        ConcreteAttributePath trustedRootsPath(0, OperationalCredentials::Id,
                                               OperationalCredentials::Attributes::TrustedRootCertificates::Id);
        ReturnErrorOnFailure(SetAttribute(trustedRootsPath, rcacCerts));

        // Setup Operational Credentials cluster fabrics list attribute
        OperationalCredentials::Structs::FabricDescriptorStruct::Type fabricDescriptor;
        fabricDescriptor.fabricIndex = static_cast<chip::FabricIndex>(fabricIndex);
        fabricDescriptor.vendorID    = static_cast<chip::VendorId>(fabricInfo->GetVendorId()); // Example vendor ID
        fabricDescriptor.fabricID    = static_cast<chip::FabricId>(fabricInfo->GetFabricId());
        fabricDescriptor.nodeID      = static_cast<chip::NodeId>(nodeId);

        Credentials::P256PublicKeySpan trustedCAPublicKeySpan;
        ReturnErrorOnFailure(Credentials::ExtractPublicKeyFromChipCert(rcacSpan, trustedCAPublicKeySpan));

        Crypto::P256PublicKey trustedCAPublicKey{ trustedCAPublicKeySpan };
        fabricDescriptor.rootPublicKey = ByteSpan{ trustedCAPublicKey.ConstBytes(), trustedCAPublicKey.Length() };

        OperationalCredentials::Structs::FabricDescriptorStruct::Type fabricListData[1] = { std::move(fabricDescriptor) };
        DataModel::List<const OperationalCredentials::Structs::FabricDescriptorStruct::Type> fabricsList;
        fabricsList = fabricListData;
        ConcreteAttributePath fabricsPath(0, OperationalCredentials::Id, OperationalCredentials::Attributes::Fabrics::Id);
        ReturnErrorOnFailure(SetAttributeForWrite(fabricsPath, fabricsList));

        // Setup NOCs list attribute
        OperationalCredentials::Structs::NOCStruct::Type nocStruct;
        nocStruct.fabricIndex = fabricDescriptor.fabricIndex;

        uint8_t icacBuf[Credentials::kMaxCHIPCertLength];
        MutableByteSpan icacSpan{ icacBuf };
        ReturnErrorOnFailure(fabricTable.FetchICACert(fabricIndex, icacSpan));
        nocStruct.icac = icacSpan;

        uint8_t nocBuf[Credentials::kMaxCHIPCertLength];
        MutableByteSpan nocSpan{ nocBuf };
        ReturnErrorOnFailure(fabricTable.FetchNOCCert(fabricIndex, nocSpan));
        nocStruct.noc = nocSpan;

        OperationalCredentials::Structs::NOCStruct::Type nocListData[1] = { nocStruct };
        DataModel::List<OperationalCredentials::Structs::NOCStruct::Type> nocsList;
        nocsList = nocListData;

        ConcreteAttributePath nocsPath(0, OperationalCredentials::Id, OperationalCredentials::Attributes::NOCs::Id);
        ReturnErrorOnFailure(SetAttributeForWrite(nocsPath, nocsList));

        return CHIP_NO_ERROR;
    }

    void TearDown() { ClearEventCache(); }

    template <typename AttrType>
    CHIP_ERROR SetAttribute(const ConcreteAttributePath & path, const AttrType data)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> handle;
        handle.Calloc(3000);
        TLV::ScopedBufferTLVWriter writer(std::move(handle), 3000);
        ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), data));
        uint32_t writtenLength = writer.GetLengthWritten();
        ReturnErrorOnFailure(writer.Finalize(handle));

        TLV::ScopedBufferTLVReader reader;
        StatusIB aStatus;
        reader.Init(std::move(handle), writtenLength);
        ReturnErrorOnFailure(reader.Next());
        ReadClient::Callback & callback = GetBufferedCallback();
        callback.OnAttributeData(path, &reader, aStatus);

        return CHIP_NO_ERROR;
    }

    template <typename AttrType>
    CHIP_ERROR SetAttributeForWrite(const ConcreteAttributePath & path, const AttrType data)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> handle;
        handle.Calloc(3000);
        TLV::ScopedBufferTLVWriter writer(std::move(handle), 3000);
        ReturnErrorOnFailure(DataModel::EncodeForRead(writer, TLV::AnonymousTag(), static_cast<chip::FabricIndex>(1), data));
        uint32_t writtenLength = writer.GetLengthWritten();
        ReturnErrorOnFailure(writer.Finalize(handle));

        TLV::ScopedBufferTLVReader reader;
        StatusIB aStatus;
        reader.Init(std::move(handle), writtenLength);
        ReturnErrorOnFailure(reader.Next());
        ReadClient::Callback & callback = GetBufferedCallback();
        callback.OnAttributeData(path, &reader, aStatus);

        return CHIP_NO_ERROR;
    }

private:
    MockClusterStateCacheCallback mClusterStateCacheCallback;
};

class MockDeviceProxy : public DeviceProxy, public SessionDelegate
{
public:
    MockDeviceProxy(Messaging::ExchangeManager * exchangeManager, const SessionHandle & session, NodeId nodeId) :
        mExchangeManager(exchangeManager), mSecureSession(*this), mRemoteNodeId(nodeId)
    {
        mSecureSession.Grab(session);
    }
    void Disconnect() override {}
    NodeId GetDeviceId() const override { return mRemoteNodeId; }
    Messaging::ExchangeManager * GetExchangeManager() const override { return mExchangeManager; }
    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession.Get(); }
    bool IsSecureConnected() const override { return true; }
    void OnSessionReleased() override {}

private:
    Messaging::ExchangeManager * mExchangeManager;
    SessionHolderWithDelegate mSecureSession;
    const NodeId mRemoteNodeId;
};

class TestableDeviceCommissioner : public DeviceCommissioner
{
public:
    void OnTrustVerificationComplete(TrustVerificationError result) override
    {
        mResult = result;

        ChipLogProgress(Controller, "TestableDeviceCommissioner::OnTrustVerificationComplete called with result: %hu",
                        static_cast<uint16_t>(result));
    }

    TrustVerificationError mResult;
};

class TestVendorIDVerificationDataModel : public CodegenDataModelProvider
{
public:
    TestVendorIDVerificationDataModel(MessagingContext * messagingContext) : mMessagingContext(messagingContext) {}

    static TestVendorIDVerificationDataModel & Instance(MessagingContext * messagingContext)
    {
        static TestVendorIDVerificationDataModel instance(messagingContext);
        return instance;
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & aRequest,
                                                               chip::TLV::TLVReader & aReader, CommandHandler * aHandler) override
    {
        if (aRequest.path.mClusterId != Clusters::OperationalCredentials::Id ||
            aRequest.path.mCommandId !=
                Clusters::OperationalCredentials::Commands::SignVIDVerificationRequest::Type::GetCommandId())
        {
            aHandler->AddStatus(aRequest.path, Protocols::InteractionModel::Status::UnsupportedCommand, "Invalid cluster/command");
            return std::nullopt; // handler status is set by the dispatch
        }

        Clusters::OperationalCredentials::Commands::SignVIDVerificationRequest::DecodableType dataRequest;

        if (DataModel::Decode(aReader, dataRequest) != CHIP_NO_ERROR)
        {
            aHandler->AddStatus(aRequest.path, Protocols::InteractionModel::Status::Failure, "Unable to decode the request");
            return std::nullopt; // handler status is set by the dispatch
        }

        ChipLogDetail(Controller, "Received Cluster Command: Endpoint=%x Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                      aRequest.path.mEndpointId, ChipLogValueMEI(aRequest.path.mClusterId),
                      ChipLogValueMEI(aRequest.path.mCommandId));

        // Get the fabric table.
        FabricTable & fabricTable = mMessagingContext->GetFabricTable();

        // Sign the VID verification request.
        SessionHandle sessionHandle       = mMessagingContext->GetJFSessionAToB();
        ByteSpan attestationChallengeSpan = sessionHandle->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();
        FabricTable::SignVIDVerificationResponseData responseData;

        CHIP_ERROR err = fabricTable.SignVIDVerificationRequest(dataRequest.fabricIndex, dataRequest.clientChallenge,
                                                                attestationChallengeSpan, responseData);
        if (err != CHIP_NO_ERROR)
        {
            aHandler->AddStatus(aRequest.path, Protocols::InteractionModel::Status::Failure);
            return std::nullopt; // handler status is set by the dispatch
        }

        // Return the response
        Clusters::OperationalCredentials::Commands::SignVIDVerificationResponse::Type response;
        response.fabricIndex          = responseData.fabricIndex;
        response.fabricBindingVersion = responseData.fabricBindingVersion;
        response.signature            = responseData.signature.Span();
        aHandler->AddResponse(aRequest.path, response);
        aHandler->AddStatus(aRequest.path, Protocols::InteractionModel::Status::Success);

        return std::nullopt; // handler status is set by the dispatch
    }

private:
    MessagingContext * mMessagingContext;
};

const MockNodeConfig & TestMockNodeConfig()
{
    using namespace chip::app;
    using namespace chip::app::Clusters::Globals::Attributes;

    // clang-format off
    static const MockNodeConfig config({
        MockEndpointConfig(kRootEndpointId, {
            MockClusterConfig(Clusters::OperationalCredentials::Id, {
                ClusterRevision::Id, FeatureMap::Id,
            },
            {}, // events
            {
               Clusters::OperationalCredentials::Commands::SignVIDVerificationRequest::Id,
            }, // accepted commands
            {} // generated commands
          ),
        }),
    });
    // clang-format on
    return config;
}

class TestCommissioner : public AppContext
{
public:
    TestCommissioner() { mInfo.attributes = &mClusterStateCache; }

    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);

        AppContext::SetUpTestSuite();
    }

    // Performs shared teardown for all tests in the test suite.  Run once for the whole suite.
    static void TearDownTestSuite()
    {
        AppContext::TearDownTestSuite();

        Platform::MemoryShutdown();
    }

    void TestTrustVerificationStageFinishedProgressesThroughStages();
    void TestTrustVerificationStageFinishedHandlesUserConsent();
    void TestTrustVerificationStageFinishedHandlesError();
    void TestParseExtraCommissioningInfo();
    void TestParseAdminFabricIndexAndEndpointId();
    void TestParseOperationalCredentials();
    void TestParseTrustedRoot();

protected:
    void SetUp() override
    {
        SetupForJFTests();
        AppContext::SetUp();
        mOldProvider =
            InteractionModelEngine::GetInstance()->SetDataModelProvider(&TestVendorIDVerificationDataModel::Instance(this));
        SetMockNodeConfig(TestMockNodeConfig());

        mDeviceCommissioner = new TestableDeviceCommissioner();
        mDeviceCommissioner->RegisterTrustVerificationDelegate(&mTrustVerificationDelegate);
        SessionHandle session             = GetJFSessionAToB();
        NodeId jfbNodeId                  = GetJFBFabric()->GetNodeId();
        mDeviceProxy                      = new MockDeviceProxy(&GetExchangeManager(), session, jfbNodeId);
        mDeviceCommissioner->mDeviceProxy = mDeviceProxy;

#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
        mCommissioningParams.SetUseJCM(true);
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
        mAutoCommissioner.SetCommissioningParameters(mCommissioningParams);

        CHIP_ERROR err = mClusterStateCache.SetUp(GetFabricTable(), GetJFBFabricIndex(), jfbNodeId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "MockClusterStateCache::SetUp failed with error: %s", ErrorStr(err));
        }
    }

    void TearDown() override
    {
        ExpireJFSessionAToB();

        chip::Testing::ResetMockNodeConfig();
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        AppContext::TearDown();

        mClusterStateCache.TearDown();

        delete mDeviceCommissioner;
        mDeviceCommissioner = nullptr;

        delete mDeviceProxy;
        mDeviceProxy = nullptr;
    }

protected:
    chip::app::DataModel::Provider * mOldProvider = nullptr;

private:
    AutoCommissioner mAutoCommissioner;
    DeviceCommissioner * mDeviceCommissioner;
    MockTrustVerificationDelegate mTrustVerificationDelegate;
    MockClusterStateCache mClusterStateCache;
    ReadCommissioningInfo mInfo;
    CommissioningParameters mCommissioningParams;
    DeviceProxy * mDeviceProxy = nullptr;
};

TEST_F_FROM_FIXTURE(TestCommissioner, TestTrustVerificationStageFinishedProgressesThroughStages)
{
    // Simulate user consenting
    mTrustVerificationDelegate.mShouldConsent = true;
    // Set up the mock ReadCommissioningInfo
    mDeviceCommissioner->ParseExtraCommissioningInfo(mInfo, mCommissioningParams);

    TrustVerificationStage stage = TrustVerificationStage::kIdle;
    TrustVerificationError error = TrustVerificationError::kSuccess;

    // Start at Started, advance through all stages
    // Advance to kAskUserForConsent (should trigger consent)
    mDeviceCommissioner->TrustVerificationStageFinished(stage, error);

    // Wait for JCM to complete
    DrainAndServiceIO();

    EXPECT_EQ(mTrustVerificationDelegate.mProgressUpdates, 5); // Progress not incremented for consent
    EXPECT_EQ(mTrustVerificationDelegate.mLastStage, TrustVerificationStage::kComplete);
    EXPECT_EQ(mTrustVerificationDelegate.mLastError, TrustVerificationError::kSuccess);
    EXPECT_TRUE(mTrustVerificationDelegate.mAskedForConsent);
    EXPECT_TRUE(mTrustVerificationDelegate.mLookedUpOperationalTrustAnchor);
    EXPECT_EQ(mTrustVerificationDelegate.mLastVendorId, chip::VendorId(65521));
}

TEST_F_FROM_FIXTURE(TestCommissioner, TestTrustVerificationStageFinishedHandlesUserConsent)
{
    // Simulate user rejecting consent
    mTrustVerificationDelegate.mShouldConsent = false;

    // Simulate reaching consent stage
    mDeviceCommissioner->mInfo.adminFabricIndex = 1;
    mDeviceCommissioner->mInfo.adminEndpointId  = 1;

    TrustVerificationStage stage = TrustVerificationStage::kPerformingVendorIDVerification;
    TrustVerificationError error = TrustVerificationError::kSuccess;

    mDeviceCommissioner->TrustVerificationStageFinished(stage, error);
    EXPECT_TRUE(mTrustVerificationDelegate.mAskedForConsent);
    EXPECT_EQ(mTrustVerificationDelegate.mProgressUpdates, 2); // Only OnProgressUpdate called for error
    EXPECT_EQ(mTrustVerificationDelegate.mLastStage, TrustVerificationStage::kAskingUserForConsent);
    EXPECT_EQ(mTrustVerificationDelegate.mLastError, TrustVerificationError::kUserDeniedConsent);
}

TEST_F_FROM_FIXTURE(TestCommissioner, TestTrustVerificationStageFinishedHandlesError)
{
    // Simulate vendor id verification failure
    mTrustVerificationDelegate.mVerifyVendorIdError = CHIP_ERROR_INTERNAL;

    TrustVerificationStage stage = TrustVerificationStage::kVerifyingAdministratorInformation;
    TrustVerificationError error = TrustVerificationError::kSuccess;

    // Simulate error at operational credentials stage
    mDeviceCommissioner->TrustVerificationStageFinished(stage, error);

    // Wait for JCM to complete
    DrainAndServiceIO();

    EXPECT_EQ(mTrustVerificationDelegate.mProgressUpdates, 2);
    EXPECT_EQ(mTrustVerificationDelegate.mLastStage, TrustVerificationStage::kPerformingVendorIDVerification);
    EXPECT_EQ(mTrustVerificationDelegate.mLastError, TrustVerificationError::kVendorIdVerificationFailed);
}

// Test getting admin fabric index and endpoint ID
TEST_F_FROM_FIXTURE(TestCommissioner, TestParseAdminFabricIndexAndEndpointId)
{
    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, GetJFBFabricIndex());
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);
}

// Test getting operational credentials
TEST_F_FROM_FIXTURE(TestCommissioner, TestParseOperationalCredentials)
{
    // Set up the prerequisites for ParseTrustedRoot
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));

    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseOperationalCredentials(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, GetJFBFabricIndex());
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);

    // Verify the ParseOperationalCredentials results
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, GetJFBFabric()->GetFabricId());
    auto nocCertAsset = GetJFBNodeCertAsset();
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(nocCertAsset.mCert.size()));
    auto iaCertAsset = GetJFBIACertAsset();
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(iaCertAsset.mCert.size()));
}

// Test getting trusted root
TEST_F_FROM_FIXTURE(TestCommissioner, TestParseTrustedRoot)
{
    // Set up the prerequisites for ParseTrustedRoot
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseAdminFabricIndexAndEndpointId(mInfo));
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseOperationalCredentials(mInfo));

    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseTrustedRoot(mInfo));

    // Verify the ParseAdminFabricIndexAndEndpointId results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, GetJFBFabricIndex());
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);

    // Verify the ParseOperationalCredentials results
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, GetJFBFabric()->GetFabricId());
    auto nocCertAsset = GetJFBNodeCertAsset();
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(nocCertAsset.mCert.size()));
    auto iaCertAsset = GetJFBIACertAsset();
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(iaCertAsset.mCert.size()));

    // Verify the ParseTrustedRoot results
    auto rootCertAsset = GetJFBRootCertAsset();
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminRCAC.AllocatedSize(), static_cast<size_t>(rootCertAsset.mCert.size()));
}

// Test parsing administrator info
TEST_F_FROM_FIXTURE(TestCommissioner, TestParseExtraCommissioningInfo)
{
    // Call the method directly to test it
    EXPECT_EQ(CHIP_NO_ERROR, mDeviceCommissioner->ParseExtraCommissioningInfo(mInfo, mCommissioningParams));

    // Verify the ParseExtraCommissioningInfo results
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricIndex, GetJFBFabricIndex());
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminEndpointId, 1);
    EXPECT_EQ(mDeviceCommissioner->mInfo.rootPublicKey.AllocatedSize(), Crypto::kP256_PublicKey_Length);
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminVendorId, static_cast<VendorId>(chip::VendorId::TestVendor1));
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminFabricId, GetJFBFabric()->GetFabricId());
    auto nocCertAsset = GetJFBNodeCertAsset();
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminNOC.AllocatedSize(), static_cast<size_t>(nocCertAsset.mCert.size()));
    auto iaCertAsset = GetJFBIACertAsset();
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminICAC.AllocatedSize(), static_cast<size_t>(iaCertAsset.mCert.size()));

    // Verify the ParseTrustedRoot results
    auto rootCertAsset = GetJFBRootCertAsset();
    EXPECT_EQ(mDeviceCommissioner->mInfo.adminRCAC.AllocatedSize(), static_cast<size_t>(rootCertAsset.mCert.size()));
}
} // namespace JCM
} // namespace Controller
} // namespace chip
