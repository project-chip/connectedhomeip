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
        else
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

            attributeData.DataVersion(kDataVersion);
            ReturnErrorOnFailure(attributeData.GetError());
            AttributePathIB::Builder & attributePath = attributeData.CreatePath();
            attributePath.Endpoint(aPath.mEndpointId)
                .Cluster(aPath.mClusterId)
                .Attribute(aPath.mAttributeId)
                .EndOfAttributePathIB();
            ReturnErrorOnFailure(attributePath.GetError());

            ReturnErrorOnFailure(DataModel::Encode(*(attributeData.GetWriter()),
                                                   TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)), value));
            ReturnErrorOnFailure(attributeData.EndOfAttributeDataIB().GetError());
            return attributeReport.EndOfAttributeReportIB().GetError();
        }
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

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredDataVersion)
{
    if (aRequiredDataVersion == kDataVersion)
    {
        return true;
    }
    else
    {
        return false;
    }
}
} // namespace app
} // namespace chip

namespace {

class TestReadInteraction
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
    static void TestReadHandler_MultipleSubscriptionsWithDataVersionFilter(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerResourceExhaustion_MultipleSubscriptions(nlTestSuite * apSuite, void * apContext);
    static void TestReadHandlerResourceExhaustion_MultipleReads(nlTestSuite * apSuite, void * apContext);

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
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 2);

    ctx.ExpireSessionBobToAlice();

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, !onSuccessCbInvoked && onFailureCbInvoked);

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 1);

    ctx.DrainAndServiceIO();
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    ctx.DrainAndServiceIO();

    ctx.ExpireSessionAliceToBob();

    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);

    //
    // Let's put back the sessions so that the next tests (which assume a valid initialized set of sessions)
    // can function correctly.
    //
    ctx.CreateSessionAliceToBob();
    ctx.CreateSessionBobToAlice();

    //
    // TODO: Figure out why I cannot enable this line below.
    //
    // NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
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

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls]() { numSubscriptionEstablishedCalls++; };

    //
    // Try to issue parallel subscriptions that will exceed the value for CHIP_IM_MAX_NUM_READ_HANDLER.
    // If heap allocation is correctly setup, this should result in it successfully servicing more than the number
    // present in that define.
    //
    for (int i = 0; i < (CHIP_IM_MAX_NUM_READ_HANDLER + 1); i++)
    {
        NL_TEST_ASSERT(apSuite,
                       Controller::SubscribeAttribute<TestCluster::Attributes::ListStructOctetString::TypeInfo>(
                           &ctx.GetExchangeManager(), sessionHandle, kTestEndpointId, onSuccessCb, onFailureCb, 0, 10,
                           onSubscriptionEstablishedCb, false, true) == CHIP_NO_ERROR);
    }

    //
    // It may take a couple of service calls since we may hit the limit of CHIP_IM_MAX_REPORTS_IN_FLIGHT
    // reports.
    //
    for (int i = 0; i < 10 && (numSubscriptionEstablishedCalls != (CHIP_IM_MAX_NUM_READ_HANDLER + 1)); i++)
    {
        ctx.DrainAndServiceIO();
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        ctx.DrainAndServiceIO();
    }

    NL_TEST_ASSERT(apSuite, numSuccessCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));
    NL_TEST_ASSERT(apSuite, numSubscriptionEstablishedCalls == (CHIP_IM_MAX_NUM_READ_HANDLER + 1));

    app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();

    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
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

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls]() { numSubscriptionEstablishedCalls++; };

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

    //
    // It may take a couple of service calls since we may hit the limit of CHIP_IM_MAX_REPORTS_IN_FLIGHT
    // reports.
    //
    for (int i = 0; i < 10 && (numSubscriptionEstablishedCalls != (CHIP_IM_MAX_NUM_READ_HANDLER + 1)); i++)
    {
        ctx.DrainAndServiceIO();
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        ctx.DrainAndServiceIO();
    }

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

    auto onSubscriptionEstablishedCb = [&numSubscriptionEstablishedCalls]() { numSubscriptionEstablishedCalls++; };

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

    //
    // It may take a couple of service calls since we may hit the limit of CHIP_IM_MAX_REPORTS_IN_FLIGHT
    // reports.
    //
    for (int i = 0; i < 10; i++)
    {
        ctx.DrainAndServiceIO();
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        ctx.DrainAndServiceIO();
    }

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

    //
    // It may take a couple of service calls since we may hit the limit of CHIP_IM_MAX_REPORTS_IN_FLIGHT
    // reports.
    //
    for (int i = 0; i < 10; i++)
    {
        ctx.DrainAndServiceIO();
        app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
        ctx.DrainAndServiceIO();
    }

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
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
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
    app::InteractionModelEngine::GetInstance()->GetReportingEngine().Run();
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, onSuccessCbInvoked && !onFailureCbInvoked);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0);
    NL_TEST_ASSERT(apSuite, app::InteractionModelEngine::GetInstance()->GetNumActiveReadHandlers() == 0);
    NL_TEST_ASSERT(apSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
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
    NL_TEST_DEF("TestReadHandler_MultipleSubscriptionsWithDataVersionFilter", TestReadInteraction::TestReadHandler_MultipleSubscriptionsWithDataVersionFilter),
    NL_TEST_DEF("TestReadHandlerResourceExhaustion_MultipleSubscriptions", TestReadInteraction::TestReadHandlerResourceExhaustion_MultipleSubscriptions),
    NL_TEST_DEF("TestReadHandlerResourceExhaustion_MultipleReads", TestReadInteraction::TestReadHandlerResourceExhaustion_MultipleReads),
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
