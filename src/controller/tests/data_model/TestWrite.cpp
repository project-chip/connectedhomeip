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
#include <app/tests/AppTestContext.h>
#include <controller/WriteInteraction.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>
#include <protocols/interaction_model/Constants.h>

using TestContext = chip::Test::AppContext;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitTesting;
using namespace chip::Protocols;

namespace {

constexpr EndpointId kTestEndpointId       = 1;
constexpr DataVersion kRejectedDataVersion = 1;
constexpr DataVersion kAcceptedDataVersion = 5;

enum ResponseDirective
{
    kSendAttributeSuccess,
    kSendAttributeError,
    kSendMultipleSuccess,
    kSendMultipleErrors,
};

ResponseDirective responseDirective;

} // namespace

namespace chip {
namespace app {

const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath)
{
    // Note: This test does not make use of the real attribute metadata.
    static EmberAfAttributeMetadata stub = { .defaultValue = EmberAfDefaultOrMinMaxAttributeValue(uint32_t(0)) };
    return &stub;
}

CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    static ListIndex listStructOctetStringElementCount = 0;

    if (aPath.mDataVersion.HasValue() && aPath.mDataVersion.Value() == kRejectedDataVersion)
    {
        return aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::DataVersionMismatch);
    }

    if (aPath.mClusterId == Clusters::UnitTesting::Id &&
        aPath.mAttributeId == Attributes::ListStructOctetString::TypeInfo::GetAttributeId())
    {
        if (responseDirective == kSendAttributeSuccess)
        {
            if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
            {

                Attributes::ListStructOctetString::TypeInfo::DecodableType value;

                ReturnErrorOnFailure(DataModel::Decode(aReader, value));

                auto iter                         = value.begin();
                listStructOctetStringElementCount = 0;
                while (iter.Next())
                {
                    auto & item = iter.GetValue();

                    VerifyOrReturnError(item.member1 == listStructOctetStringElementCount, CHIP_ERROR_INVALID_ARGUMENT);
                    listStructOctetStringElementCount++;
                }

                aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
            }
            else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
            {
                Structs::TestListStructOctet::DecodableType item;
                ReturnErrorOnFailure(DataModel::Decode(aReader, item));
                VerifyOrReturnError(item.member1 == listStructOctetStringElementCount, CHIP_ERROR_INVALID_ARGUMENT);
                listStructOctetStringElementCount++;

                aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
            }
            else
            {
                return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            }
        }
        else
        {
            aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Failure);
        }

        return CHIP_NO_ERROR;
    }
    if (aPath.mClusterId == Clusters::UnitTesting::Id && aPath.mAttributeId == Attributes::ListFabricScoped::Id)
    {
        // Mock a invalid SubjectDescriptor
        AttributeValueDecoder decoder(aReader, Access::SubjectDescriptor());
        if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            Attributes::ListFabricScoped::TypeInfo::DecodableType value;

            ReturnErrorOnFailure(decoder.Decode(value));

            auto iter = value.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                (void) item;
            }

            aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
        }
        else if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            Structs::TestFabricScoped::DecodableType item;
            ReturnErrorOnFailure(decoder.Decode(item));

            aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        return CHIP_NO_ERROR;
    }

    if (aPath.mClusterId == Clusters::UnitTesting::Id && aPath.mAttributeId == Attributes::Boolean::TypeInfo::GetAttributeId())
    {
        InteractionModel::Status status;
        if (responseDirective == kSendMultipleSuccess)
        {
            status = InteractionModel::Status::Success;
        }
        else if (responseDirective == kSendMultipleErrors)
        {
            status = InteractionModel::Status::Failure;
        }
        else
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        for (size_t i = 0; i < 4; ++i)
        {
            aWriteHandler->AddStatus(aPath, status);
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
    static void TestDataResponseWithAcceptedDataVersion(nlTestSuite * apSuite, void * apContext);
    static void TestDataResponseWithRejectedDataVersion(nlTestSuite * apSuite, void * apContext);
    static void TestAttributeError(nlTestSuite * apSuite, void * apContext);
    static void TestFabricScopedAttributeWithoutFabricIndex(nlTestSuite * apSuite, void * apContext);
    static void TestWriteTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestMultipleSuccessResponses(nlTestSuite * apSuite, void * apContext);
    static void TestMultipleFailureResponses(nlTestSuite * apSuite, void * apContext);
};

void TestWriteInteraction::TestDataResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Clusters::UnitTesting::Structs::TestListStructOctet::Type valueBuf[4];
    Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.member1 = i;
        i++;
    }

    responseDirective = kSendAttributeSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo>(
        sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestWriteInteraction::TestDataResponseWithAcceptedDataVersion(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Clusters::UnitTesting::Structs::TestListStructOctet::Type valueBuf[4];
    Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.member1 = i;
        i++;
    }

    responseDirective = kSendAttributeSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    chip::Optional<chip::DataVersion> dataVersion;
    dataVersion.SetValue(kAcceptedDataVersion);
    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo>(
        sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb, nullptr, dataVersion);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestWriteInteraction::TestDataResponseWithRejectedDataVersion(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Clusters::UnitTesting::Structs::TestListStructOctet::Type valueBuf[4];
    Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.member1 = i;
        i++;
    }

    responseDirective = kSendAttributeSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    chip::Optional<chip::DataVersion> dataVersion(kRejectedDataVersion);
    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo>(
        sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb, nullptr, dataVersion);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestWriteInteraction::TestAttributeError(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Attributes::ListStructOctetString::TypeInfo::Type value;
    Structs::TestListStructOctet::Type valueBuf[4];

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.member1 = i;
        i++;
    }

    responseDirective = kSendAttributeError;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [apSuite, &onFailureCbInvoked](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        NL_TEST_ASSERT(apSuite, attributePath != nullptr);
        onFailureCbInvoked = true;
    };

    Controller::WriteAttribute<Attributes::ListStructOctetString::TypeInfo>(sessionHandle, kTestEndpointId, value, onSuccessCb,
                                                                            onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestWriteInteraction::TestFabricScopedAttributeWithoutFabricIndex(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;
    Clusters::UnitTesting::Structs::TestFabricScoped::Type valueBuf[4];
    Clusters::UnitTesting::Attributes::ListFabricScoped::TypeInfo::Type value;

    value = valueBuf;

    uint8_t i = 0;
    for (auto & item : valueBuf)
    {
        item.fabricIndex = i;
        i++;
    }

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&onSuccessCbInvoked](const ConcreteAttributePath & attributePath) { onSuccessCbInvoked = true; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked, &apSuite](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) {
        NL_TEST_ASSERT(apSuite, aError == CHIP_IM_GLOBAL_STATUS(UnsupportedAccess));
        onFailureCbInvoked = true;
    };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::ListFabricScoped::TypeInfo>(
        sessionHandle, kTestEndpointId, value, onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestWriteInteraction::TestMultipleSuccessResponses(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx   = *static_cast<TestContext *>(apContext);
    auto sessionHandle  = ctx.GetSessionBobToAlice();
    size_t successCalls = 0;
    size_t failureCalls = 0;

    responseDirective = kSendMultipleSuccess;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&successCalls](const ConcreteAttributePath & attributePath) { ++successCalls; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&failureCalls](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) { ++failureCalls; };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::Boolean::TypeInfo>(sessionHandle, kTestEndpointId, true,
                                                                                           onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, successCalls == 1);
    NL_TEST_ASSERT(apSuite, failureCalls == 0);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestWriteInteraction::TestMultipleFailureResponses(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx   = *static_cast<TestContext *>(apContext);
    auto sessionHandle  = ctx.GetSessionBobToAlice();
    size_t successCalls = 0;
    size_t failureCalls = 0;

    responseDirective = kSendMultipleErrors;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&successCalls](const ConcreteAttributePath & attributePath) { ++successCalls; };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&failureCalls](const ConcreteAttributePath * attributePath, CHIP_ERROR aError) { ++failureCalls; };

    chip::Controller::WriteAttribute<Clusters::UnitTesting::Attributes::Boolean::TypeInfo>(sessionHandle, kTestEndpointId, true,
                                                                                           onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, successCalls == 0);
    NL_TEST_ASSERT(apSuite, failureCalls == 1);
    NL_TEST_ASSERT(apSuite, chip::app::InteractionModelEngine::GetInstance()->GetNumActiveWriteHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestDataResponse", TestWriteInteraction::TestDataResponse),
    NL_TEST_DEF("TestDataResponseWithAcceptedDataVersion", TestWriteInteraction::TestDataResponseWithAcceptedDataVersion),
    NL_TEST_DEF("TestDataResponseWithRejectedDataVersion", TestWriteInteraction::TestDataResponseWithRejectedDataVersion),
    NL_TEST_DEF("TestAttributeError", TestWriteInteraction::TestAttributeError),
    NL_TEST_DEF("TestWriteFabricScopedAttributeWithoutFabricIndex",
                TestWriteInteraction::TestFabricScopedAttributeWithoutFabricIndex),
    NL_TEST_DEF("TestMultipleSuccessResponses", TestWriteInteraction::TestMultipleSuccessResponses),
    NL_TEST_DEF("TestMultipleFailureResponses", TestWriteInteraction::TestMultipleFailureResponses),
    NL_TEST_SENTINEL(),
};

nlTestSuite sSuite = {
    "TestWrite",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};

} // namespace

int TestWriteInteractionTest()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestWriteInteractionTest)
