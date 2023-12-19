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

#include "lib/support/CHIPMem.h"
#include <access/AccessControl.h>
#include <app/AttributeAccessInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <nlunit-test.h>
#include <protocols/interaction_model/Constants.h>

#include <type_traits>

namespace {
using namespace chip;
using namespace chip::Access;

uint8_t gDebugEventBuffer[128];
uint8_t gInfoEventBuffer[128];
uint8_t gCritEventBuffer[128];
chip::app::CircularEventBuffer gCircularEventBuffer[3];
chip::ClusterId kTestClusterId1    = 100;
chip::ClusterId kTestClusterId2    = 7;
chip::EndpointId kTestEndpointId   = 1;
chip::EventId kTestEventIdDebug    = 1;
chip::EventId kTestEventIdCritical = 2;
chip::TLV::Tag kTestEventTag       = chip::TLV::ContextTag(1);

class TestAccessControlDelegate : public AccessControl::Delegate
{
public:
    // Check
    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const chip::Access::RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        if (requestPath.cluster == kTestClusterId1)
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

class TestContext : public chip::Test::AppContext
{
public:
    // Performs setup for each individual test in the test suite
    CHIP_ERROR SetUp() override
    {
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        ReturnErrorOnFailure(chip::Test::AppContext::SetUp());

        CHIP_ERROR err = CHIP_NO_ERROR;
        VerifyOrExit((err = mEventCounter.Init(0)) == CHIP_NO_ERROR,
                     ChipLogError(AppServer, "Init EventCounter failed: %" CHIP_ERROR_FORMAT, err.Format()));
        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), ArraySize(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);

        Access::GetAccessControl().Finish();
        Access::GetAccessControl().Init(GetTestAccessControlDelegate(), gDeviceTypeResolver);

    exit:
        return err;
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        chip::app::EventManagement::DestroyEventManagement();
        chip::Test::AppContext::TearDown();
    }

private:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
};

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

void GenerateEvents(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::EventNumber eid1, eid2;
    chip::app::EventOptions options1;
    options1.mPath     = { kTestEndpointId, kTestClusterId1, kTestEventIdDebug };
    options1.mPriority = chip::app::PriorityLevel::Info;

    chip::app::EventOptions options2;
    options2.mPath     = { kTestEndpointId, kTestClusterId2, kTestEventIdCritical };
    options2.mPriority = chip::app::PriorityLevel::Critical;
    TestEventGenerator testEventGenerator;
    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();

    ChipLogDetail(DataManagement, "Generating Events");
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options1, eid1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options2, eid2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
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
class TestAclEvent
{
public:
    static void TestReadRoundtripWithEventStatusIBInEventReport(nlTestSuite * apSuite, void * apContext);
};

void TestAclEvent::TestReadRoundtripWithEventStatusIBInEventReport(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    Messaging::ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    // Shouldn't have anything in the retransmit table when starting the test.
    NL_TEST_ASSERT(apSuite, rm->TestGetCountRetransTable() == 0);

    GenerateEvents(apSuite, apContext);

    auto * engine = chip::app::InteractionModelEngine::GetInstance();
    err           = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    // A custom AccessControl::Delegate has been installed that grants privilege to any cluster except the test cluster.
    // When reading events with concrete paths without enough privilege, we will get a EventStatusIB
    {
        chip::app::EventPathParams eventPathParams[1];
        eventPathParams[0].mEndpointId = kTestEndpointId;
        eventPathParams[0].mClusterId  = kTestClusterId1;
        eventPathParams[0].mEventId    = kTestEventIdDebug;

        ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 1;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();

        NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
        NL_TEST_ASSERT(apSuite, delegate.mNumReadEventFailureStatusReceived == 1);
        NL_TEST_ASSERT(apSuite, delegate.mLastStatusReceived.mStatus == Protocols::InteractionModel::Status::UnsupportedAccess);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);
    }

    GenerateEvents(apSuite, apContext);

    // When reading events with withcard paths without enough privilege for reading all events, we will exclude all events without
    // enough priviledge when generating the report.
    {
        chip::app::EventPathParams eventPathParams[1];
        eventPathParams[0].mEndpointId = kTestEndpointId;
        eventPathParams[0].mClusterId  = kTestClusterId1;

        ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 1;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);
        NL_TEST_ASSERT(apSuite, delegate.mNumReadEventFailureStatusReceived == 0);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);
    }

    GenerateEvents(apSuite, apContext);

    // When reading events with withcard paths where the partial don't have enough privilege, we will exclude those partial
    // when generating the report.
    {
        chip::app::EventPathParams eventPathParams[1];
        eventPathParams[0].mEndpointId = kTestEndpointId;

        ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 1;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
        NL_TEST_ASSERT(apSuite, delegate.mNumReadEventFailureStatusReceived == 0);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);
    }

    GenerateEvents(apSuite, apContext);

    // When reading events with two concrete paths where one has enough priviledge, another don't have , we will get a EventStatusIB
    // and eventDataIB
    {
        chip::app::EventPathParams eventPathParams[2];
        eventPathParams[0].mEndpointId = kTestEndpointId;
        eventPathParams[0].mClusterId  = kTestClusterId1;
        eventPathParams[0].mEventId    = kTestEventIdDebug;
        eventPathParams[1].mEndpointId = kTestEndpointId;
        eventPathParams[1].mClusterId  = kTestClusterId2;
        eventPathParams[1].mEventId    = kTestEventIdCritical;

        ReadPrepareParams readPrepareParams(ctx.GetSessionBobToAlice());
        readPrepareParams.mpEventPathParamsList    = eventPathParams;
        readPrepareParams.mEventPathParamsListSize = 1;
        readPrepareParams.mEventNumber.SetValue(1);

        MockInteractionModelApp delegate;
        NL_TEST_ASSERT(apSuite, !delegate.mGotEventResponse);

        app::ReadClient readClient(chip::app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), delegate,
                                   chip::app::ReadClient::InteractionType::Read);

        err = readClient.SendRequest(readPrepareParams);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(apSuite, delegate.mGotEventResponse);
        NL_TEST_ASSERT(apSuite, delegate.mNumReadEventFailureStatusReceived == 1);
        NL_TEST_ASSERT(apSuite, !delegate.mReadError);
    }
    NL_TEST_ASSERT(apSuite, engine->GetNumActiveReadClients() == 0);
    engine->Shutdown();
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

} // namespace app
} // namespace chip

namespace {

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestReadRoundtripWithEventStatusIBInEventReport", chip::app::TestAclEvent::TestReadRoundtripWithEventStatusIBInEventReport),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestAclEvent",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
// clang-format on

} // namespace

int TestAclEvent()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAclEvent)
