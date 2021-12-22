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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <controller/ReadInteraction.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>

using TestContext = chip::Test::AppContext;

using namespace chip;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kTestEndpointId = 1;

enum ResponseDirective
{
    kSendDataResponse,
    kSendDataError
};

ResponseDirective responseDirective;

} // namespace

namespace chip {
namespace app {

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{}

bool ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath)
{
    // Mock cluster catalog, only support one command on one cluster on one endpoint.
    return (aCommandPath.mEndpointId == kTestEndpointId && aCommandPath.mClusterId == TestCluster::Id);
}

CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteReadAttributePath & aPath,
                                 AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * apEncoderState)
{

    if (responseDirective == kSendDataResponse)
    {
        AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
        ReturnErrorOnFailure(aAttributeReports.GetError());
        AttributeDataIB::Builder & attributeData = attributeReport.CreateAttributeData();
        ReturnErrorOnFailure(attributeReport.GetError());
        TestCluster::Attributes::ListStructOctetString::TypeInfo::Type value;
        TestCluster::Structs::TestListStructOctet::Type valueBuf[4];

        value = valueBuf;

        uint8_t i = 0;
        for (auto & item : valueBuf)
        {
            item.fabricIndex = i;
            i++;
        }

        attributeData.DataVersion(0);
        AttributePathIB::Builder & attributePath = attributeData.CreatePath();
        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());

        ReturnErrorOnFailure(DataModel::Encode(*(attributeData.GetWriter()),
                                               chip::TLV::ContextTag(chip::to_underlying(AttributeDataIB::Tag::kData)), value));
        ReturnErrorOnFailure(attributeData.EndOfAttributeDataIB().GetError());
        return attributeReport.EndOfAttributeReportIB().GetError();
    }
    else
    {
        AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
        ReturnErrorOnFailure(aAttributeReports.GetError());
        AttributeStatusIB::Builder & attributeStatus = attributeReport.CreateAttributeStatus();
        AttributePathIB::Builder & attributePath     = attributeStatus.CreatePath();
        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());

        StatusIB::Builder & errorStatus = attributeStatus.CreateErrorStatus();
        ReturnErrorOnFailure(attributeStatus.GetError());
        errorStatus.EncodeStatusIB(StatusIB(Protocols::InteractionModel::Status::Busy));
        attributeStatus.EndOfAttributeStatusIB();
        ReturnErrorOnFailure(attributeStatus.GetError());
        return attributeReport.EndOfAttributeReportIB().GetError();
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, ClusterInfo & aClusterInfo,
                                  TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace app
} // namespace chip

namespace {

class TestReadInteraction
{
public:
    TestReadInteraction() {}

    static void TestReadAttributeResponse(nlTestSuite * apSuite, void * apContext);
    static void TestReadAttributeError(nlTestSuite * apSuite, void * apContext);
    static void TestReadAttributeTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestReadEventResponse(nlTestSuite * apSuite, void * apContext);

private:
};

void TestReadInteraction::TestReadAttributeResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath, const auto & dataResponse) {
        uint8_t i = 0;

        auto iter = dataResponse.begin();
        while (iter.Next())
        {
            auto & item = iter.GetValue();
            NL_TEST_ASSERT(apSuite, item.fabricIndex == i);
            i++;
        }

        NL_TEST_ASSERT(apSuite, i == 4);
        NL_TEST_ASSERT(apSuite, iter.GetStatus() == CHIP_NO_ERROR);

        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteAttributePath * attributePath, app::StatusIB aIMStatus,
                                             CHIP_ERROR aError) { onFailureCbInvoked = true; };

    chip::Controller::ReadAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
        &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();
    chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadEventResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &onSuccessCbInvoked](const app::EventHeader & eventHeader, const auto & EventResponse) {
        // TODO: Need to add check when IM event server integration completes
        IgnoreUnusedVariable(apSuite);
        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::EventHeader * eventHeader, Protocols::InteractionModel::Status aIMStatus,
                                             CHIP_ERROR aError) { onFailureCbInvoked = true; };

    chip::Controller::ReadEvent<TestCluster::Events::TestEvent::DecodableType>(&ctx.GetExchangeManager(), sessionHandle,
                                                                               kTestEndpointId, onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();
    chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadAttributeError(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;

    responseDirective = kSendDataError;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath, const auto & dataResponse) {
        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked, apSuite](const app::ConcreteAttributePath * attributePath,
                                                      Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError) {
        NL_TEST_ASSERT(apSuite, (aError != CHIP_NO_ERROR) && (aIMStatus == Protocols::InteractionModel::Status::Busy));
        onFailureCbInvoked = true;
    };

    chip::Controller::ReadAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
        &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();
    chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadAttributeTimeout(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;

    responseDirective = kSendDataError;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath, const auto & dataResponse) {
        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked, apSuite](const app::ConcreteAttributePath * attributePath,
                                                      Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError) {
        NL_TEST_ASSERT(apSuite, aError == CHIP_ERROR_TIMEOUT);
        onFailureCbInvoked = true;
    };

    chip::Controller::ReadAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
        &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 2);

    ctx.GetExchangeManager().ExpireExchangesForSession(ctx.GetSessionBobToAlice());

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);

    //
    // TODO: Figure out why I cannot enable this line below.
    //
    // NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 1);

    ctx.DrainAndServiceIO();
    chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    ctx.DrainAndServiceIO();

    ctx.GetExchangeManager().ExpireExchangesForSession(ctx.GetSessionAliceToBob());

    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);

    //
    // TODO: Figure out why I cannot enable this line below.
    //
    // NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestReadAttributeResponse", TestReadInteraction::TestReadAttributeResponse),
    NL_TEST_DEF("TestReadEventResponse", TestReadInteraction::TestReadEventResponse),
    NL_TEST_DEF("TestReadAttributeError", TestReadInteraction::TestReadAttributeError),
    NL_TEST_DEF("TestReadAttributeTimeout", TestReadInteraction::TestReadAttributeTimeout),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestRead",
    &sTests[0],
    TestContext::InitializeAsync,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestReadInteractionTest()
{
    TestContext gContext;
    nlTestRunner(&sSuite, &gContext);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestReadInteractionTest)
