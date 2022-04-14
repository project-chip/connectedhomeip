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

#include "transport/SecureSession.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <controller/ReadInteraction.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>
#include <protocols/interaction_model/Constants.h>

using TestContext = chip::Test::AppContext;

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::Protocols;

namespace {

constexpr EndpointId kTestEndpointId = 1;
constexpr DataVersion kDataVersion   = 5;

enum ResponseDirective
{
    kSendDataResponse,
    kSendDataError
};

ResponseDirective responseDirective;

// Number of reads of TestCluster::Attributes::Int16u that we have observed.
// Every read will increment this count by 1 and return the new value.
uint16_t totalReadCount = 0;

} // namespace

namespace chip {
namespace app {
CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * apEncoderState)
{
    if (responseDirective == kSendDataResponse)
    {
        if (aPath.mClusterId == app::Clusters::TestCluster::Id &&
            aPath.mAttributeId == app::Clusters::TestCluster::Attributes::ListFabricScoped::Id)
        {
            AttributeValueEncoder::AttributeEncodeState state =
                (apEncoderState == nullptr ? AttributeValueEncoder::AttributeEncodeState() : *apEncoderState);
            AttributeValueEncoder valueEncoder(aAttributeReports, aSubjectDescriptor.fabricIndex, aPath, 0 /* data version */,
                                               aIsFabricFiltered, state);

            return valueEncoder.EncodeList([aSubjectDescriptor](const auto & encoder) -> CHIP_ERROR {
                app::Clusters::TestCluster::Structs::TestFabricScoped::Type val;
                val.fabricIndex = aSubjectDescriptor.fabricIndex;
                ReturnErrorOnFailure(encoder.Encode(val));
                val.fabricIndex = (val.fabricIndex == 1) ? 2 : 1;
                ReturnErrorOnFailure(encoder.Encode(val));
                return CHIP_NO_ERROR;
            });
        }
        if (aPath.mClusterId == app::Clusters::TestCluster::Id &&
            aPath.mAttributeId == app::Clusters::TestCluster::Attributes::Int16u::Id)
        {
            AttributeValueEncoder::AttributeEncodeState state =
                (apEncoderState == nullptr ? AttributeValueEncoder::AttributeEncodeState() : *apEncoderState);
            AttributeValueEncoder valueEncoder(aAttributeReports, aSubjectDescriptor.fabricIndex, aPath, 0 /* data version */,
                                               aIsFabricFiltered, state);

            return valueEncoder.Encode(++totalReadCount);
        }

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

        attributeData.DataVersion(kDataVersion);
        ReturnErrorOnFailure(attributeData.GetError());
        AttributePathIB::Builder & attributePath = attributeData.CreatePath();
        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());

        ReturnErrorOnFailure(
            DataModel::Encode(*(attributeData.GetWriter()), TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)), value));
        ReturnErrorOnFailure(attributeData.EndOfAttributeDataIB().GetError());
        return attributeReport.EndOfAttributeReportIB().GetError();
    }

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

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredDataVersion)
{
    if (aRequiredDataVersion == kDataVersion)
    {
        return true;
    }

    return false;
}

bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint)
{
    return false;
}
} // namespace app
} // namespace chip

namespace {

class TestReadInteraction : public app::ReadHandler::ApplicationCallback
{
public:
    TestReadInteraction() {}

    static void TestReadAttributeResponse(nlTestSuite * apSuite, void * apContext);
    static void TestReadDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadAttributeError(nlTestSuite * apSuite, void * apContext);
    static void TestReadAttributeTimeout(nlTestSuite * apSuite, void * apContext);
    static void TestReadEventResponse(nlTestSuite * apSuite, void * apContext);
    static void TestReadFabricScopedWithoutFabricFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadFabricScopedWithFabricFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_MultipleSubscriptions(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_SubscriptionAppRejection(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_MultipleReads(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_OneSubscribeMultipleReads(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_TwoSubscribesMultipleReads(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_MultipleSubscriptionsWithDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_SubscriptionAlteredReportingIntervals(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerResourceExhaustion_MultipleSubscriptions(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerResourceExhaustion_MultipleReads(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_KillOverQuotaSubscriptions(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandler_KillOldestSubscriptions(nlTestSuite * apSuite, void * apContext);

private:
    static constexpr uint16_t kTestMinInterval = 33;
    static constexpr uint16_t kTestMaxInterval = 66;

    CHIP_ERROR OnSubscriptionRequested(app::ReadHandler & aReadHandler, Transport::SecureSession & aSecureSession)
    {
        VerifyOrReturnError(!mEmitSubscriptionError, CHIP_ERROR_INVALID_ARGUMENT);

        if (mAlterSubscriptionIntervals)
        {
            ReturnErrorOnFailure(aReadHandler.SetReportingIntervals(kTestMinInterval, kTestMaxInterval));
        }

        return CHIP_NO_ERROR;
    }

    void OnSubscriptionEstablished(app::ReadHandler & aReadHandler) { mNumActiveSubscriptions++; }

    void OnSubscriptionTerminated(app::ReadHandler & aReadHandler) { mNumActiveSubscriptions--; }

    // Issue the given number of reads in parallel and wait for them all to
    // succeed.
    static void MultipleReadHelper(nlTestSuite * apSuite, TestContext & aCtx, size_t aReadCount);

    // Establish the given number of subscriptions, then issue the given number
    // of reads in parallel and wait for them all to succeed.
    static void SubscribeThenReadHelper(nlTestSuite * apSuite, TestContext & aCtx, size_t aSubscribeCount, size_t aReadCount);

    bool mEmitSubscriptionError      = false;
    int32_t mNumActiveSubscriptions  = 0;
    bool mAlterSubscriptionIntervals = false;
};

TestReadInteraction gTestReadInteraction;

void TestReadInteraction::TestReadAttributeResponse(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &onSuccessCbInvoked](const app::ConcreteDataAttributePath & attributePath,
                                                      const auto & dataResponse) {
        uint8_t i = 0;
        NL_TEST_ASSERT(apSuite, attributePath.mDataVersion.HasValue() && attributePath.mDataVersion.Value() == kDataVersion);
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
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    Controller::ReadAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(&ctx.GetExchangeManager(), sessionHandle,
                                                                                        kTestEndpointId, onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadDataVersionFilter(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &onSuccessCbInvoked](const app::ConcreteDataAttributePath & attributePath,
                                                      const auto & dataResponse) {
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
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    Optional<DataVersion> dataVersion(kDataVersion);
    Controller::ReadAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
        &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, true, dataVersion);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
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
    auto onFailureCb = [&onFailureCbInvoked](const app::EventHeader * eventHeader, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    Controller::ReadEvent<TestCluster::Events::TestEvent::DecodableType>(&ctx.GetExchangeManager(), sessionHandle, kTestEndpointId,
                                                                         onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
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
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked, apSuite](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        NL_TEST_ASSERT(apSuite, aError.IsIMStatus() && app::StatusIB(aError).mStatus == Protocols::InteractionModel::Status::Busy);
        onFailureCbInvoked = true;
    };

    Controller::ReadAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(&ctx.GetExchangeManager(), sessionHandle,
                                                                                        kTestEndpointId, onSuccessCb, onFailureCb);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
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
    auto onSuccessCb = [&onSuccessCbInvoked](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked, apSuite](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        NL_TEST_ASSERT(apSuite, aError == CHIP_ERROR_TIMEOUT);
        onFailureCbInvoked = true;
    };

    Controller::ReadAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(&ctx.GetExchangeManager(), sessionHandle,
                                                                                        kTestEndpointId, onSuccessCb, onFailureCb);

    ctx.ExpireSessionAliceToBob();

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 1);

    ctx.ExpireSessionBobToAlice();

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);

    //
    // Let's put back the sessions so that the next tests (which assume a valid initialized set of sessions)
    // can function correctly.
    //
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadHandler_MultipleSubscriptions(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                        = *static_cast<TestContext *>(apContext);
    auto sessionHandle                       = ctx.GetSessionBobToAlice();
    uint32_t numSuccessCalls                 = 0;
    uint32_t numSubscriptionEstablishedCalls = 0;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&numSuccessCalls](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        numSuccessCalls++;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&apSuite](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        //
        // We shouldn't be encountering any failures in this test.
        //
        NL_TEST_ASSERT(apSuite, false);
    };

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls](const app::ReadClient & readClient) {
        numSubscriptionEstablishedCalls++;
    };

    //
    // Test the application callback as well to ensure we get the right number of SubscriptionEstablishment/Termination
    // callbacks.
    //
    app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&gTestReadInteraction);

    //
    // Try to issue parallel subscriptions that will exceed the value for CHIP_IM_MAX_NUM_READ_HANDLER.
    // If heap allocation is correctly setup, this should result in it successfully servicing more than the number
    // present in that define.
    //
    for (int i = 0; i < (CHIP_IM_MAX_NUM_READ_HANDLER + 1); i++)
    {
        NL_TEST_ASSERT(apSuite,
                       Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                           &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 20,
                           onSubscriptionEstablishedCb, false, true) == CHIP_NO_ERROR);
    }

    // There are too many messages and the test (gcc_debug, which includes many sanity checks) will be quite slow. Note: report
    // engine is using ScheduleWork which cannot be handled by DrainAndServiceIO correctly.
    ctx.GetIOContext().DriveIOUntil(System::Clock::Seconds16(60), [&]() {
        return numSuccessCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1) &&
            numSubscriptionEstablishedCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1);
    });

    NL_TEST_ASSERT(apSuite, numSuccessCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));
    NL_TEST_ASSERT(apSuite, numSubscriptionEstablishedCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));
    NL_TEST_ASSERT(apSuite, gTestReadInteraction.mNumActiveSubscriptions == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();

    NL_TEST_ASSERT(apSuite, gTestReadInteraction.mNumActiveSubscriptions == 0);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    app::InteractionModelEngine::GetInstance()->UnregisterReadHandlerAppCallback();
}

void TestReadInteraction::TestReadHandler_SubscriptionAppRejection(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                        = *static_cast<TestContext *>(apContext);
    auto sessionHandle                       = ctx.GetSessionBobToAlice();
    uint32_t numSuccessCalls                 = 0;
    uint32_t numFailureCalls                 = 0;
    uint32_t numSubscriptionEstablishedCalls = 0;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&numSuccessCalls](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        numSuccessCalls++;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&numFailureCalls](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        numFailureCalls++;
    };

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls](const app::ReadClient & readClient) {
        numSubscriptionEstablishedCalls++;
    };

    //
    // Test the application callback as well to ensure we get the right number of SubscriptionEstablishment/Termination
    // callbacks.
    //
    app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&gTestReadInteraction);

    //
    // Test the application rejecting subscriptions.
    //
    gTestReadInteraction.mEmitSubscriptionError = true;

    NL_TEST_ASSERT(apSuite,
                   Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                       &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 10,
                       onSubscriptionEstablishedCb, false, true) == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, numSuccessCalls == 0);

    //
    // Failures won't get routed to us here since re-subscriptions are enabled by default in the Controller::SubscribeAttribute
    // implementation.
    //
    NL_TEST_ASSERT(apSuite, numFailureCalls == 0);
    NL_TEST_ASSERT(apSuite, numSubscriptionEstablishedCalls == 0);
    NL_TEST_ASSERT(apSuite, gTestReadInteraction.mNumActiveSubscriptions == 0);

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();

    NL_TEST_ASSERT(apSuite, gTestReadInteraction.mNumActiveSubscriptions == 0);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    app::InteractionModelEngine::GetInstance()->UnregisterReadHandlerAppCallback();
    gTestReadInteraction.mEmitSubscriptionError = false;
}

void TestReadInteraction::TestReadHandler_SubscriptionAlteredReportingIntervals(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                        = *static_cast<TestContext *>(apContext);
    auto sessionHandle                       = ctx.GetSessionBobToAlice();
    uint32_t numSuccessCalls                 = 0;
    uint32_t numFailureCalls                 = 0;
    uint32_t numSubscriptionEstablishedCalls = 0;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&numSuccessCalls](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        numSuccessCalls++;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&numFailureCalls](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        numFailureCalls++;
    };

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls, &apSuite](const app::ReadClient & readClient) {
        uint16_t minInterval = 0, maxInterval = 0;

        CHIP_ERROR err = readClient.GetReportingIntervals(minInterval, maxInterval);

        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(apSuite, minInterval == kTestMinInterval);
        NL_TEST_ASSERT(apSuite, maxInterval == kTestMaxInterval);

        numSubscriptionEstablishedCalls++;
    };

    //
    // Test the application callback as well to ensure we get the right number of SubscriptionEstablishment/Termination
    // callbacks.
    //
    app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&gTestReadInteraction);

    //
    // Test the server-side application altering the subscription intervals.
    //
    gTestReadInteraction.mAlterSubscriptionIntervals = true;

    NL_TEST_ASSERT(apSuite,
                   Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                       &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 10,
                       onSubscriptionEstablishedCb, false, true) == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    //
    // Failures won't get routed to us here since re-subscriptions are enabled by default in the Controller::SubscribeAttribute
    // implementation.
    //
    NL_TEST_ASSERT(apSuite, numSuccessCalls != 0);
    NL_TEST_ASSERT(apSuite, numFailureCalls == 0);
    NL_TEST_ASSERT(apSuite, numSubscriptionEstablishedCalls == 1);
    NL_TEST_ASSERT(apSuite, gTestReadInteraction.mNumActiveSubscriptions == 1);

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();

    NL_TEST_ASSERT(apSuite, gTestReadInteraction.mNumActiveSubscriptions == 0);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    app::InteractionModelEngine::GetInstance()->UnregisterReadHandlerAppCallback();
    gTestReadInteraction.mAlterSubscriptionIntervals = false;
}

void TestReadInteraction::TestReadHandler_MultipleReads(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    static_assert(CHIP_IM_MAX_REPORTS_IN_FLIGHT <= CHIP_IM_MAX_NUM_READ_HANDLER,
                  "How can we have more reports in flight than read handlers?");

    MultipleReadHelper(apSuite, ctx, CHIP_IM_MAX_REPORTS_IN_FLIGHT);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();
}

void TestReadInteraction::TestReadHandler_OneSubscribeMultipleReads(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    static_assert(CHIP_IM_MAX_REPORTS_IN_FLIGHT <= CHIP_IM_MAX_NUM_READ_HANDLER,
                  "How can we have more reports in flight than read handlers?");
    static_assert(CHIP_IM_MAX_REPORTS_IN_FLIGHT > 1, "We won't do any reads");

    SubscribeThenReadHelper(apSuite, ctx, 1, CHIP_IM_MAX_REPORTS_IN_FLIGHT - 1);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();
}

void TestReadInteraction::TestReadHandler_TwoSubscribesMultipleReads(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);

    static_assert(CHIP_IM_MAX_REPORTS_IN_FLIGHT <= CHIP_IM_MAX_NUM_READ_HANDLER,
                  "How can we have more reports in flight than read handlers?");
    static_assert(CHIP_IM_MAX_REPORTS_IN_FLIGHT > 2, "We won't do any reads");

    SubscribeThenReadHelper(apSuite, ctx, 2, CHIP_IM_MAX_REPORTS_IN_FLIGHT - 2);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();
}

void TestReadInteraction::SubscribeThenReadHelper(nlTestSuite * apSuite, TestContext & aCtx, size_t aSubscribeCount,
                                                  size_t aReadCount)
{
    auto sessionHandle                       = aCtx.GetSessionBobToAlice();
    uint32_t numSuccessCalls                 = 0;
    uint32_t numSubscriptionEstablishedCalls = 0;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&numSuccessCalls](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        numSuccessCalls++;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&apSuite](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        //
        // We shouldn't be encountering any failures in this test.
        //
        NL_TEST_ASSERT(apSuite, false);
    };

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls, &apSuite, &aCtx, aSubscribeCount,
                                        aReadCount](const app::ReadClient & readClient) {
        numSubscriptionEstablishedCalls++;
        if (numSubscriptionEstablishedCalls == aSubscribeCount)
        {
            MultipleReadHelper(apSuite, aCtx, aReadCount);
        }
    };

    for (size_t i = 0; i < aSubscribeCount; ++i)
    {
        NL_TEST_ASSERT(apSuite,
                       Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                           &aCtx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 10,
                           onSubscriptionEstablishedCb, false, true) == CHIP_NO_ERROR);
    }

    aCtx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, numSuccessCalls == aSubscribeCount);
    NL_TEST_ASSERT(apSuite, numSubscriptionEstablishedCalls == aSubscribeCount);
}

void TestReadInteraction::MultipleReadHelper(nlTestSuite * apSuite, TestContext & aCtx, size_t aReadCount)
{
    auto sessionHandle       = aCtx.GetSessionBobToAlice();
    uint32_t numSuccessCalls = 0;
    uint32_t numFailureCalls = 0;

    responseDirective = kSendDataResponse;

    uint16_t firstExpectedResponse = totalReadCount + 1;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&apSuite, &numFailureCalls](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        numFailureCalls++;

        NL_TEST_ASSERT(apSuite, attributePath == nullptr);
    };

    for (size_t i = 0; i < aReadCount; ++i)
    {
        // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise,
        // it's not safe to do so.
        auto onSuccessCb = [&numSuccessCalls, &apSuite, firstExpectedResponse,
                            i](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
            NL_TEST_ASSERT(apSuite, dataResponse == firstExpectedResponse + i);
            numSuccessCalls++;
        };

        NL_TEST_ASSERT(apSuite,
                       Controller::ReadAttribute<TestCluster::Attributes::Int16u::TypeInfo>(
                           &aCtx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb) == CHIP_NO_ERROR);
    }

    aCtx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, numSuccessCalls == aReadCount);
    NL_TEST_ASSERT(apSuite, numFailureCalls == 0);
}

void TestReadInteraction::TestReadHandler_MultipleSubscriptionsWithDataVersionFilter(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                        = *static_cast<TestContext *>(apContext);
    auto sessionHandle                       = ctx.GetSessionBobToAlice();
    uint32_t numSuccessCalls                 = 0;
    uint32_t numSubscriptionEstablishedCalls = 0;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &numSuccessCalls](const app::ConcreteDataAttributePath & attributePath,
                                                   const auto & dataResponse) {
        NL_TEST_ASSERT(apSuite, attributePath.mDataVersion.HasValue() && attributePath.mDataVersion.Value() == kDataVersion);
        numSuccessCalls++;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&apSuite](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        //
        // We shouldn't be encountering any failures in this test.
        //
        NL_TEST_ASSERT(apSuite, false);
    };

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls](const app::ReadClient & readClient) {
        numSubscriptionEstablishedCalls++;
    };

    //
    // Try to issue parallel subscriptions that will exceed the value for CHIP_IM_MAX_NUM_READ_HANDLER.
    // If heap allocation is correctly setup, this should result in it successfully servicing more than the number
    // present in that define.
    //
    chip::Optional<chip::DataVersion> dataVersion(1);
    for (int i = 0; i < (CHIP_IM_MAX_NUM_READ_HANDLER + 1); i++)
    {
        NL_TEST_ASSERT(apSuite,
                       Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                           &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 10,
                           onSubscriptionEstablishedCb, false, true, dataVersion) == CHIP_NO_ERROR);
    }

    // There are too many messages and the test (gcc_debug, which includes many sanity checks) will be quite slow. Note: report
    // engine is using ScheduleWork which cannot be handled by DrainAndServiceIO correctly.
    ctx.GetIOContext().DriveIOUntil(System::Clock::Seconds16(30), [&]() {
        return numSubscriptionEstablishedCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1) &&
            numSuccessCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1);
    });

    ChipLogError(Zcl, "Success call cnt: %u (expect %u) subscription cnt: %u (expect %u)", numSuccessCalls,
                 uint32_t(CHIP_IM_MAX_NUM_READ_HANDLER + 1), numSubscriptionEstablishedCalls,
                 uint32_t(CHIP_IM_MAX_NUM_READ_HANDLER + 1));

    NL_TEST_ASSERT(apSuite, numSuccessCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));
    NL_TEST_ASSERT(apSuite, numSubscriptionEstablishedCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadHandlerResourceExhaustion_MultipleSubscriptions(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                        = *static_cast<TestContext *>(apContext);
    auto sessionHandle                       = ctx.GetSessionBobToAlice();
    uint32_t numSuccessCalls                 = 0;
    uint32_t numFailureCalls                 = 0;
    uint32_t numSubscriptionEstablishedCalls = 0;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&numSuccessCalls](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        numSuccessCalls++;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&apSuite, &numFailureCalls](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        numFailureCalls++;

        NL_TEST_ASSERT(apSuite, aError == CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
        NL_TEST_ASSERT(apSuite, attributePath == nullptr);
    };

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls](const app::ReadClient & readClient) {
        numSubscriptionEstablishedCalls++;
    };

    //
    // Artifically limit the capacity to 2 ReadHandlers. This will also validate reservation of handlers for Reads,
    // since the second subscription below should fail correctly.
    //
    app::InteractionModelEngine::GetInstance()->SetHandlerCapacity(2);
    NL_TEST_ASSERT(apSuite,
                   Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                       &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 10,
                       onSubscriptionEstablishedCb, false, true) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite,
                   Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                       &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 10,
                       onSubscriptionEstablishedCb, false, true) == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, numSuccessCalls == 1);
    NL_TEST_ASSERT(apSuite, numSubscriptionEstablishedCalls == 1);
    // Resubscription is happening for second subscribe call
    NL_TEST_ASSERT(apSuite, numFailureCalls == 0);

    app::InteractionModelEngine::GetInstance()->SetHandlerCapacity(-1);
    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadHandlerResourceExhaustion_MultipleReads(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx        = *static_cast<TestContext *>(apContext);
    auto sessionHandle       = ctx.GetSessionBobToAlice();
    uint32_t numSuccessCalls = 0;
    uint32_t numFailureCalls = 0;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [&numSuccessCalls](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        numSuccessCalls++;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&apSuite, &numFailureCalls](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        numFailureCalls++;

        NL_TEST_ASSERT(apSuite, aError == CHIP_IM_GLOBAL_STATUS(ResourceExhausted));
        NL_TEST_ASSERT(apSuite, attributePath == nullptr);
    };

    app::InteractionModelEngine::GetInstance()->SetHandlerCapacity(0);

    NL_TEST_ASSERT(apSuite,
                   Controller::ReadAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                       &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb) == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();

    app::InteractionModelEngine::GetInstance()->SetHandlerCapacity(-1);
    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();

    NL_TEST_ASSERT(apSuite, numSuccessCalls == 0);
    NL_TEST_ASSERT(apSuite, numFailureCalls == 1);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadFabricScopedWithoutFabricFilter(nlTestSuite * apSuite, void * apContext)
{
    /**
     *  TODO: we cannot implement the e2e read tests w/ fabric filter since the test session has only one session, and the
     * ReadSingleClusterData is not the one in real applications. We should be able to move some logic out of the ember library and
     * make it possible to have more fabrics in test setup so we can have a better test coverage.
     *
     *  NOTE: Based on the TODO above, the test is testing two separate logics:
     *   - When a fabric filtered read request is received, the server is able to pass the required fabric index to the response
     * encoder.
     *   - When a fabric filtered read request is received, the response encoder is able to encode the attribute correctly.
     */
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &onSuccessCbInvoked](const app::ConcreteDataAttributePath & attributePath,
                                                      const auto & dataResponse) {
        size_t len = 0;

        NL_TEST_ASSERT(apSuite, dataResponse.ComputeSize(&len) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, len > 1);

        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    Controller::ReadAttribute<TestCluster::Attributes::ListFabricScoped::TypeInfo>(
        &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, false /* fabric filtered */);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

void TestReadInteraction::TestReadFabricScopedWithFabricFilter(nlTestSuite * apSuite, void * apContext)
{
    /**
     *  TODO: we cannot implement the e2e read tests w/ fabric filter since the test session has only one session, and the
     * ReadSingleClusterData is not the one in real applications. We should be able to move some logic out of the ember library and
     * make it possible to have more fabrics in test setup so we can have a better test coverage.
     *
     *  NOTE: Based on the TODO above, the test is testing two separate logics:
     *   - When a fabric filtered read request is received, the server is able to pass the required fabric index to the response
     * encoder.
     *   - When a fabric filtered read request is received, the response encoder is able to encode the attribute correctly.
     */
    TestContext & ctx       = *static_cast<TestContext *>(apContext);
    auto sessionHandle      = ctx.GetSessionBobToAlice();
    bool onSuccessCbInvoked = false, onFailureCbInvoked = false;

    responseDirective = kSendDataResponse;

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onSuccessCb = [apSuite, &onSuccessCbInvoked](const app::ConcreteDataAttributePath & attributePath,
                                                      const auto & dataResponse) {
        size_t len = 0;

        NL_TEST_ASSERT(apSuite, dataResponse.ComputeSize(&len) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(apSuite, len == 1);

        // TODO: Uncomment the following code after we have fabric support in unit tests.
        /*
        auto iter = dataResponse.begin();
        if (iter.Next())
        {
            auto & item = iter.GetValue();
            NL_TEST_ASSERT(apSuite, item.fabricIndex == 1);
        }
        */
        onSuccessCbInvoked = true;
    };

    // Passing of stack variables by reference is only safe because of synchronous completion of the interaction. Otherwise, it's
    // not safe to do so.
    auto onFailureCb = [&onFailureCbInvoked](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
        onFailureCbInvoked = true;
    };

    Controller::ReadAttribute<TestCluster::Attributes::ListFabricScoped::TypeInfo>(
        &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, true /* fabric filtered */);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
}

namespace SubscriptionPathQuotaHelpers {
class TestReadCallback : public app::ReadClient::Callback
{
public:
    TestReadCallback() {}
    void OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                         const app::StatusIB & aStatus) override
    {
        if (apData != nullptr)
        {
            mAttributeCount++;
        }
    }

    void OnDone() override { mOnDone++; }

    void OnReportEnd() override { mOnReportEnd++; }

    void OnError(CHIP_ERROR aError) override
    {
        mOnError++;
        mLastError = aError;
    }

    void OnSubscriptionEstablished(uint64_t aSubscriptionId) override { mOnSubscriptionEstablishedCount++; }

    void ClearCounters()
    {
        mAttributeCount                 = 0;
        mOnReportEnd                    = 0;
        mOnSubscriptionEstablishedCount = 0;
        mOnDone                         = 0;
        mOnError                        = 0;
        mLastError                      = CHIP_NO_ERROR;
    }

    int32_t mAttributeCount                 = 0;
    int32_t mOnReportEnd                    = 0;
    int32_t mOnSubscriptionEstablishedCount = 0;
    int32_t mOnDone                         = 0;
    int32_t mOnError                        = 0;
    CHIP_ERROR mLastError                   = CHIP_NO_ERROR;
};

void EstablishSubscriptions(nlTestSuite * apSuite, void * apContext, int32_t numSubs, int32_t pathPerSub,
                            app::ReadClient::Callback * callback, std::vector<std::unique_ptr<app::ReadClient>> & readClients)
{
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();
    std::vector<app::AttributePathParams> attributePaths(
        pathPerSub, app::AttributePathParams(kTestEndpointId, TestCluster::Id, TestCluster::Attributes::Int16u::Id));

    app::ReadPrepareParams readParams(sessionHandle);
    readParams.mpAttributePathParamsList    = attributePaths.data();
    readParams.mAttributePathParamsListSize = pathPerSub;
    readParams.mMaxIntervalCeilingSeconds   = 1;
    readParams.mKeepSubscriptions           = true;

    for (int32_t i = 0; i < numSubs; i++)
    {
        std::unique_ptr<app::ReadClient> readClient =
            std::make_unique<app::ReadClient>(app::InteractionModelEngine::GetInstance(), &ctx.GetExchangeManager(), *callback,
                                              app::ReadClient::InteractionType::Subscribe);
        NL_TEST_ASSERT(apSuite, readClient->SendRequest(readParams) == CHIP_NO_ERROR);
        readClients.push_back(std::move(readClient));
    }
}

} // namespace SubscriptionPathQuotaHelpers

// TODO: (#17381) Need to add the case with more than one fabrics.
void TestReadInteraction::TestReadHandler_KillOverQuotaSubscriptions(nlTestSuite * apSuite, void * apContext)
{
    using namespace SubscriptionPathQuotaHelpers;
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();

    const int32_t kExpectedParallelSubs =
        app::InteractionModelEngine::kMinSupportedSubscriptionPerFabric * ctx.GetFabricTable().FabricCount();
    const int32_t kExpectedParallelPaths = kExpectedParallelSubs * app::InteractionModelEngine::kMinSupportedPathPerSubscription;

    app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&gTestReadInteraction);

    TestReadCallback readCallback;
    std::vector<std::unique_ptr<app::ReadClient>> readClients;

    // Intentially establish subscriptions using exceeded resources.
    app::InteractionModelEngine::GetInstance()->SetForceHandlerQuota(false);
    EstablishSubscriptions(apSuite, apContext, kExpectedParallelSubs,
                           app::InteractionModelEngine::kMinSupportedPathPerSubscription + 1, &readCallback, readClients);

    // There are too many messages and the test (gcc_debug, which includes many sanity checks) will be quite slow. Note: report
    // engine is using ScheduleWork which cannot be handled by DrainAndServiceIO correctly.
    ctx.GetIOContext().DriveIOUntil(System::Clock::Seconds16(60), [&]() {
        return readCallback.mOnSubscriptionEstablishedCount == static_cast<size_t>(CHIP_IM_MAX_NUM_READ_HANDLER);
    });

    NL_TEST_ASSERT(apSuite,
                   readCallback.mAttributeCount ==
                       kExpectedParallelSubs *
                           static_cast<int32_t>(app::InteractionModelEngine::kMinSupportedPathPerSubscription + 1));
    NL_TEST_ASSERT(apSuite, readCallback.mOnSubscriptionEstablishedCount == kExpectedParallelSubs);

    // We have set up the environment for testing the evicting logic.
    app::InteractionModelEngine::GetInstance()->SetForceHandlerQuota(true);
    app::InteractionModelEngine::GetInstance()->SetHandlerCapacityForSubscriptions(kExpectedParallelSubs);
    app::InteractionModelEngine::GetInstance()->SetPathPoolCapacityForSubscriptions(kExpectedParallelPaths);

    // The following check will trigger the logic in im to kill the read handlers that uses more paths than the limit per fabric.
    {
        TestReadCallback callback;
        std::vector<std::unique_ptr<app::ReadClient>> outReadClient;
        EstablishSubscriptions(apSuite, apContext, 1, app::InteractionModelEngine::kMinSupportedPathPerSubscription + 1, &callback,
                               outReadClient);

        ctx.DrainAndServiceIO();

        // Over-sized request after used all paths will receive Paths Exhausted status code.
        NL_TEST_ASSERT(apSuite, callback.mOnError == 1);
        NL_TEST_ASSERT(apSuite, callback.mLastError == CHIP_IM_GLOBAL_STATUS(PathsExhausted));
    }

    // The following check will trigger the logic in im to kill the read handlers that uses more paths than the limit per fabric.
    {
        TestReadCallback callback;
        std::vector<std::unique_ptr<app::ReadClient>> outReadClient;
        EstablishSubscriptions(apSuite, apContext, 1, app::InteractionModelEngine::kMinSupportedPathPerSubscription, &callback,
                               outReadClient);

        ctx.DrainAndServiceIO();

        // This read handler should evict some existing subscriptions for enough space
        NL_TEST_ASSERT(apSuite, callback.mOnSubscriptionEstablishedCount == 1);
        NL_TEST_ASSERT(apSuite, callback.mAttributeCount == app::InteractionModelEngine::kMinSupportedPathPerSubscription);
    }

    {
        app::AttributePathParams path;
        path.mEndpointId  = kTestEndpointId;
        path.mClusterId   = TestCluster::Id;
        path.mAttributeId = TestCluster::Attributes::Int16u::Id;
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
    }
    readCallback.ClearCounters();
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount <= kExpectedParallelPaths);
    NL_TEST_ASSERT(apSuite,
                   app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() <=
                       static_cast<size_t>(kExpectedParallelSubs));

    // Shutdown all clients
    readClients.clear();

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    app::InteractionModelEngine::GetInstance()->SetForceHandlerQuota(false);
    app::InteractionModelEngine::GetInstance()->SetHandlerCapacityForSubscriptions(-1);
    app::InteractionModelEngine::GetInstance()->SetPathPoolCapacityForSubscriptions(-1);
}

void TestReadInteraction::TestReadHandler_KillOldestSubscriptions(nlTestSuite * apSuite, void * apContext)
{
    using namespace SubscriptionPathQuotaHelpers;
    TestContext & ctx  = *static_cast<TestContext *>(apContext);
    auto sessionHandle = ctx.GetSessionBobToAlice();

    const int32_t kExpectedParallelSubs =
        app::InteractionModelEngine::kMinSupportedSubscriptionPerFabric * ctx.GetFabricTable().FabricCount();
    const int32_t kExpectedParallelPaths = kExpectedParallelSubs * app::InteractionModelEngine::kMinSupportedPathPerSubscription;

    app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&gTestReadInteraction);

    TestReadCallback readCallback;
    std::vector<std::unique_ptr<app::ReadClient>> readClients;

    app::InteractionModelEngine::GetInstance()->SetForceHandlerQuota(true);
    app::InteractionModelEngine::GetInstance()->SetHandlerCapacityForSubscriptions(kExpectedParallelSubs);
    app::InteractionModelEngine::GetInstance()->SetPathPoolCapacityForSubscriptions(kExpectedParallelPaths);

    // This should just use all availbale resources.
    EstablishSubscriptions(apSuite, apContext, kExpectedParallelSubs, app::InteractionModelEngine::kMinSupportedPathPerSubscription,
                           &readCallback, readClients);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite,
                   readCallback.mAttributeCount ==
                       kExpectedParallelSubs * static_cast<int32_t>(app::InteractionModelEngine::kMinSupportedPathPerSubscription));
    NL_TEST_ASSERT(apSuite, readCallback.mOnSubscriptionEstablishedCount == kExpectedParallelSubs);
    NL_TEST_ASSERT(apSuite,
                   app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() ==
                       static_cast<size_t>(kExpectedParallelSubs));

    // The following check will trigger the logic in im to kill the read handlers that uses more paths than the limit per fabric.
    {
        TestReadCallback callback;
        std::vector<std::unique_ptr<app::ReadClient>> outReadClient;
        EstablishSubscriptions(apSuite, apContext, 1, app::InteractionModelEngine::kMinSupportedPathPerSubscription + 1, &callback,
                               outReadClient);

        ctx.DrainAndServiceIO();

        // Over-sized request after used all paths will receive Paths Exhausted status code.
        NL_TEST_ASSERT(apSuite, callback.mOnError == 1);
        NL_TEST_ASSERT(apSuite, callback.mLastError == CHIP_IM_GLOBAL_STATUS(PathsExhausted));
    }

    // The following check will trigger the logic in im to kill the read handlers that uses more paths than the limit per fabric.
    {
        TestReadCallback callback;
        std::vector<std::unique_ptr<app::ReadClient>> outReadClient;
        EstablishSubscriptions(apSuite, apContext, 1, app::InteractionModelEngine::kMinSupportedPathPerSubscription, &callback,
                               outReadClient);

        ctx.DrainAndServiceIO();

        // This read handler should evict some existing subscriptions for enough space
        NL_TEST_ASSERT(apSuite, callback.mOnSubscriptionEstablishedCount == 1);
        NL_TEST_ASSERT(apSuite, callback.mAttributeCount == app::InteractionModelEngine::kMinSupportedPathPerSubscription);
    }

    {
        app::AttributePathParams path;
        path.mEndpointId  = kTestEndpointId;
        path.mClusterId   = TestCluster::Id;
        path.mAttributeId = TestCluster::Attributes::Int16u::Id;
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
    }
    readCallback.ClearCounters();
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, readCallback.mAttributeCount <= kExpectedParallelPaths);
    NL_TEST_ASSERT(apSuite,
                   app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() ==
                       static_cast<size_t>(kExpectedParallelSubs));

    // Shutdown all clients
    readClients.clear();

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    app::InteractionModelEngine::GetInstance()->SetForceHandlerQuota(false);
    app::InteractionModelEngine::GetInstance()->SetHandlerCapacityForSubscriptions(-1);
    app::InteractionModelEngine::GetInstance()->SetPathPoolCapacityForSubscriptions(-1);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestReadAttributeResponse", TestReadInteraction::TestReadAttributeResponse),
    NL_TEST_DEF("TestReadDataVersionFilter", TestReadInteraction::TestReadDataVersionFilter),
    NL_TEST_DEF("TestReadEventResponse", TestReadInteraction::TestReadEventResponse),
    NL_TEST_DEF("TestReadAttributeError", TestReadInteraction::TestReadAttributeError),
    NL_TEST_DEF("TestReadFabricScopedWithoutFabricFilter", TestReadInteraction::TestReadFabricScopedWithoutFabricFilter),
    NL_TEST_DEF("TestReadFabricScopedWithFabricFilter", TestReadInteraction::TestReadFabricScopedWithFabricFilter),
    NL_TEST_DEF("TestReadHandler_MultipleSubscriptions", TestReadInteraction::TestReadHandler_MultipleSubscriptions),
    NL_TEST_DEF("TestReadHandler_SubscriptionAppRejection", TestReadInteraction::TestReadHandler_SubscriptionAppRejection),
    NL_TEST_DEF("TestReadHandler_MultipleSubscriptionsWithDataVersionFilter", TestReadInteraction::TestReadHandler_MultipleSubscriptionsWithDataVersionFilter),
    NL_TEST_DEF("TestReadHandler_MultipleReads", TestReadInteraction::TestReadHandler_MultipleReads),
    NL_TEST_DEF("TestReadHandler_OneSubscribeMultipleReads", TestReadInteraction::TestReadHandler_OneSubscribeMultipleReads),
    NL_TEST_DEF("TestReadHandler_TwoSubscribesMultipleReads", TestReadInteraction::TestReadHandler_TwoSubscribesMultipleReads),
    NL_TEST_DEF("TestReadHandlerResourceExhaustion_MultipleSubscriptions", TestReadInteraction::TestReadHandlerResourceExhaustion_MultipleSubscriptions),
    NL_TEST_DEF("TestReadHandlerResourceExhaustion_MultipleReads", TestReadInteraction::TestReadHandlerResourceExhaustion_MultipleReads),
    NL_TEST_DEF("TestReadAttributeTimeout", TestReadInteraction::TestReadAttributeTimeout),
    NL_TEST_DEF("TestReadHandler_SubscriptionAlteredReportingIntervals", TestReadInteraction::TestReadHandler_SubscriptionAlteredReportingIntervals),
    NL_TEST_DEF("TestReadHandler_KillOverQuotaSubscriptions", TestReadInteraction::TestReadHandler_KillOverQuotaSubscriptions),
    NL_TEST_DEF("TestReadHandler_KillOldestSubscriptions", TestReadInteraction::TestReadHandler_KillOldestSubscriptions),
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
