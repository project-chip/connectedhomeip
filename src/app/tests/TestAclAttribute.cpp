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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <access/AccessControl.h>
#include <access/examples/PermissiveAccessControlDelegate.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteEventPath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/MessageDef/StatusIB.h>
#include <app/WriteClient.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/test-interaction-model-api.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/interaction_model/Constants.h>

namespace {
using namespace chip;
using namespace chip::app;
using namespace chip::Access;
using namespace chip::Testing;

constexpr AttributeId kTestUnsupportedAttributeId = 77;
constexpr AttributeId kTestAttributeId            = 4;

const MockNodeConfig & TestMockNodeConfig()
{
    using namespace chip::app;
    using namespace chip::app::Clusters::Globals::Attributes;

    // clang-format off
    static const MockNodeConfig config({
        MockEndpointConfig(kRootEndpointId, {
            MockClusterConfig(chip::app::Clusters::AccessControl::Id, {
                ClusterRevision::Id, FeatureMap::Id,
                chip::app::Clusters::AccessControl::Attributes::Acl::Id,
            }),
        }),
        MockEndpointConfig(kTestEndpointId, {
            MockClusterConfig(kTestClusterId, {
                ClusterRevision::Id, FeatureMap::Id, 1, 2, kTestAttributeId
            }),
            MockClusterConfig(kTestDeniedClusterId2, {
                ClusterRevision::Id, FeatureMap::Id, 1, 2, kTestAttributeId
            }),
        }),
        MockEndpointConfig(kTestDeniedEndpointId, {
            MockClusterConfig(kTestClusterId, {
                ClusterRevision::Id, FeatureMap::Id, 1, 2, kTestAttributeId
            }),
        }),
    });
    // clang-format on
    return config;
}

class TestAccessControlDelegate : public AccessControl::Delegate
{
public:
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const chip::Access::RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        if (requestPath.cluster == kTestDeniedClusterId2 || requestPath.endpoint == kTestDeniedEndpointId)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }
        return CHIP_NO_ERROR;
    }
};

AccessControl::Delegate * GetTestAccessControlDelegate()
{
    static TestAccessControlDelegate accessControlDelegate;
    return &accessControlDelegate;
}

class TestDeviceTypeResolver : public AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override { return false; }
} gDeviceTypeResolver;

class MockInteractionModelApp : public chip::app::ReadClient::Callback
{
public:
    void OnAttributeData(const chip::app::ConcreteDataAttributePath & aPath, chip::TLV::TLVReader * apData,
                         const chip::app::StatusIB & status) override
    {
        mGotReport          = true;
        mLastStatusReceived = status;
    }

    void OnError(CHIP_ERROR aError) override { mError = aError; }

    void OnDone(chip::app::ReadClient *) override {}

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        if (aReadPrepareParams.mpAttributePathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpAttributePathParamsList;
        }

        if (aReadPrepareParams.mpDataVersionFilterList != nullptr)
        {
            delete[] aReadPrepareParams.mpDataVersionFilterList;
        }
    }

    bool mGotReport = false;
    chip::app::StatusIB mLastStatusReceived;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};

class TestWriteClientCallback : public WriteClient::Callback
{
public:
    void ResetCounter() { mOnSuccessCalled = mOnErrorCalled = mOnDoneCalled = 0; }
    void OnResponse(const WriteClient * apWriteClient, const ConcreteDataAttributePath & path, StatusIB status) override
    {
        mStatus = status;
        mOnSuccessCalled++;
    }
    void OnError(const WriteClient * apWriteClient, CHIP_ERROR chipError) override
    {
        mOnErrorCalled++;
        mLastErrorReason = app::StatusIB(chipError);
        mError           = chipError;
    }
    void OnDone(WriteClient * apWriteClient) override { mOnDoneCalled++; }

    int mOnSuccessCalled = 0;
    int mOnErrorCalled   = 0;
    int mOnDoneCalled    = 0;
    StatusIB mStatus;
    StatusIB mLastErrorReason;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};
} // namespace

namespace chip {
namespace app {

class TestAclAttribute : public AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();

        Access::GetAccessControl().Finish();
        EXPECT_SUCCESS(Access::GetAccessControl().Init(GetTestAccessControlDelegate(), gDeviceTypeResolver));
        mOldProvider = InteractionModelEngine::GetInstance()->SetDataModelProvider(&TestImCustomDataModel::Instance());
        SetMockNodeConfig(TestMockNodeConfig());
    }

    void TearDown() override
    {
        ResetMockNodeConfig();
        AppContext::TearDown();
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
    }

private:
    chip::app::DataModel::Provider * mOldProvider = nullptr;
};

// Read Client sends a malformed subscribe request, interaction model engine fails to parse the request and generates a status
// report to client, and client is closed.
TEST_F(TestAclAttribute, TestACLDeniedAttribute_Subscribe)
{
    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];
        attributePathParams[0].mEndpointId  = kTestEndpointId;
        attributePathParams[0].mClusterId   = kTestUnsupportedClusterId;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mEndpointId  = kTestEndpointId;
        attributePathParams[1].mClusterId   = kTestUnsupportedClusterId;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
        EXPECT_FALSE(delegate.mGotReport);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];

        attributePathParams[0].mEndpointId  = kTestEndpointId;
        attributePathParams[0].mClusterId   = kTestDeniedClusterId2;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mEndpointId  = kTestDeniedEndpointId;
        attributePathParams[1].mClusterId   = kTestClusterId;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_IM_GLOBAL_STATUS(InvalidAction));
        EXPECT_FALSE(delegate.mGotReport);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    {
        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Subscribe);

        chip::app::AttributePathParams attributePathParams[2];
        attributePathParams[0].mEndpointId  = kTestEndpointId;
        attributePathParams[0].mClusterId   = kTestUnsupportedClusterId;
        attributePathParams[0].mAttributeId = 1;

        attributePathParams[1].mEndpointId  = kTestEndpointId;
        attributePathParams[1].mClusterId   = kTestClusterId;
        attributePathParams[1].mAttributeId = 2;

        ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
        readPrepareParams.mpAttributePathParamsList    = attributePathParams;
        readPrepareParams.mAttributePathParamsListSize = 2;

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_EQ(delegate.mError, CHIP_NO_ERROR);
        EXPECT_TRUE(delegate.mGotReport);
        EXPECT_EQ(engine->GetNumActiveReadHandlers(ReadHandler::InteractionType::Subscribe), 1u);
        delegate.mError     = CHIP_NO_ERROR;
        delegate.mGotReport = false;
    }

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestAclAttribute, TestACLDeniedAttribute_Read)
{
    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    MockInteractionModelApp delegate;
    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                               chip::app::ReadClient::InteractionType::Read);

    chip::app::AttributePathParams attributePathParams[1];

    attributePathParams[0].mEndpointId  = kTestEndpointId;
    attributePathParams[0].mClusterId   = kTestDeniedClusterId2;
    attributePathParams[0].mAttributeId = kTestAttributeId;

    ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
    readPrepareParams.mpAttributePathParamsList    = attributePathParams;
    readPrepareParams.mAttributePathParamsListSize = 1;

    EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

    DrainAndServiceIO();
    EXPECT_EQ(delegate.mLastStatusReceived.mStatus, Protocols::InteractionModel::Status::UnsupportedAccess);
    EXPECT_TRUE(delegate.mGotReport);

    delegate.mGotReport = false;

    EXPECT_EQ(engine->GetNumActiveReadClients(), 0u);
    engine->Shutdown();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestAclAttribute, AccessDeniedPrecedenceOverUnsupportedEndpoint_Write)
{
    {
        Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
        // Shouldn't have anything in the retransmit table when starting the test.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        TestWriteClientCallback callback;
        auto * engine = chip::app::InteractionModelEngine::GetInstance();
        EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()),
                  CHIP_NO_ERROR);

        app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

        AttributePathParams attributePathParams;
        attributePathParams.mEndpointId  = kTestUnsupportedEndpointId;
        attributePathParams.mClusterId   = kTestDeniedClusterId2;
        attributePathParams.mAttributeId = kTestAttributeId;

        app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataTx;

        EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx), CHIP_NO_ERROR);

        EXPECT_EQ(callback.mOnSuccessCalled, 0);

        EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(callback.mOnSuccessCalled, 1);
        EXPECT_EQ(callback.mOnErrorCalled, 0);
        EXPECT_EQ(callback.mOnDoneCalled, 1);
        EXPECT_EQ(callback.mStatus.mStatus, Protocols::InteractionModel::Status::UnsupportedAccess);

        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        engine->Shutdown();
    }

    // Retesting with an UnsupportedEndpoint that is Granted Access, just to make sure that we get the expected Status
    {
        Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
        // Shouldn't have anything in the retransmit table when starting the test.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        TestWriteClientCallback callback;
        auto * engine = chip::app::InteractionModelEngine::GetInstance();
        EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()),
                  CHIP_NO_ERROR);

        app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

        AttributePathParams attributePathParams;
        attributePathParams.mEndpointId  = kTestUnsupportedEndpointId;
        attributePathParams.mClusterId   = kTestClusterId;
        attributePathParams.mAttributeId = kTestAttributeId;

        app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataTx;

        EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx), CHIP_NO_ERROR);

        EXPECT_EQ(callback.mOnSuccessCalled, 0);

        EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(callback.mOnSuccessCalled, 1);
        EXPECT_EQ(callback.mOnErrorCalled, 0);
        EXPECT_EQ(callback.mOnDoneCalled, 1);
        EXPECT_EQ(callback.mStatus.mStatus, Protocols::InteractionModel::Status::UnsupportedEndpoint);

        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        engine->Shutdown();
    }
}

TEST_F(TestAclAttribute, AccessDeniedPrecedenceOverUnsupportedCluster_Write)
{

    {
        Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
        // Shouldn't have anything in the retransmit table when starting the test.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        TestWriteClientCallback callback;
        auto * engine = chip::app::InteractionModelEngine::GetInstance();
        EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()),
                  CHIP_NO_ERROR);

        app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

        AttributePathParams attributePathParams;
        attributePathParams.mEndpointId  = kTestDeniedEndpointId;
        attributePathParams.mClusterId   = kTestUnsupportedClusterId;
        attributePathParams.mAttributeId = kTestAttributeId;

        app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataTx;

        EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx), CHIP_NO_ERROR);

        EXPECT_EQ(callback.mOnSuccessCalled, 0);

        EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(callback.mOnSuccessCalled, 1);
        EXPECT_EQ(callback.mOnErrorCalled, 0);
        EXPECT_EQ(callback.mOnDoneCalled, 1);
        EXPECT_EQ(callback.mStatus.mStatus, Protocols::InteractionModel::Status::UnsupportedAccess);

        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        engine->Shutdown();
    }

    // Retesting with an UnsupportedCluster that is Granted Access, just to make sure that we get the expected Status
    {
        Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
        // Shouldn't have anything in the retransmit table when starting the test.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        TestWriteClientCallback callback;
        auto * engine = chip::app::InteractionModelEngine::GetInstance();
        EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()),
                  CHIP_NO_ERROR);

        app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

        AttributePathParams attributePathParams;
        attributePathParams.mEndpointId  = kTestEndpointId;
        attributePathParams.mClusterId   = kTestUnsupportedClusterId;
        attributePathParams.mAttributeId = kTestAttributeId;

        app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataTx;

        EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx), CHIP_NO_ERROR);

        EXPECT_EQ(callback.mOnSuccessCalled, 0);

        EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(callback.mOnSuccessCalled, 1);
        EXPECT_EQ(callback.mOnErrorCalled, 0);
        EXPECT_EQ(callback.mOnDoneCalled, 1);
        EXPECT_EQ(callback.mStatus.mStatus, Protocols::InteractionModel::Status::UnsupportedCluster);

        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        engine->Shutdown();
    }
}

TEST_F(TestAclAttribute, AccessDeniedPrecedenceOverUnsupportedAttribute_Write)
{

    {
        Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
        // Shouldn't have anything in the retransmit table when starting the test.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        TestWriteClientCallback callback;
        auto * engine = chip::app::InteractionModelEngine::GetInstance();
        EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()),
                  CHIP_NO_ERROR);

        app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

        AttributePathParams attributePathParams;
        attributePathParams.mEndpointId  = kTestDeniedEndpointId;
        attributePathParams.mClusterId   = kTestClusterId;
        attributePathParams.mAttributeId = kTestUnsupportedAttributeId;

        app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataTx;

        EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx), CHIP_NO_ERROR);

        EXPECT_EQ(callback.mOnSuccessCalled, 0);

        EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(callback.mOnSuccessCalled, 1);
        EXPECT_EQ(callback.mOnErrorCalled, 0);
        EXPECT_EQ(callback.mOnDoneCalled, 1);
        EXPECT_EQ(callback.mStatus.mStatus, Protocols::InteractionModel::Status::UnsupportedAccess);

        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        engine->Shutdown();
    }

    // Retesting with an UnsupportedAttribute that is Granted Access, just to make sure that we get the expected Status
    {
        Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
        // Shouldn't have anything in the retransmit table when starting the test.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        TestWriteClientCallback callback;
        auto * engine = chip::app::InteractionModelEngine::GetInstance();
        EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()),
                  CHIP_NO_ERROR);

        app::WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

        AttributePathParams attributePathParams;
        attributePathParams.mEndpointId  = kTestEndpointId;
        attributePathParams.mClusterId   = kTestClusterId;
        attributePathParams.mAttributeId = kTestUnsupportedAttributeId;

        app::Clusters::UnitTesting::Structs::SimpleStruct::Type dataTx;

        EXPECT_EQ(writeClient.EncodeAttribute(attributePathParams, dataTx), CHIP_NO_ERROR);

        EXPECT_EQ(callback.mOnSuccessCalled, 0);

        EXPECT_EQ(writeClient.SendWriteRequest(GetSessionBobToAlice()), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_EQ(callback.mOnSuccessCalled, 1);
        EXPECT_EQ(callback.mOnErrorCalled, 0);
        EXPECT_EQ(callback.mOnDoneCalled, 1);
        EXPECT_EQ(callback.mStatus.mStatus, Protocols::InteractionModel::Status::UnsupportedAttribute);

        // By now we should have closed all exchanges and sent all pending acks, so
        // there should be no queued-up things in the retransmit table.
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

        engine->Shutdown();
    }
}

namespace {

// Simulates the writer losing privilege between the ReplaceAll and AppendItem AttributeDataIBs of a legacy-encoded ACL list write:
// allows the first two Check() calls (kView pre-check + actual write privilege for the first AttributeDataIB), then revokes the
// writer's privilege.
class WriterRevokedAfterFirstAttributeDataIBDelegate : public AccessControl::Delegate
{
public:
    CHIP_ERROR Check(const SubjectDescriptor &, const chip::Access::RequestPath &, Privilege) override
    {
        static constexpr uint8_t kChecksPerWrite = 2;

        mCheckCount++;
        if (mWriterRevoked)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }

        if (mCheckCount == kChecksPerWrite)
        {
            // First AttributeDataIB has been fully checked; simulate writer losing privilege that the following call to
            // WriteHandler::CheckWriteAccess (without the WriteHandler cache) would deny.
            mWriterRevoked = true;
        }
        return CHIP_NO_ERROR;
    }

    uint8_t mCheckCount = 0;
    bool mWriterRevoked = false;
};

} // namespace

// Legacy-encoded ACL list write produces, in one WriteRequestMessage:
//   #1 ReplaceAll([])  — empties ACL, removing the writer's own admin entry
//   #2 AppendItem(item) — same path; its ACL re-check would be denied if not for
//                         the WriteHandler mLastSuccessfullyWrittenPath cache.
// This test asserts mStatus.mStatus == Success to guard the cache: if anyone removes
// it, this test fails and signals that legacy-encoded ACL writes are broken.
TEST_F(TestAclAttribute, LegacyEncodingCacheReuseDuringWrite)
{
    using namespace Protocols::InteractionModel;

    WriterRevokedAfterFirstAttributeDataIBDelegate aclDelegate;
    Access::GetAccessControl().Finish();
    EXPECT_SUCCESS(Access::GetAccessControl().Init(&aclDelegate, gDeviceTypeResolver));

    auto * engine = InteractionModelEngine::GetInstance();

    TestWriteClientCallback callback;
    WriteClient writeClient(engine->GetExchangeManager(), &callback, Optional<uint16_t>::Missing());

    // Encode Attribute
    uint8_t tlvBuf[64];
    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(tlvBuf, sizeof(tlvBuf));
    TLV::TLVType arrayType;
    EXPECT_SUCCESS(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
    {
        TLV::TLVType structType;
        EXPECT_SUCCESS(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType));
        EXPECT_SUCCESS(tlvWriter.EndContainer(structType));
    }
    EXPECT_SUCCESS(tlvWriter.EndContainer(arrayType));
    EXPECT_SUCCESS(tlvWriter.Finalize());

    // Send Write Request
    TLV::TLVReader tlvReader;
    tlvReader.Init(tlvBuf, tlvWriter.GetLengthWritten());
    EXPECT_SUCCESS(tlvReader.Next());
    ConcreteDataAttributePath aclPath(kRootEndpointId, Clusters::AccessControl::Id, Clusters::AccessControl::Attributes::Acl::Id);
    EXPECT_SUCCESS(
        writeClient.PutPreencodedAttribute(aclPath, tlvReader, WriteClient::TestListEncodingOverride::kForceLegacyEncoding));
    EXPECT_SUCCESS(writeClient.SendWriteRequest(GetSessionBobToAlice()));
    DrainAndServiceIO();

    EXPECT_EQ(callback.mOnDoneCalled, 1);

    // Two calls to OnResponse: one for the ReplaceAll, one for the AppendItem.
    EXPECT_EQ(callback.mOnSuccessCalled, 2);

    // Last AttributeStatusIB is the AppendItem; cache (mLastSuccessfullyWrittenPath) makes its re-check pass.
    EXPECT_EQ(callback.mStatus.mStatus, Status::Success);

    EXPECT_EQ(aclDelegate.mCheckCount, 2);
    EXPECT_TRUE(aclDelegate.mWriterRevoked);

    Access::GetAccessControl().Finish();
}

} // namespace app
} // namespace chip
