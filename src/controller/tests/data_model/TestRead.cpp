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

CHIP_ERROR ReadSingleClusterData(const ConcreteAttributePath & aPath, TLV::TLVWriter * apWriter, bool * apDataExists)
{
    if (responseDirective == kSendDataResponse)
    {
        TestCluster::Attributes::ListStructOctetString::TypeInfo::Type value;
        TestCluster::Structs::TestListStructOctet::Type valueBuf[4];

        value = valueBuf;

        uint8_t i = 0;
        for (auto & item : valueBuf)
        {
            item.fabricIndex = i;
            i++;
        }

        ReturnErrorOnFailure(DataModel::Encode(*apWriter, chip::TLV::ContextTag(AttributeDataElement::kCsTag_Data), value));
        return apWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_DataVersion), static_cast<uint64_t>(0));
    }
    else
    {
        return apWriter->Put(chip::TLV::ContextTag(AttributeDataElement::kCsTag_Status),
                             chip::Protocols::InteractionModel::Status::Busy);
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
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

    static void TestDataResponse(nlTestSuite * apSuite, void * apContext);
    static void TestAttributeError(nlTestSuite * apSuite, void * apContext);
    static void TestReadTimeout(nlTestSuite * apSuite, void * apContext);

private:
};

void TestReadInteraction::TestDataResponse(nlTestSuite * apSuite, void * apContext)
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

    chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestAttributeError(nlTestSuite * apSuite, void * apContext)
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

    chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadTimeout(nlTestSuite * apSuite, void * apContext)
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

    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 1);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 2);

    ctx.GetExchangeManager().OnConnectionExpired(ctx.GetSessionBobToAlice());

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);

    //
    // TODO: Figure out why I cannot enable this line below.
    //
    // NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 1);

    chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();

    ctx.GetExchangeManager().OnConnectionExpired(ctx.GetSessionAliceToBob());

    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);

    //
    // TODO: Figure out why I cannot enable this line below.
    //
    // NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestDataResponse", TestReadInteraction::TestDataResponse),
    NL_TEST_DEF("TestAttributeError", TestReadInteraction::TestAttributeError),
    NL_TEST_DEF("TestReadTimeout", TestReadInteraction::TestReadTimeout),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestRead",
    &sTests[0],
    TestContext::Initialize,
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
