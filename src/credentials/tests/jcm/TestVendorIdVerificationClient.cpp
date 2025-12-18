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

#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <credentials/jcm/VendorIdVerificationClient.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <pw_unit_test/framework.h>
#include <transport/SecureSession.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Credentials::JCM;
using namespace chip::Messaging;
using namespace chip::Platform;
using namespace chip::Testing;
using namespace chip::TestCerts;
using namespace chip::Transport;

// Mock function for linking
__attribute__((weak)) void InitDataModelHandler() {}

namespace chip {
namespace app {
__attribute__((weak)) void DispatchSingleClusterCommand(const ConcreteCommandPath & aRequestCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{}
} // namespace app
namespace Credentials {
namespace JCM {

class TestVendorIDVerificationDataModel : public CodegenDataModelProvider
{
public:
    TestVendorIDVerificationDataModel() : mMessagingContext(nullptr) {}

    static TestVendorIDVerificationDataModel & Instance(MessagingContext * messagingContext)
    {
        static TestVendorIDVerificationDataModel instance;
        instance.SetMessagingContext(messagingContext);
        return instance;
    }

    void SetMessagingContext(MessagingContext * messagingContext) { mMessagingContext = messagingContext; }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & aRequest,
                                                               chip::TLV::TLVReader & aReader, CommandHandler * aHandler) override
    {
        VerifyOrDie(mMessagingContext != nullptr);

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

class VendorIdVerificationClientTest : public AppContext, public Credentials::JCM::VendorIdVerificationClient
{
public:
    // Performs shared setup for all tests in the test suite.  Run once for the whole suite.
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

    CHIP_ERROR OnLookupOperationalTrustAnchor(VendorId vendorID, Credentials::CertificateKeyId & subjectKeyId,
                                              ByteSpan & globallyTrustedRootSpan) override
    {
        if (mInvalidOperationTrustAnchor)
        {
            globallyTrustedRootSpan = ByteSpan(reinterpret_cast<const uint8_t *>("root"), 4);

            return CHIP_NO_ERROR;
        }

        if (mNoOperationTrustAnchor)
        {
            return CHIP_ERROR_CERT_NOT_FOUND;
        }

        globallyTrustedRootSpan = GetJFBRootCertAsset().mCert;
        return CHIP_NO_ERROR;
    }

    void OnVendorIdVerificationComplete(const CHIP_ERROR & err) override
    {
        mVerificationCompleteCalled = true;
        mVerificationResult         = err;
    }

    void TestVerificationSucceeds();
    void TestVerificationFailsDueToBadRCAC();
    void TestVerificationFailsDueToBadICAC();
    void TestVerificationFailsDueToBadNOC();
    void TestVerificationFailsDueToBadFabricIndex();
    void TestVerificationFailsDueToBadFabricId();
    void TestVerificationFailsDueToBadVendorId();
    void TestVerificationFailsDueToNoOperationalTrustAnchor();
    void TestVerificationFailsDueToInvalidOperationalTrustAnchor();

protected:
    void SetUp() override
    {
        SetupForJFTests();
        AppContext::SetUp();
        mOldProvider =
            InteractionModelEngine::GetInstance()->SetDataModelProvider(&TestVendorIDVerificationDataModel::Instance(this));
        SetMockNodeConfig(TestMockNodeConfig());

        mInfo.adminFabricIndex = GetJFBFabricIndex();
        mInfo.adminVendorId    = VendorId::TestVendor1;
        mInfo.adminNOC.CopyFromSpan(GetJFBNodeCertAsset().mCert);
        mInfo.adminICAC.CopyFromSpan(GetJFBIACertAsset().mCert);
        mInfo.adminRCAC.CopyFromSpan(GetJFBRootCertAsset().mCert);
        mInfo.adminFabricId   = GetJFBFabric()->GetFabricId();
        mInfo.adminEndpointId = 1;
    }

    void TearDown() override
    {
        mInvalidOperationTrustAnchor = false;
        mNoOperationTrustAnchor      = false;

        ExpireJFSessionAToB();

        ResetMockNodeConfig();
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        AppContext::TearDown();
    }

    chip::app::DataModel::Provider * mOldProvider = nullptr;

private:
    bool mVerificationCompleteCalled  = false;
    CHIP_ERROR mVerificationResult    = CHIP_ERROR_INTERNAL;
    bool mInvalidOperationTrustAnchor = false;
    bool mNoOperationTrustAnchor      = false;
    TrustVerificationInfo mInfo;
};

class MockSessionHolder : public SessionHolder
{
public:
    MockSessionHolder(const SessionHandle & handle) : SessionHolder(handle) {}
    ~MockSessionHolder() override = default;
};

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationSucceeds)
{
    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, CHIP_NO_ERROR);
}

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationFailsDueToBadRCAC)
{
    mInfo.adminRCAC.CopyFromSpan(ByteSpan(reinterpret_cast<const uint8_t *>("badrcac"), 7));

    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, CHIP_ERROR_TLV_UNDERRUN);
}

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationFailsDueToBadICAC)
{
    mInfo.adminICAC.CopyFromSpan(ByteSpan(reinterpret_cast<const uint8_t *>("badicac"), 7));

    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, CHIP_ERROR_TLV_UNDERRUN);
}

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationFailsDueToBadNOC)
{
    mInfo.adminNOC.CopyFromSpan(ByteSpan(reinterpret_cast<const uint8_t *>("badnoc"), 7));

    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, CHIP_ERROR_TLV_UNDERRUN);
}

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationFailsDueToBadFabricIndex)
{
    mInfo.adminFabricIndex = FabricIndex(12345);

    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, ChipError(0x501)); // kErrorCode_BadFabricForIndex
}

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationFailsDueToBadFabricId)
{
    mInfo.adminFabricId = FabricId(12345);

    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, CHIP_ERROR_INVALID_SIGNATURE);
}

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationFailsDueToBadVendorId)
{
    mInfo.adminVendorId = VendorId(12345);

    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, CHIP_ERROR_INVALID_SIGNATURE);
}

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationFailsDueToNoOperationalTrustAnchor)
{
    mNoOperationTrustAnchor = true;

    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, CHIP_ERROR_CERT_NOT_FOUND);
}

TEST_F_FROM_FIXTURE(VendorIdVerificationClientTest, TestVerificationFailsDueToInvalidOperationalTrustAnchor)
{
    mInvalidOperationTrustAnchor = true;

    MockSessionHolder sessionHolder(GetJFSessionAToB());
    auto getSession = [sessionHolder]() { return sessionHolder.Get(); };

    CHIP_ERROR err = VerifyVendorId(&GetExchangeManager(), getSession, &mInfo);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    EXPECT_TRUE(mVerificationCompleteCalled);
    EXPECT_EQ(mVerificationResult, CHIP_ERROR_TLV_UNDERRUN);
}

} // namespace JCM
} // namespace Credentials
} // namespace chip
