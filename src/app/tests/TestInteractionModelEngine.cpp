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

/**
 *    @file
 *      This file implements unit tests for CHIP Interaction Model Engine
 *
 */

#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/icd/server/ICDServerConfig.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#include <app/SimpleSubscriptionResumptionStorage.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

#include <nlunit-test.h>

namespace {

using TestContext = chip::Test::AppContext;

class NullReadHandlerCallback : public chip::app::ReadHandler::ManagementCallback
{
public:
    void OnDone(chip::app::ReadHandler & apReadHandlerObj) override {}
    chip::app::ReadHandler::ApplicationCallback * GetAppCallback() override { return nullptr; }
    chip::app::InteractionModelEngine * GetInteractionModelEngine() override
    {
        return chip::app::InteractionModelEngine::GetInstance();
    }
};

} // namespace

namespace chip {
namespace app {
class TestInteractionModelEngine
{
public:
    static void TestAttributePathParamsPushRelease(nlTestSuite * apSuite, void * apContext);
    static void TestRemoveDuplicateConcreteAttribute(nlTestSuite * apSuite, void * apContext);
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    static void TestSubjectHasPersistedSubscription(nlTestSuite * apSuite, void * apContext);
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    static void TestSubscriptionResumptionTimer(nlTestSuite * apSuite, void * apContext);
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    static int GetAttributePathListLength(SingleLinkedListNode<AttributePathParams> * apattributePathParamsList);
    static void TestSubjectHasActiveSubscriptionSingleSubOneEntry(nlTestSuite * apSuite, void * apContext);
    static void TestSubjectHasActiveSubscriptionSingleSubMultipleEntries(nlTestSuite * apSuite, void * apContext);
    static void TestSubjectHasActiveSubscriptionMultipleSubsSingleEntry(nlTestSuite * apSuite, void * apContext);
    static void TestSubjectHasActiveSubscriptionMultipleSubsMultipleEntries(nlTestSuite * apSuite, void * apContext);
    static void TestSubjectHasActiveSubscriptionSubWithCAT(nlTestSuite * apSuite, void * apContext);
    static void TestDecrementNumSubscriptionsToResume(nlTestSuite * apSuite, void * apContext);
};

int TestInteractionModelEngine::GetAttributePathListLength(SingleLinkedListNode<AttributePathParams> * apAttributePathParamsList)
{
    int length                                         = 0;
    SingleLinkedListNode<AttributePathParams> * runner = apAttributePathParamsList;
    while (runner != nullptr)
    {
        runner = runner->mpNext;
        length++;
    }
    return length;
}

void TestInteractionModelEngine::TestAttributePathParamsPushRelease(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    err               = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(),
                                                                    app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    SingleLinkedListNode<AttributePathParams> * attributePathParamsList = nullptr;
    AttributePathParams attributePathParams1;
    AttributePathParams attributePathParams2;
    AttributePathParams attributePathParams3;

    attributePathParams1.mEndpointId = 1;
    attributePathParams2.mEndpointId = 2;
    attributePathParams3.mEndpointId = 3;

    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    NL_TEST_ASSERT(apSuite,
                   attributePathParamsList != nullptr &&
                       attributePathParams1.mEndpointId == attributePathParamsList->mValue.mEndpointId);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 1);

    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    NL_TEST_ASSERT(apSuite,
                   attributePathParamsList != nullptr &&
                       attributePathParams2.mEndpointId == attributePathParamsList->mValue.mEndpointId);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 2);

    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    NL_TEST_ASSERT(apSuite,
                   attributePathParamsList != nullptr &&
                       attributePathParams3.mEndpointId == attributePathParamsList->mValue.mEndpointId);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 3);

    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 0);
}

void TestInteractionModelEngine::TestRemoveDuplicateConcreteAttribute(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    err               = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(),
                                                                    app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    SingleLinkedListNode<AttributePathParams> * attributePathParamsList = nullptr;
    AttributePathParams attributePathParams1;
    AttributePathParams attributePathParams2;
    AttributePathParams attributePathParams3;

    // Three concrete paths, no duplicates
    attributePathParams1.mEndpointId  = Test::kMockEndpoint3;
    attributePathParams1.mClusterId   = Test::MockClusterId(2);
    attributePathParams1.mAttributeId = Test::MockAttributeId(1);

    attributePathParams2.mEndpointId  = Test::kMockEndpoint3;
    attributePathParams2.mClusterId   = Test::MockClusterId(2);
    attributePathParams2.mAttributeId = Test::MockAttributeId(2);

    attributePathParams3.mEndpointId  = Test::kMockEndpoint3;
    attributePathParams3.mClusterId   = Test::MockClusterId(2);
    attributePathParams3.mAttributeId = Test::MockAttributeId(3);

    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 3);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    attributePathParams1.mEndpointId  = kInvalidEndpointId;
    attributePathParams1.mClusterId   = kInvalidClusterId;
    attributePathParams1.mAttributeId = kInvalidAttributeId;

    attributePathParams2.mEndpointId  = Test::kMockEndpoint3;
    attributePathParams2.mClusterId   = Test::MockClusterId(2);
    attributePathParams2.mAttributeId = Test::MockAttributeId(2);

    attributePathParams3.mEndpointId  = Test::kMockEndpoint3;
    attributePathParams3.mClusterId   = Test::MockClusterId(2);
    attributePathParams3.mAttributeId = Test::MockAttributeId(3);

    // 1st path is wildcard endpoint, 2nd, 3rd paths are concrete paths, the concrete ones would be removed.
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 1);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    // 2nd path is wildcard endpoint, 1st, 3rd paths are concrete paths, the latter two would be removed.
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 1);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    // 3nd path is wildcard endpoint, 1st, 2nd paths are concrete paths, the latter two would be removed.
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 1);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    attributePathParams1.mEndpointId  = Test::kMockEndpoint3;
    attributePathParams1.mClusterId   = Test::MockClusterId(2);
    attributePathParams1.mAttributeId = kInvalidAttributeId;

    attributePathParams2.mEndpointId  = Test::kMockEndpoint2;
    attributePathParams2.mClusterId   = Test::MockClusterId(2);
    attributePathParams2.mAttributeId = Test::MockAttributeId(2);

    attributePathParams3.mEndpointId  = Test::kMockEndpoint2;
    attributePathParams3.mClusterId   = Test::MockClusterId(2);
    attributePathParams3.mAttributeId = Test::MockAttributeId(3);

    // 1st is wildcard one, but not intersect with the latter two concrete paths, so the paths in total are 3 finally
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 3);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    attributePathParams1.mEndpointId  = kInvalidEndpointId;
    attributePathParams1.mClusterId   = kInvalidClusterId;
    attributePathParams1.mAttributeId = kInvalidAttributeId;

    attributePathParams2.mEndpointId  = Test::kMockEndpoint3;
    attributePathParams2.mClusterId   = kInvalidClusterId;
    attributePathParams2.mAttributeId = kInvalidAttributeId;

    attributePathParams3.mEndpointId  = kInvalidEndpointId;
    attributePathParams3.mClusterId   = kInvalidClusterId;
    attributePathParams3.mAttributeId = Test::MockAttributeId(3);

    // Wildcards cannot be deduplicated.
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 3);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    attributePathParams1.mEndpointId  = kInvalidEndpointId;
    attributePathParams1.mClusterId   = Test::MockClusterId(2);
    attributePathParams1.mAttributeId = Test::MockAttributeId(10);

    attributePathParams2.mEndpointId  = Test::kMockEndpoint3;
    attributePathParams2.mClusterId   = Test::MockClusterId(2);
    attributePathParams2.mAttributeId = Test::MockAttributeId(10);

    // 1st path is wildcard endpoint, 2nd path is invalid attribute
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    NL_TEST_ASSERT(apSuite, GetAttributePathListLength(attributePathParamsList) == 2);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);
}

/**
 * @brief Test verifies the SubjectHasActiveSubscription with a single subscription with a single entry
 */
void TestInteractionModelEngine::TestSubjectHasActiveSubscriptionSingleSubOneEntry(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    NullReadHandlerCallback nullCallback;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

    NodeId bobNodeId           = 0x12344321ull;
    FabricIndex bobFabricIndex = 1;

    // Create ExchangeContext
    Messaging::ExchangeContext * exchangeCtx1 = ctx.NewExchangeToBob(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx1);

    // InteractionModelEngine init
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), reporting::GetDefaultReportScheduler()));

    // Verify that there are no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Create and setup readHandler 1
    ReadHandler * readHandler1 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx1, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    // Verify that Bob still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Set readHandler1 to active
    readHandler1->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, true);

    // Verify that Bob still has an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();

    // Verify that there are no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);
}

/**
 * @brief Test verifies that the SubjectHasActiveSubscription will continue iterating till it fines at least one valid active
 * subscription
 */
void TestInteractionModelEngine::TestSubjectHasActiveSubscriptionSingleSubMultipleEntries(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    NullReadHandlerCallback nullCallback;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

    NodeId bobNodeId           = 0x12344321ull;
    FabricIndex bobFabricIndex = 1;

    // Create ExchangeContexts
    Messaging::ExchangeContext * exchangeCtx1 = ctx.NewExchangeToBob(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx1);

    Messaging::ExchangeContext * exchangeCtx2 = ctx.NewExchangeToBob(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx1);

    // InteractionModelEngine init
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), reporting::GetDefaultReportScheduler()));

    // Verify that both Alice and Bob have no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Create readHandler 1
    engine->GetReadHandlerPool().CreateObject(nullCallback, exchangeCtx1, ReadHandler::InteractionType::Subscribe,
                                              reporting::GetDefaultReportScheduler());

    // Verify that Bob still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Create and setup readHandler 2
    ReadHandler * readHandler2 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx2, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    // Verify that Bob still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Set readHandler2 to active
    readHandler2->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, true);

    // Verify that Bob still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Release active ReadHandler
    engine->GetReadHandlerPool().ReleaseObject(readHandler2);

    // Verify that there are no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();
}

/**
 * @brief Test validates that the SubjectHasActiveSubscription can support multiple subscriptions from different clients
 */
void TestInteractionModelEngine::TestSubjectHasActiveSubscriptionMultipleSubsSingleEntry(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    NullReadHandlerCallback nullCallback;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

    NodeId bobNodeId             = 0x12344321ull;
    FabricIndex bobFabricIndex   = 1;
    NodeId aliceNodeId           = 0x11223344ull;
    FabricIndex aliceFabricIndex = 2;

    // Create ExchangeContexts
    Messaging::ExchangeContext * exchangeCtx1 = ctx.NewExchangeToBob(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx1);

    Messaging::ExchangeContext * exchangeCtx2 = ctx.NewExchangeToAlice(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx2);

    // InteractionModelEngine init
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), reporting::GetDefaultReportScheduler()));

    // Verify that both Alice and Bob have no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId) == false);

    // Create and setup readHandler 1
    ReadHandler * readHandler1 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx1, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    // Create and setup readHandler 2
    ReadHandler * readHandler2 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx2, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    // Verify that Bob still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Set readHandler1 to active
    readHandler1->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, true);

    // Verify that Bob has an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId) == false);

    // Set readHandler2 to active
    readHandler2->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, true);

    // Verify that Bob has an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Set readHandler1 to inactive
    readHandler1->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, false);

    // Verify that Bob doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Verify that Alice still has an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();

    // Verify that both Alice and Bob have no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId) == false);
}

/**
 * @brief Test validates that the SubjectHasActiveSubscription can find the active subscription even if there are multiple
 * subscriptions for each client
 */
void TestInteractionModelEngine::TestSubjectHasActiveSubscriptionMultipleSubsMultipleEntries(nlTestSuite * apSuite,
                                                                                             void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    NullReadHandlerCallback nullCallback;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

    NodeId bobNodeId             = 0x12344321ull;
    FabricIndex bobFabricIndex   = 1;
    NodeId aliceNodeId           = 0x11223344ull;
    FabricIndex aliceFabricIndex = 2;

    // Create ExchangeContexts
    Messaging::ExchangeContext * exchangeCtx11 = ctx.NewExchangeToBob(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx11);

    Messaging::ExchangeContext * exchangeCtx12 = ctx.NewExchangeToBob(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx12);

    Messaging::ExchangeContext * exchangeCtx21 = ctx.NewExchangeToAlice(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx21);

    Messaging::ExchangeContext * exchangeCtx22 = ctx.NewExchangeToAlice(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx22);

    // InteractionModelEngine init
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), reporting::GetDefaultReportScheduler()));

    // Verify that both Alice and Bob have no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId) == false);

    // Create and setup readHandler 1-1
    engine->GetReadHandlerPool().CreateObject(nullCallback, exchangeCtx11, ReadHandler::InteractionType::Subscribe,
                                              reporting::GetDefaultReportScheduler());

    // Create and setup readHandler 1-2
    ReadHandler * readHandler12 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx12, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    // Create and setup readHandler 2-1
    engine->GetReadHandlerPool().CreateObject(nullCallback, exchangeCtx21, ReadHandler::InteractionType::Subscribe,
                                              reporting::GetDefaultReportScheduler());

    // Create and setup readHandler 2-2
    ReadHandler * readHandler22 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx22, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    // Verify that both Alice and Bob have no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId) == false);

    // Set readHandler 1-2 to active
    readHandler12->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, true);

    // Verify that Bob has an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId) == false);

    // Set readHandler 2-2 to active
    readHandler22->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, true);

    // Verify that Bob has an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice still doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Set readHandler1 to inactive
    readHandler12->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, false);

    // Verify that Bob doesn't have an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);

    // Verify that Alice still has an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();

    // Verify that both Alice and Bob have no active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId) == false);
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId) == false);
}

/**
 * @brief Verifies that SubjectHasActiveSubscription support CATs as a subject-id
 */
void TestInteractionModelEngine::TestSubjectHasActiveSubscriptionSubWithCAT(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx               = *reinterpret_cast<TestContext *>(apContext);
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();
    NullReadHandlerCallback nullCallback;

    CASEAuthTag cat            = 0x1111'0001;
    CASEAuthTag invalidCAT     = 0x1112'0001;
    CATValues cats             = CATValues{ { cat } };
    NodeId valideSubjectId     = NodeIdFromCASEAuthTag(cat);
    NodeId invalideSubjectId   = NodeIdFromCASEAuthTag(invalidCAT);
    FabricIndex bobFabricIndex = 1;

    // InteractionModelEngine init
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), reporting::GetDefaultReportScheduler()));

    // Make sure we are using CASE sessions, because there is no defunct-marking for PASE.
    ctx.ExpireSessionBobToAlice();
    ctx.ExpireSessionAliceToBob();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == ctx.CreateCASESessionBobToAlice(cats));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == ctx.CreateCASESessionAliceToBob(cats));

    // Create ExchangeContexts
    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToBob(nullptr, false);
    NL_TEST_ASSERT(apSuite, exchangeCtx);

    // Create readHandler
    ReadHandler * readHandler = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    // Verify there are not active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, valideSubjectId) == false);
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, invalideSubjectId) == false);

    // Set readHandler to active
    readHandler->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription, true);

    // Verify tthat valid subjectID has an active subscription
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, valideSubjectId));
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, invalideSubjectId) == false);

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();

    // Verify there are not active subscriptions
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, valideSubjectId) == false);
    NL_TEST_ASSERT(apSuite, engine->SubjectHasActiveSubscription(bobFabricIndex, invalideSubjectId) == false);
}

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

/**
 * @brief Test verifies the SubjectHasPersistedSubscription with single and multiple persisted subscriptions.
 */
void TestInteractionModelEngine::TestSubjectHasPersistedSubscription(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    chip::TestPersistentStorageDelegate storage;
    chip::app::SimpleSubscriptionResumptionStorage subscriptionStorage;

    err = subscriptionStorage.Init(&storage);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(),
                                                      app::reporting::GetDefaultReportScheduler(), nullptr, &subscriptionStorage);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NodeId nodeId1      = 1;
    FabricIndex fabric1 = 1;
    SubscriptionId sub1 = 1;
    NodeId nodeId2      = 2;
    FabricIndex fabric2 = 2;
    SubscriptionId sub2 = 2;

    SubscriptionResumptionStorage::SubscriptionInfo info1 = { .mNodeId         = nodeId1,
                                                              .mFabricIndex    = fabric1,
                                                              .mSubscriptionId = sub1 };
    SubscriptionResumptionStorage::SubscriptionInfo info2 = { .mNodeId         = nodeId2,
                                                              .mFabricIndex    = fabric2,
                                                              .mSubscriptionId = sub2 };

    // Test with no persisted subscriptions - Should return false
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric1, nodeId1) == false);

    // Add one entry
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == subscriptionStorage.Save(info1));

    // Verify that entry matches - Should return true
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric1, nodeId1));

    // Test with absent subscription - Should return false
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric2, nodeId2) == false);

    // Add second entry
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == subscriptionStorage.Save(info2));

    // Verify that entry matches - Should return true
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric2, nodeId2));
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric1, nodeId1));

    // Remove an entry
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == subscriptionStorage.Delete(nodeId1, fabric1, sub1));

    // Test with absent subscription - Should return false
    NL_TEST_ASSERT(apSuite, InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric1, nodeId1) == false);

    // Clean Up entries
    subscriptionStorage.DeleteAll(fabric1);
    subscriptionStorage.DeleteAll(fabric2);
}

#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

void TestInteractionModelEngine::TestSubscriptionResumptionTimer(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    err               = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(),
                                                                    app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    uint32_t timeTillNextResubscriptionMs;
    InteractionModelEngine::GetInstance()->mNumSubscriptionResumptionRetries = 0;
    timeTillNextResubscriptionMs = InteractionModelEngine::GetInstance()->ComputeTimeSecondsTillNextSubscriptionResumption();
    NL_TEST_ASSERT(apSuite, timeTillNextResubscriptionMs == CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MIN_RETRY_INTERVAL_SECS);

    uint32_t lastTimeTillNextResubscriptionMs = timeTillNextResubscriptionMs;
    for (InteractionModelEngine::GetInstance()->mNumSubscriptionResumptionRetries = 1;
         InteractionModelEngine::GetInstance()->mNumSubscriptionResumptionRetries <=
         CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_FIBONACCI_STEP_INDEX;
         InteractionModelEngine::GetInstance()->mNumSubscriptionResumptionRetries++)
    {
        timeTillNextResubscriptionMs = InteractionModelEngine::GetInstance()->ComputeTimeSecondsTillNextSubscriptionResumption();
        NL_TEST_ASSERT(apSuite, timeTillNextResubscriptionMs >= lastTimeTillNextResubscriptionMs);
        NL_TEST_ASSERT(apSuite, timeTillNextResubscriptionMs < CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS);
        lastTimeTillNextResubscriptionMs = timeTillNextResubscriptionMs;
    }

    InteractionModelEngine::GetInstance()->mNumSubscriptionResumptionRetries = 2000;
    timeTillNextResubscriptionMs = InteractionModelEngine::GetInstance()->ComputeTimeSecondsTillNextSubscriptionResumption();
    NL_TEST_ASSERT(apSuite, timeTillNextResubscriptionMs == CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS);
}

#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

void TestInteractionModelEngine::TestDecrementNumSubscriptionsToResume(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx                       = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    InteractionModelEngine * engine         = InteractionModelEngine::GetInstance();
    constexpr uint8_t kNumberOfSubsToResume = 5;
    uint8_t numberOfSubsRemaining           = kNumberOfSubsToResume;

    err = engine->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable(), app::reporting::GetDefaultReportScheduler());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    ICDManager manager;
    engine->SetICDManager(&manager);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

    // Set number of subs
    engine->mNumOfSubscriptionToResume = kNumberOfSubsToResume;

#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    // Verify mIsBootUpResumeSubscriptionExecuted has not been set
    NL_TEST_ASSERT(apSuite, !manager.GetIsBootUpResumeSubscriptionExecuted());
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

    // Decrease number of subs by 1
    numberOfSubsRemaining--;
    engine->DecrementNumSubscriptionsToResume();
    NL_TEST_ASSERT_EQUALS(apSuite, numberOfSubsRemaining, engine->mNumOfSubscriptionToResume);

    // Decrease to 0 subs remaining
    while (numberOfSubsRemaining > 0)
    {
#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
        // Verify mIsBootUpResumeSubscriptionExecuted has not been set
        NL_TEST_ASSERT(apSuite, !manager.GetIsBootUpResumeSubscriptionExecuted());
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

        numberOfSubsRemaining--;
        engine->DecrementNumSubscriptionsToResume();
        NL_TEST_ASSERT_EQUALS(apSuite, numberOfSubsRemaining, engine->mNumOfSubscriptionToResume);
    }

#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    // Verify mIsBootUpResumeSubscriptionExecuted has been set
    NL_TEST_ASSERT(apSuite, manager.GetIsBootUpResumeSubscriptionExecuted());
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

    // Make sure we don't rollover / go negative
    engine->DecrementNumSubscriptionsToResume();
    NL_TEST_ASSERT_EQUALS(apSuite, numberOfSubsRemaining, engine->mNumOfSubscriptionToResume);

    // Clean up
#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    engine->SetICDManager(nullptr);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
}
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

} // namespace app
} // namespace chip

namespace {

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("TestAttributePathParamsPushRelease", chip::app::TestInteractionModelEngine::TestAttributePathParamsPushRelease),
                NL_TEST_DEF("TestRemoveDuplicateConcreteAttribute", chip::app::TestInteractionModelEngine::TestRemoveDuplicateConcreteAttribute),
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
                NL_TEST_DEF("TestSubjectHasPersistedSubscription", chip::app::TestInteractionModelEngine::TestSubjectHasPersistedSubscription),
                NL_TEST_DEF("TestDecrementNumSubscriptionsToResume", chip::app::TestInteractionModelEngine::TestDecrementNumSubscriptionsToResume),
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
                NL_TEST_DEF("TestSubscriptionResumptionTimer", chip::app::TestInteractionModelEngine::TestSubscriptionResumptionTimer),
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
                NL_TEST_DEF("TestSubjectHasActiveSubscriptionSingleSubOneEntry", chip::app::TestInteractionModelEngine::TestSubjectHasActiveSubscriptionSingleSubOneEntry),
                NL_TEST_DEF("TestSubjectHasActiveSubscriptionSingleSubMultipleEntries", chip::app::TestInteractionModelEngine::TestSubjectHasActiveSubscriptionSingleSubMultipleEntries),
                NL_TEST_DEF("TestSubjectHasActiveSubscriptionMultipleSubsSingleEntry", chip::app::TestInteractionModelEngine::TestSubjectHasActiveSubscriptionMultipleSubsSingleEntry),
                NL_TEST_DEF("TestSubjectHasActiveSubscriptionMultipleSubsMultipleEntries", chip::app::TestInteractionModelEngine::TestSubjectHasActiveSubscriptionMultipleSubsMultipleEntries),
                NL_TEST_DEF("TestSubjectHasActiveSubscriptionSubWithCAT", chip::app::TestInteractionModelEngine::TestSubjectHasActiveSubscriptionSubWithCAT),
                NL_TEST_SENTINEL()
        };
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestInteractionModelEngine",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
// clang-format on

} // namespace

int TestInteractionModelEngine()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestInteractionModelEngine)
