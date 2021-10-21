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

#include "app-common/zap-generated/ids/Clusters.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <controller/WriteInteraction.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app::Clusters;

namespace {
chip::TransportMgrBase gTransportManager;
chip::Test::LoopbackTransport gLoopback;
chip::Test::IOContext gIOContext;
chip::Messaging::ExchangeManager * gExchangeManager;
secure_channel::MessageCounterManager gMessageCounterManager;

using TestContext = chip::Test::MessagingContext;
TestContext sContext;

constexpr EndpointId kTestEndpointId = 1;

enum ResponseDirective
{
    kSendAttributeSuccess,
    kSendAttributeError,
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
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    if (aClusterInfo.mClusterId == TestCluster::Id && aClusterInfo.mFieldId == TestCluster::Attributes::ListStructOctetString::TypeInfo::GetAttributeId()) {
        if (responseDirective == kSendAttributeSuccess) {
            TestCluster::Attributes::ListStructOctetString::TypeInfo::DecodableType value;

            ReturnErrorOnFailure(DataModel::Decode(aReader, value));

            auto iter = value.begin();
            uint8_t i = 0;
            while (iter.Next()) {
                auto &item = iter.GetValue();

                VerifyOrReturnError(item.fabricIndex == i, CHIP_ERROR_INVALID_ARGUMENT);
                i++;
            }

            VerifyOrReturnError(i == 4, CHIP_ERROR_INVALID_ARGUMENT);

            AttributePathParams attributePathParams(aClusterInfo.mClusterId, aClusterInfo.mEndpointId, aClusterInfo.mFieldId);
            aWriteHandler->AddStatus(attributePathParams, Protocols::InteractionModel::Status::Success);
        }
        else {
            AttributePathParams attributePathParams(aClusterInfo.mClusterId, aClusterInfo.mEndpointId, aClusterInfo.mFieldId);
            aWriteHandler->AddStatus(attributePathParams, Protocols::InteractionModel::Status::Failure);
        }

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace app
} // namespace chip

namespace {

class TestWriteInteraction
{
public:
    TestWriteInteraction() {}

    static void TestDataResponse(nlTestSuite * apSuite, void * apContext);
    static void TestAttributeError(nlTestSuite * apSuite, void * apContext);
    static void TestWriteTimeout(nlTestSuite * apSuite, void * apContext);

private:
};

void TestWriteInteraction::TestDataResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    TestCluster::Structs::TestListStructOctet::Type valueBuf[4];
    TestCluster::Attributes::ListStructOctetString::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.fabricIndex = i;
        i++;
    }

    responseDirective = kSendAttributeSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath) {
        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteAttributePath * attributePath,
                                             app::StatusIB status,
                                             CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    chip::Controller::WriteAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
        gExchangeManager, sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb);

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);
}

void TestWriteInteraction::TestAttributeError(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    TestCluster::Attributes::ListStructOctetString::TypeInfo::Type value;
    TestCluster::Structs::TestListStructOctet::Type valueBuf[4];

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.fabricIndex = i;
        i++;
    }

    responseDirective = kSendAttributeError;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath) {
        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [apSuite, &onFailureCbInvoked](const app::ConcreteAttributePath * attributePath,
                                             app::StatusIB status,
                                             CHIP_ERROR aError) {
        NL_TEST_ASSERT(apSuite, attributePath != nullptr);
        onFailureCbInvoked = true;
    };

    chip::Controller::WriteAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
        gExchangeManager, sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb);

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, gExchangeManager->GetNumActiveExchanges() == 0);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestDataResponse", TestWriteInteraction::TestDataResponse),
    NL_TEST_DEF("TestAttributeError", TestWriteInteraction::TestAttributeError),
    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
        "TestWrite",
        &sTests[0],
        Initialize,
        Finalize
};
// clang-format on

int Initialize(void * aContext)
{
    // Initialize System memory and resources
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gIOContext.Init(&sSuite) == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gTransportManager.Init(&gLoopback) == CHIP_NO_ERROR, FAILURE);

    auto * ctx = static_cast<TestContext *>(aContext);
    VerifyOrReturnError(ctx->Init(&sSuite, &gTransportManager, &gIOContext) == CHIP_NO_ERROR, FAILURE);

    gTransportManager.SetSessionManager(&ctx->GetSecureSessionManager());
    gExchangeManager = &ctx->GetExchangeManager();
    VerifyOrReturnError(
        chip::app::InteractionModelEngine::GetInstance()->Init(&ctx->GetExchangeManager(), nullptr) == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

int Finalize(void * aContext)
{
    // Shutdown will ensure no leaked exchange context.
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    gIOContext.Shutdown();
    chip::Platform::MemoryShutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

int TestWriteInteractionTest()
{
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestWriteInteractionTest)
