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

#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/icd/server/ICDServerConfig.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <app/tests/InteractionModelEngineTestAccess.h>
#include <app/tests/ReadHandlerTestAccess.h>

#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#include <app/SimpleSubscriptionResumptionStorage.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>
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
class TestInteractionModelEngine : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        mpTestContext = new chip::Test::AppContext;
        mpTestContext->SetUpTestSuite();
    }
    static void TearDownTestSuite()
    {
        mpTestContext->TearDownTestSuite();
        if (mpTestContext != nullptr)
        {
            delete mpTestContext;
        }
    }

    void SetUp() override
    {

        if (mpTestContext != nullptr)
        {
            mpTestContext->SetUp();
        }
    }
    void TearDown() override
    {
        if (mpTestContext != nullptr)
        {
            mpTestContext->TearDown();
        }
    }
    static chip::Test::AppContext * mpTestContext;

    static int GetAttributePathListLength(SingleLinkedListNode<AttributePathParams> * apattributePathParamsList);
};

chip::Test::AppContext * TestInteractionModelEngine::mpTestContext = nullptr;

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

TEST_F(TestInteractionModelEngine, TestAttributePathParamsPushRelease)
{

    CHIP_ERROR err = CHIP_NO_ERROR;
    err = InteractionModelEngine::GetInstance()->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                                                      app::reporting::GetDefaultReportScheduler());
    EXPECT_EQ(err, CHIP_NO_ERROR);
    SingleLinkedListNode<AttributePathParams> * attributePathParamsList = nullptr;
    AttributePathParams attributePathParams1;
    AttributePathParams attributePathParams2;
    AttributePathParams attributePathParams3;

    attributePathParams1.mEndpointId = 1;
    attributePathParams2.mEndpointId = 2;
    attributePathParams3.mEndpointId = 3;

    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    ASSERT_NE(attributePathParamsList, nullptr);
    EXPECT_EQ(attributePathParams1.mEndpointId, attributePathParamsList->mValue.mEndpointId);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 1);

    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    ASSERT_NE(attributePathParamsList, nullptr);
    EXPECT_EQ(attributePathParams2.mEndpointId, attributePathParamsList->mValue.mEndpointId);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 2);

    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    ASSERT_NE(attributePathParamsList, nullptr);
    EXPECT_EQ(attributePathParams3.mEndpointId, attributePathParamsList->mValue.mEndpointId);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 3);

    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 0);
}

TEST_F(TestInteractionModelEngine, TestRemoveDuplicateConcreteAttribute)
{

    CHIP_ERROR err = CHIP_NO_ERROR;
    err = InteractionModelEngine::GetInstance()->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                                                      app::reporting::GetDefaultReportScheduler());
    EXPECT_EQ(err, CHIP_NO_ERROR);
    SingleLinkedListNode<AttributePathParams> * attributePathParamsList = nullptr;
    AttributePathParams attributePathParams1;
    AttributePathParams attributePathParams2;
    AttributePathParams attributePathParams3;

    // Three concrete paths, no duplicates
    attributePathParams1.mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams1.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams1.mAttributeId = chip::Test::MockAttributeId(1);

    attributePathParams2.mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams2.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams2.mAttributeId = chip::Test::MockAttributeId(2);

    attributePathParams3.mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams3.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams3.mAttributeId = chip::Test::MockAttributeId(3);

    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 3);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    attributePathParams1.mEndpointId  = kInvalidEndpointId;
    attributePathParams1.mClusterId   = kInvalidClusterId;
    attributePathParams1.mAttributeId = kInvalidAttributeId;

    attributePathParams2.mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams2.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams2.mAttributeId = chip::Test::MockAttributeId(2);

    attributePathParams3.mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams3.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams3.mAttributeId = chip::Test::MockAttributeId(3);

    // 1st path is wildcard endpoint, 2nd, 3rd paths are concrete paths, the concrete ones would be removed.
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 1);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    // 2nd path is wildcard endpoint, 1st, 3rd paths are concrete paths, the latter two would be removed.
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 1);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    // 3nd path is wildcard endpoint, 1st, 2nd paths are concrete paths, the latter two would be removed.
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 1);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    attributePathParams1.mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams1.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams1.mAttributeId = kInvalidAttributeId;

    attributePathParams2.mEndpointId  = chip::Test::kMockEndpoint2;
    attributePathParams2.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams2.mAttributeId = chip::Test::MockAttributeId(2);

    attributePathParams3.mEndpointId  = chip::Test::kMockEndpoint2;
    attributePathParams3.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams3.mAttributeId = chip::Test::MockAttributeId(3);

    // 1st is wildcard one, but not intersect with the latter two concrete paths, so the paths in total are 3 finally
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 3);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    attributePathParams1.mEndpointId  = kInvalidEndpointId;
    attributePathParams1.mClusterId   = kInvalidClusterId;
    attributePathParams1.mAttributeId = kInvalidAttributeId;

    attributePathParams2.mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams2.mClusterId   = kInvalidClusterId;
    attributePathParams2.mAttributeId = kInvalidAttributeId;

    attributePathParams3.mEndpointId  = kInvalidEndpointId;
    attributePathParams3.mClusterId   = kInvalidClusterId;
    attributePathParams3.mAttributeId = chip::Test::MockAttributeId(3);

    // Wildcards cannot be deduplicated.
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams3);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 3);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);

    attributePathParams1.mEndpointId  = kInvalidEndpointId;
    attributePathParams1.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams1.mAttributeId = chip::Test::MockAttributeId(10);

    attributePathParams2.mEndpointId  = chip::Test::kMockEndpoint3;
    attributePathParams2.mClusterId   = chip::Test::MockClusterId(2);
    attributePathParams2.mAttributeId = chip::Test::MockAttributeId(10);

    // 1st path is wildcard endpoint, 2nd path is invalid attribute
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams1);
    InteractionModelEngine::GetInstance()->PushFrontAttributePathList(attributePathParamsList, attributePathParams2);
    InteractionModelEngine::GetInstance()->RemoveDuplicateConcreteAttributePath(attributePathParamsList);
    EXPECT_EQ(GetAttributePathListLength(attributePathParamsList), 2);
    InteractionModelEngine::GetInstance()->ReleaseAttributePathList(attributePathParamsList);
}

/**
 * @brief Test verifies the SubjectHasActiveSubscription with a single subscription with a single entry
 */
TEST_F(TestInteractionModelEngine, TestSubjectHasActiveSubscriptionSingleSubOneEntry)
{

    NullReadHandlerCallback nullCallback;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

    NodeId bobNodeId           = 0x12344321ull;
    FabricIndex bobFabricIndex = 1;

    // Create ExchangeContext
    Messaging::ExchangeContext * exchangeCtx1 = mpTestContext->NewExchangeToBob(nullptr, false);
    EXPECT_TRUE(exchangeCtx1);

    // InteractionModelEngine init
    EXPECT_EQ(CHIP_NO_ERROR,
              engine->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                           reporting::GetDefaultReportScheduler()));

    // Verify that there are no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Create and setup readHandler 1
    ReadHandler * readHandler1 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx1, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    // Verify that Bob still doesn't have an active subscription
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));
    chip::Test::ReadHandlerTestAccess privatereadHandler1(readHandler1);

    // Set readHandler1 to active
    privatereadHandler1.SetStateFlagToActiveSubscription(true);

    // Verify that Bob still has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();

    // Verify that there are no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));
}

/**
 * @brief Test verifies that the SubjectHasActiveSubscription will continue iterating till it fines at least one valid active
 * subscription
 */
TEST_F(TestInteractionModelEngine, TestSubjectHasActiveSubscriptionSingleSubMultipleEntries)
{

    NullReadHandlerCallback nullCallback;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

    NodeId bobNodeId           = 0x12344321ull;
    FabricIndex bobFabricIndex = 1;

    // Create ExchangeContexts
    Messaging::ExchangeContext * exchangeCtx1 = mpTestContext->NewExchangeToBob(nullptr, false);
    EXPECT_TRUE(exchangeCtx1);

    Messaging::ExchangeContext * exchangeCtx2 = mpTestContext->NewExchangeToBob(nullptr, false);
    EXPECT_TRUE(exchangeCtx1);

    // InteractionModelEngine init
    EXPECT_EQ(CHIP_NO_ERROR,
              engine->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                           reporting::GetDefaultReportScheduler()));

    // Verify that both Alice and Bob have no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Create readHandler 1
    engine->GetReadHandlerPool().CreateObject(nullCallback, exchangeCtx1, ReadHandler::InteractionType::Subscribe,
                                              reporting::GetDefaultReportScheduler());

    // Verify that Bob still doesn't have an active subscription
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Create and setup readHandler 2
    ReadHandler * readHandler2 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx2, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    chip::Test::ReadHandlerTestAccess privatereadHandler2(readHandler2);

    // Verify that Bob still doesn't have an active subscription
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Set readHandler2 to active
    privatereadHandler2.SetStateFlagToActiveSubscription(true);

    // Verify that Bob has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Release active ReadHandler
    engine->GetReadHandlerPool().ReleaseObject(readHandler2);

    // Verify that there are no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();
}

/**
 * @brief Test validates that the SubjectHasActiveSubscription can support multiple subscriptions from different clients
 */
TEST_F(TestInteractionModelEngine, TestSubjectHasActiveSubscriptionMultipleSubsSingleEntry)
{

    NullReadHandlerCallback nullCallback;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

    NodeId bobNodeId             = 0x12344321ull;
    FabricIndex bobFabricIndex   = 1;
    NodeId aliceNodeId           = 0x11223344ull;
    FabricIndex aliceFabricIndex = 2;

    // Create ExchangeContexts
    Messaging::ExchangeContext * exchangeCtx1 = mpTestContext->NewExchangeToBob(nullptr, false);
    EXPECT_TRUE(exchangeCtx1);

    Messaging::ExchangeContext * exchangeCtx2 = mpTestContext->NewExchangeToAlice(nullptr, false);
    EXPECT_TRUE(exchangeCtx2);

    // InteractionModelEngine init
    EXPECT_EQ(CHIP_NO_ERROR,
              engine->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                           reporting::GetDefaultReportScheduler()));

    // Verify that both Alice and Bob have no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Create and setup readHandler 1
    ReadHandler * readHandler1 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx1, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    chip::Test::ReadHandlerTestAccess privatereadHandler1(readHandler1);

    // Create and setup readHandler 2
    ReadHandler * readHandler2 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx2, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    chip::Test::ReadHandlerTestAccess privatereadHandler2(readHandler2);

    // Verify that Bob still doesn't have an active subscription
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Set readHandler1 to active
    privatereadHandler1.SetStateFlagToActiveSubscription(true);

    // Verify that Bob has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice still doesn't have an active subscription
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Set readHandler2 to active
    privatereadHandler2.SetStateFlagToActiveSubscription(true);

    // Verify that Bob has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Set readHandler1 to inactive
    privatereadHandler1.SetStateFlagToActiveSubscription(false);

    // Verify that Bob doesn't have an active subscription
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice still has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();

    // Verify that both Alice and Bob have no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));
}

/**
 * @brief Test validates that the SubjectHasActiveSubscription can find the active subscription even if there are multiple
 * subscriptions for each client
 */
TEST_F(TestInteractionModelEngine, TestSubjectHasActiveSubscriptionMultipleSubsMultipleEntries)
{
    NullReadHandlerCallback nullCallback;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();

    NodeId bobNodeId             = 0x12344321ull;
    FabricIndex bobFabricIndex   = 1;
    NodeId aliceNodeId           = 0x11223344ull;
    FabricIndex aliceFabricIndex = 2;

    // Create ExchangeContexts
    Messaging::ExchangeContext * exchangeCtx11 = mpTestContext->NewExchangeToBob(nullptr, false);
    EXPECT_TRUE(exchangeCtx11);

    Messaging::ExchangeContext * exchangeCtx12 = mpTestContext->NewExchangeToBob(nullptr, false);
    EXPECT_TRUE(exchangeCtx12);

    Messaging::ExchangeContext * exchangeCtx21 = mpTestContext->NewExchangeToAlice(nullptr, false);
    EXPECT_TRUE(exchangeCtx21);

    Messaging::ExchangeContext * exchangeCtx22 = mpTestContext->NewExchangeToAlice(nullptr, false);
    EXPECT_TRUE(exchangeCtx22);

    // InteractionModelEngine init
    EXPECT_EQ(CHIP_NO_ERROR,
              engine->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                           reporting::GetDefaultReportScheduler()));

    // Verify that both Alice and Bob have no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Create and setup readHandler 1-1
    engine->GetReadHandlerPool().CreateObject(nullCallback, exchangeCtx11, ReadHandler::InteractionType::Subscribe,
                                              reporting::GetDefaultReportScheduler());

    // Create and setup readHandler 1-2
    ReadHandler * readHandler12 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx12, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    chip::Test::ReadHandlerTestAccess privatereadHandler12(readHandler12);

    // Create and setup readHandler 2-1
    engine->GetReadHandlerPool().CreateObject(nullCallback, exchangeCtx21, ReadHandler::InteractionType::Subscribe,
                                              reporting::GetDefaultReportScheduler());

    // Create and setup readHandler 2-2
    ReadHandler * readHandler22 = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx22, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    chip::Test::ReadHandlerTestAccess privatereadHandler22(readHandler22);

    // Verify that both Alice and Bob have no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Set readHandler 1-2 to active
    privatereadHandler12.SetStateFlagToActiveSubscription(true);

    // Verify that Bob has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice still doesn't have an active subscription
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Set readHandler 2-2 to active
    privatereadHandler22.SetStateFlagToActiveSubscription(true);

    // Verify that Bob has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Set readHandler1 to inactive
    privatereadHandler12.SetStateFlagToActiveSubscription(false);

    // Verify that Bob doesn't have an active subscription
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));

    // Verify that Alice still has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();

    // Verify that both Alice and Bob have no active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, bobNodeId));
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(aliceFabricIndex, aliceNodeId));
}

/**
 * @brief Verifies that SubjectHasActiveSubscription support CATs as a subject-id
 */
TEST_F(TestInteractionModelEngine, TestSubjectHasActiveSubscriptionSubWithCAT)
{
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();
    NullReadHandlerCallback nullCallback;

    CASEAuthTag cat            = 0x1111'0001;
    CASEAuthTag invalidCAT     = 0x1112'0001;
    CATValues cats             = CATValues{ { cat } };
    NodeId valideSubjectId     = NodeIdFromCASEAuthTag(cat);
    NodeId invalideSubjectId   = NodeIdFromCASEAuthTag(invalidCAT);
    FabricIndex bobFabricIndex = 1;

    // InteractionModelEngine init
    EXPECT_EQ(CHIP_NO_ERROR,
              engine->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                           reporting::GetDefaultReportScheduler()));

    // Make sure we are using CASE sessions, because there is no defunct-marking for PASE.
    mpTestContext->ExpireSessionBobToAlice();
    mpTestContext->ExpireSessionAliceToBob();
    EXPECT_EQ(CHIP_NO_ERROR, mpTestContext->CreateCASESessionBobToAlice(cats));
    EXPECT_EQ(CHIP_NO_ERROR, mpTestContext->CreateCASESessionAliceToBob(cats));

    // Create ExchangeContexts
    Messaging::ExchangeContext * exchangeCtx = mpTestContext->NewExchangeToBob(nullptr, false);
    EXPECT_TRUE(exchangeCtx);

    // Create readHandler
    ReadHandler * readHandler = engine->GetReadHandlerPool().CreateObject(
        nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, reporting::GetDefaultReportScheduler());

    chip::Test::ReadHandlerTestAccess privatereadHandler(readHandler);

    // Verify there are not active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, valideSubjectId));
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, invalideSubjectId));

    // Set readHandler to active
    privatereadHandler.SetStateFlagToActiveSubscription(true);

    // Verify tthat valid subjectID has an active subscription
    EXPECT_TRUE(engine->SubjectHasActiveSubscription(bobFabricIndex, valideSubjectId));
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, invalideSubjectId));

    // Clean up read handlers
    engine->GetReadHandlerPool().ReleaseAll();

    // Verify there are not active subscriptions
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, valideSubjectId));
    EXPECT_FALSE(engine->SubjectHasActiveSubscription(bobFabricIndex, invalideSubjectId));
}

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

/**
 * @brief Test verifies the SubjectHasPersistedSubscription with single and multiple persisted subscriptions.
 */
TEST_F(TestInteractionModelEngine, TestSubjectHasPersistedSubscription)
{

    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::TestPersistentStorageDelegate storage;
    chip::app::SimpleSubscriptionResumptionStorage subscriptionStorage;

    err = subscriptionStorage.Init(&storage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = InteractionModelEngine::GetInstance()->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                                                      app::reporting::GetDefaultReportScheduler(), nullptr, &subscriptionStorage);
    EXPECT_EQ(err, CHIP_NO_ERROR);

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
    EXPECT_FALSE(InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric1, nodeId1));

    // Add one entry
    EXPECT_EQ(CHIP_NO_ERROR, subscriptionStorage.Save(info1));

    // Verify that entry matches - Should return true
    EXPECT_TRUE(InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric1, nodeId1));

    // Test with absent subscription - Should return false
    EXPECT_FALSE(InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric2, nodeId2));

    // Add second entry
    EXPECT_EQ(CHIP_NO_ERROR, subscriptionStorage.Save(info2));

    // Verify that entry matches - Should return true
    EXPECT_TRUE(InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric2, nodeId2));
    EXPECT_TRUE(InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric1, nodeId1));

    // Remove an entry
    EXPECT_EQ(CHIP_NO_ERROR, subscriptionStorage.Delete(nodeId1, fabric1, sub1));

    // Test with absent subscription - Should return false
    EXPECT_FALSE(InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(fabric1, nodeId1));

    // Clean Up entries
    subscriptionStorage.DeleteAll(fabric1);
    subscriptionStorage.DeleteAll(fabric2);
}

#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

TEST_F(TestInteractionModelEngine, TestSubscriptionResumptionTimer)
{

    CHIP_ERROR err = CHIP_NO_ERROR;
    err = InteractionModelEngine::GetInstance()->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                                                      app::reporting::GetDefaultReportScheduler());
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::Test::InteractionModelEngineTestAccess privateEngine(InteractionModelEngine::GetInstance());
    uint32_t timeTillNextResubscriptionMs;
    privateEngine.SetNumSubscriptionResumptionRetries(0);
    timeTillNextResubscriptionMs = privateEngine.ComputeTimeSecondsTillNextSubscriptionResumption();
    EXPECT_EQ(timeTillNextResubscriptionMs, (unsigned) CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MIN_RETRY_INTERVAL_SECS);

    uint32_t lastTimeTillNextResubscriptionMs = timeTillNextResubscriptionMs;
    for (privateEngine.SetNumSubscriptionResumptionRetries(1); privateEngine.GetNumSubscriptionResumptionRetries() <=
         CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_FIBONACCI_STEP_INDEX;
         privateEngine.SetNumSubscriptionResumptionRetries(privateEngine.GetNumSubscriptionResumptionRetries() + 1))
    {
        timeTillNextResubscriptionMs = privateEngine.ComputeTimeSecondsTillNextSubscriptionResumption();
        EXPECT_GE(timeTillNextResubscriptionMs, lastTimeTillNextResubscriptionMs);
        EXPECT_LT(timeTillNextResubscriptionMs, (unsigned) CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS);
        lastTimeTillNextResubscriptionMs = timeTillNextResubscriptionMs;
    }

    privateEngine.SetNumSubscriptionResumptionRetries(2000);
    timeTillNextResubscriptionMs = privateEngine.ComputeTimeSecondsTillNextSubscriptionResumption();
    EXPECT_EQ(timeTillNextResubscriptionMs, (unsigned) CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS);
}

#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

TEST_F(TestInteractionModelEngine, TestDecrementNumSubscriptionsToResume)
{
    CHIP_ERROR err                  = CHIP_NO_ERROR;
    InteractionModelEngine * engine = InteractionModelEngine::GetInstance();
    chip::Test::InteractionModelEngineTestAccess privateEngine(engine);
    constexpr uint8_t kNumberOfSubsToResume = 5;
    uint8_t numberOfSubsRemaining           = kNumberOfSubsToResume;

    err = engine->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                       app::reporting::GetDefaultReportScheduler());
    EXPECT_EQ(err, CHIP_NO_ERROR);

#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    ICDManager manager;
    engine->SetICDManager(&manager);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

    // Set number of subs
    privateEngine.SetNumOfSubscriptionsToResume(kNumberOfSubsToResume);

#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    // Verify mIsBootUpResumeSubscriptionExecuted has not been set
    EXPECT_FALSE(manager.GetIsBootUpResumeSubscriptionExecuted());
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

    // Decrease number of subs by 1
    numberOfSubsRemaining--;
    engine->DecrementNumSubscriptionsToResume();
    EXPECT_EQ(numberOfSubsRemaining, privateEngine.GetNumOfSubscriptionsToResume());

    // Decrease to 0 subs remaining
    while (numberOfSubsRemaining > 0)
    {
#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
        // Verify mIsBootUpResumeSubscriptionExecuted has not been set
        EXPECT_FALSE(manager.GetIsBootUpResumeSubscriptionExecuted());
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

        numberOfSubsRemaining--;
        engine->DecrementNumSubscriptionsToResume();
        EXPECT_EQ(numberOfSubsRemaining, privateEngine.GetNumOfSubscriptionsToResume());
    }

#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    // Verify mIsBootUpResumeSubscriptionExecuted has been set
    EXPECT_TRUE(manager.GetIsBootUpResumeSubscriptionExecuted());
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

    // Make sure we don't rollover / go negative
    engine->DecrementNumSubscriptionsToResume();
    EXPECT_EQ(numberOfSubsRemaining, privateEngine.GetNumOfSubscriptionsToResume());

    // Clean up
#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    engine->SetICDManager(nullptr);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
}
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

} // namespace app
} // namespace chip
