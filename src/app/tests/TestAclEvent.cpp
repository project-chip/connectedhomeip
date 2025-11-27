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

#include <type_traits>

#include <pw_unit_test/framework.h>

#include <access/AccessControl.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/test-interaction-model-api.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/interaction_model/Constants.h>

#include <app/server-cluster/DefaultServerCluster.h>

#include <app/util/mock/MockNodeConfig.h>

namespace {
using namespace chip;
using namespace chip::Access;
using namespace chip::Testing;

uint8_t gDebugEventBuffer[128];
uint8_t gInfoEventBuffer[128];
uint8_t gCritEventBuffer[128];
chip::app::CircularEventBuffer gCircularEventBuffer[3];
chip::ClusterId kTestClusterId2    = 7;
chip::EndpointId kTestEndpointId   = 1;
chip::EventId kTestEventIdDebug    = 1;
chip::EventId kTestEventIdCritical = 2;
chip::EventId kTestEventIdNotKnown = 3;
chip::TLV::Tag kTestEventTag       = chip::TLV::ContextTag(1);

const chip::Testing::MockNodeConfig & TestMockNodeConfig()
{
    using namespace chip::app;
    using namespace chip::app::Clusters::Globals::Attributes;
    using namespace chip::Testing;

    // clang-format off
    static const MockNodeConfig config({
        MockEndpointConfig(kTestEndpointId, {

            MockClusterConfig(chip::Testing::kTestDeniedClusterId2,
                 {},                    /* attributes */
                 {kTestEventIdDebug}    /* events */
                ),
            MockClusterConfig(kTestClusterId2,
                 {},                    /* attributes */
                 {kTestEventIdCritical} /* events */
                ),
            MockClusterConfig(MockClusterId(2),
                 {},                    /* attributes */
                 {} /* events */
                )

        })
    });
    // clang-format on
    return config;
}

class TestAccessControlDelegate : public AccessControl::Delegate
{
public:
    // Check
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const chip::Access::RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        mNumOfTimesAclIsChecked++;

        if (requestPath.cluster == chip::Testing::kTestDeniedClusterId2)
        {
            return CHIP_ERROR_ACCESS_DENIED;
        }
        return CHIP_NO_ERROR;
    }
    uint8_t mNumOfTimesAclIsChecked = 0;
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

class TestEventGenerator : public chip::app::EventLoggingDelegate
{
public:
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter)
    {
        chip::TLV::TLVType dataContainerType;
        ReturnErrorOnFailure(aWriter.StartContainer(chip::TLV::ContextTag(chip::to_underlying(chip::app::EventDataIB::Tag::kData)),
                                                    chip::TLV::kTLVType_Structure, dataContainerType));
        ReturnErrorOnFailure(aWriter.Put(kTestEventTag, mStatus));
        return aWriter.EndContainer(dataContainerType);
    }

    void SetStatus(int32_t aStatus) { mStatus = aStatus; }

private:
    int32_t mStatus;
};

void GenerateEvents()
{
    chip::EventNumber eid1, eid2;
    chip::app::EventOptions options1;
    options1.mPath     = { kTestEndpointId, chip::Testing::kTestDeniedClusterId2, kTestEventIdDebug };
    options1.mPriority = chip::app::PriorityLevel::Info;

    chip::app::EventOptions options2;
    options2.mPath     = { kTestEndpointId, kTestClusterId2, kTestEventIdCritical };
    options2.mPriority = chip::app::PriorityLevel::Critical;
    TestEventGenerator testEventGenerator;
    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();

    ChipLogDetail(DataManagement, "Generating Events");
    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options1, eid1), CHIP_NO_ERROR);

    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options2, eid2), CHIP_NO_ERROR);
}

class MockInteractionModelApp : public chip::app::ReadClient::Callback
{
public:
    void OnEventData(const chip::app::EventHeader & aEventHeader, chip::TLV::TLVReader * apData,
                     const chip::app::StatusIB * apStatus) override
    {
        ++mNumDataElementIndex;
        mGotEventResponse = true;
        if (apStatus != nullptr && !apStatus->IsSuccess())
        {
            mNumReadEventFailureStatusReceived++;
            mLastStatusReceived = *apStatus;
        }
        else
        {
            mLastStatusReceived = chip::app::StatusIB();
        }
    }

    void OnError(CHIP_ERROR aError) override
    {
        mError     = aError;
        mReadError = true;
    }

    void OnDone(chip::app::ReadClient *) override {}

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        if (aReadPrepareParams.mpEventPathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpEventPathParamsList;
        }
    }

    int mNumDataElementIndex               = 0;
    bool mGotEventResponse                 = false;
    int mNumReadEventFailureStatusReceived = 0;
    int mNumAttributeResponse              = 0;
    bool mGotReport                        = false;
    bool mReadError                        = false;
    chip::app::ReadHandler * mpReadHandler = nullptr;
    chip::app::StatusIB mLastStatusReceived;
    CHIP_ERROR mError = CHIP_NO_ERROR;
    std::vector<chip::app::ConcreteAttributePath> mReceivedAttributePaths;
};

} // namespace

namespace chip {
namespace app {

class TestAclEvent : public AppContext
{
public:
    // Performs setup for each individual test in the test suite
    void SetUp() override
    {
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        AppContext::SetUp();

        ASSERT_EQ(mEventCounter.Init(0), CHIP_NO_ERROR);
        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), MATTER_ARRAY_SIZE(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);

        Access::GetAccessControl().Finish();
        EXPECT_SUCCESS(Access::GetAccessControl().Init(GetTestAccessControlDelegate(), gDeviceTypeResolver));

        mAccessControlDelegate                          = static_cast<TestAccessControlDelegate *>(GetTestAccessControlDelegate());
        mAccessControlDelegate->mNumOfTimesAclIsChecked = 0;

        chip::Testing::SetMockNodeConfig(TestMockNodeConfig());
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        chip::Testing::ResetMockNodeConfig();
        chip::app::EventManagement::DestroyEventManagement();
        AppContext::TearDown();
    }

protected:
    TestAccessControlDelegate * mAccessControlDelegate = nullptr;

private:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
};

TEST_F(TestAclEvent, TestReadRoundtripWithEventStatusIBInEventReport)
{

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_FALSE(rm->TestGetCountRetransTable());

    GenerateEvents();

    auto * engine = chip::app::InteractionModelEngine::GetInstance();

    engine->SetDataModelProvider(CodegenDataModelProviderInstance(nullptr /* delegate */));
    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    // A custom AccessControl::Delegate has been installed that grants privilege to any cluster except the test cluster.
    // When reading events with concrete paths without enough privilege, we will get a EventStatusIB
    {
        chip::app::EventPathParams eventPathParams[1];
        eventPathParams[0].mEndpointId = kTestEndpointId;
        eventPathParams[0].mClusterId  = chip::Testing::kTestDeniedClusterId2;
        eventPathParams[0].mEventId    = kTestEventIdDebug;

        ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 1;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        EXPECT_FALSE(delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotEventResponse);
        EXPECT_EQ(delegate.mNumReadEventFailureStatusReceived, 1); // need exactly one UnsupportedAccess, nothing else
        EXPECT_EQ(delegate.mLastStatusReceived.mStatus, Protocols::InteractionModel::Status::UnsupportedAccess);
        EXPECT_FALSE(delegate.mReadError);
    }

    GenerateEvents();

    // When reading events with withcard paths without enough privilege for reading all events, we will exclude all events without
    // enough priviledge when generating the report.
    {
        chip::app::EventPathParams eventPathParams[1];
        eventPathParams[0].mEndpointId = kTestEndpointId;
        eventPathParams[0].mClusterId  = chip::Testing::kTestDeniedClusterId2;

        ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 1;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        EXPECT_FALSE(delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_FALSE(delegate.mGotEventResponse);
        EXPECT_FALSE(delegate.mNumReadEventFailureStatusReceived);
        EXPECT_FALSE(delegate.mReadError);
    }

    GenerateEvents();

    // When reading events with withcard paths where the partial don't have enough privilege, we will exclude those partial
    // when generating the report.
    {
        chip::app::EventPathParams eventPathParams[1];
        eventPathParams[0].mEndpointId = kTestEndpointId;

        ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 1;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        EXPECT_FALSE(delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_TRUE(delegate.mGotEventResponse);
        EXPECT_FALSE(delegate.mNumReadEventFailureStatusReceived);
        EXPECT_FALSE(delegate.mReadError);
    }

    GenerateEvents();

    // When reading events with two concrete paths where one has enough priviledge, another don't have , we will get a EventStatusIB
    // and eventDataIB
    {
        chip::app::EventPathParams eventPathParams[2];
        eventPathParams[0].mEndpointId = kTestEndpointId;
        eventPathParams[0].mClusterId  = chip::Testing::kTestDeniedClusterId2;
        eventPathParams[0].mEventId    = kTestEventIdDebug;

        eventPathParams[1].mEndpointId = kTestEndpointId;
        eventPathParams[1].mClusterId  = kTestClusterId2;
        eventPathParams[1].mEventId    = kTestEventIdCritical;

        ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 2;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        EXPECT_FALSE(delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();
        EXPECT_TRUE(delegate.mGotEventResponse);
        EXPECT_EQ(delegate.mNumReadEventFailureStatusReceived, 1); // need exactly one UnsupportedAccess, nothing else
        EXPECT_EQ(delegate.mLastStatusReceived.mStatus,
                  chip::Protocols::InteractionModel::Status::Success); // The Second Concrete Event Path is Valid, therefore Last
                                                                       // Status is a Success
        EXPECT_FALSE(delegate.mReadError);
    }
    EXPECT_FALSE(engine->GetNumActiveReadClients());
    engine->Shutdown();
    EXPECT_FALSE(GetExchangeManager().GetNumActiveExchanges());
}

// This DefaultServerCluster is only used for Testing the case where EventInfo() returns a Failure
class FakeDefaultServerCluster : public DefaultServerCluster
{
public:
    static constexpr uint32_t kFakeFeatureMap      = 0x35;
    static constexpr uint32_t kFakeClusterRevision = 1234;

    FakeDefaultServerCluster(const ConcreteClusterPath & path) : DefaultServerCluster(path) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        using namespace chip::app::Clusters;

        switch (request.path.mAttributeId)
        {
        case Globals::Attributes::FeatureMap::Id: {
            uint32_t value = kFakeFeatureMap;
            return encoder.Encode<uint32_t>(std::move(value));
        }
        case Globals::Attributes::ClusterRevision::Id: {
            uint32_t value = kFakeClusterRevision;
            return encoder.Encode<uint32_t>(std::move(value));
        }
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override
    {
        if (path.mEventId == kTestEventIdNotKnown)
        {
            return CHIP_ERROR_INTERNAL;
        }

        return CHIP_NO_ERROR;
    }

    Access::Privilege mEventInfoFakePrivilege = Access::Privilege::kView;
};

// Testing the case when:
// - EventInfo() returns an Error with a Concrete Event Path AND and the path has a Valid Endpoint and Cluster.
// - In that Case, we should get UnsupportedEvent as StatusIB and we should SKIP the second ACL check.
TEST_F(TestAclEvent, TestUnsupportedEventWithValidClusterPath)
{
    using namespace chip::Testing;

    Messaging::ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    EXPECT_FALSE(rm->TestGetCountRetransTable());

    const ConcreteClusterPath kTestClusterPath(kTestEndpointId, MockClusterId(2));
    FakeDefaultServerCluster fakeClusterServer(kTestClusterPath);
    ServerClusterRegistration registration(fakeClusterServer);

    CodegenDataModelProvider model;
    ASSERT_EQ(model.Registry().Register(registration), CHIP_NO_ERROR);

    chip::app::DataModel::Provider * mOldProvider = nullptr;
    auto * engine                                 = chip::app::InteractionModelEngine::GetInstance();
    mOldProvider                                  = engine->SetDataModelProvider(&model);

    EXPECT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), app::reporting::GetDefaultReportScheduler()), CHIP_NO_ERROR);

    GenerateEvents();

    // A custom AccessControl::Delegate has been installed that grants privilege to any cluster except the test cluster.
    // When reading events with concrete paths without enough privilege, we will get a EventStatusIB
    {
        chip::app::EventPathParams eventPathParams[1];
        eventPathParams[0].mEndpointId = kTestEndpointId;
        eventPathParams[0].mClusterId  = MockClusterId(2);
        eventPathParams[0].mEventId    = kTestEventIdNotKnown;

        ReadPrepareParams readPrepareParams(GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 1;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        EXPECT_FALSE(delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        EXPECT_EQ(readClient.SendRequest(readPrepareParams), CHIP_NO_ERROR);

        DrainAndServiceIO();

        EXPECT_TRUE(delegate.mGotEventResponse);
        EXPECT_EQ(delegate.mNumReadEventFailureStatusReceived, 1); // need exactly one UnsupportedEvent, nothing else
        EXPECT_EQ(delegate.mLastStatusReceived.mStatus, Protocols::InteractionModel::Status::UnsupportedEvent);

        // Since we failed the Existence check, we shouldn't reach the 2nd ACL Check, and should have only done the 1st ACL Check
        // (against the View Privilege according to Spec)
        EXPECT_EQ(mAccessControlDelegate->mNumOfTimesAclIsChecked, 1);
        EXPECT_FALSE(delegate.mReadError);
    }

    EXPECT_FALSE(engine->GetNumActiveReadClients());

    engine->Shutdown();
    EXPECT_FALSE(GetExchangeManager().GetNumActiveExchanges());
    engine->SetDataModelProvider(mOldProvider);

    EXPECT_SUCCESS(model.Registry().Unregister(&fakeClusterServer));
    EXPECT_SUCCESS(model.Shutdown());
}

} // namespace app
} // namespace chip
