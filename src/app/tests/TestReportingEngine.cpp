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
 *      This file implements unit tests for CHIP Interaction Model Reporting Engine
 *
 */

#include <cinttypes>

#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/Engine.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <pw_unit_test/framework.h>

using TestContext = chip::Test::AppContext;

namespace chip {

constexpr ClusterId kTestClusterId        = 6;
constexpr EndpointId kTestEndpointId      = 1;
constexpr chip::AttributeId kTestFieldId1 = 1;
constexpr chip::AttributeId kTestFieldId2 = 2;

namespace app {
namespace reporting {

std::unique_ptr<TestContext> mpTestContext;

class TestReportingEngine : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {

        mpTestContext = std::make_unique<TestContext>();
        ASSERT_NE(mpTestContext, nullptr);
        mpTestContext->SetUpTestSuite();
    }
    static void TearDownTestSuite()
    {
        mpTestContext->TearDownTestSuite();
        mpTestContext.reset();
    }
    void SetUp() { mpTestContext->SetUp(); }
    void TearDown() { mpTestContext->TearDown(); }

    template <typename... Args>
    static bool VerifyDirtySetContent(const Args &... args);
    static bool InsertToDirtySet(const AttributePathParams & aPath);

    void TestBuildAndSendSingleReportData();
    void TestMergeOverlappedAttributePath();
    void TestMergeAttributePathWhenDirtySetPoolExhausted();

private:
    struct ExpectedDirtySetContent : public AttributePathParams
    {
        ExpectedDirtySetContent(const AttributePathParams & path) : AttributePathParams(path) {}
        bool verified = false;
    };
};

class TestExchangeDelegate : public Messaging::ExchangeDelegate
{
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

class DummyDelegate : public ReadHandler::ManagementCallback
{
public:
    void OnDone(ReadHandler & apHandler) override {}
    chip::app::ReadHandler::ApplicationCallback * GetAppCallback() override { return nullptr; }
    chip::app::InteractionModelEngine * GetInteractionModelEngine() override
    {
        return chip::app::InteractionModelEngine::GetInstance();
    }
};

template <typename... Args>
bool TestReportingEngine::VerifyDirtySetContent(const Args &... args)
{
    const int size                        = sizeof...(args);
    ExpectedDirtySetContent content[size] = { ExpectedDirtySetContent(args)... };

    if (InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ForEachActiveObject([&](auto * path) {
            for (int i = 0; i < size; i++)
            {
                if (static_cast<AttributePathParams>(content[i]) == static_cast<AttributePathParams>(*path))
                {
                    content[i].verified = true;
                    return Loop::Continue;
                }
            }
            ChipLogDetail(DataManagement, "Dirty path Endpoint %x Cluster %" PRIx32 ", Attribute %" PRIx32 " is not expected",
                          path->mEndpointId, path->mClusterId, path->mAttributeId);
            return Loop::Break;
        }) == Loop::Break)
    {
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        if (!content[i].verified)
        {
            ChipLogDetail(DataManagement,
                          "Dirty path Endpoint %x Cluster %" PRIx32 ", Attribute %" PRIx32 " is not found in the dirty set",
                          content[i].mEndpointId, content[i].mClusterId, content[i].mAttributeId);
            return false;
        }
    }
    return true;
}

bool TestReportingEngine::InsertToDirtySet(const AttributePathParams & aPath)
{
    auto path = InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.CreateObject();
    VerifyOrReturnError(path != nullptr, false);
    *path             = aPath;
    path->mGeneration = InteractionModelEngine::GetInstance()->GetReportingEngine().GetDirtySetGeneration();
    return true;
}

TEST_F_FROM_FIXTURE(TestReportingEngine, TestBuildAndSendSingleReportData)
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    DummyDelegate dummy;

    EXPECT_EQ(InteractionModelEngine::GetInstance()->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                                                          app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);
    TestExchangeDelegate delegate;
    Messaging::ExchangeContext * exchangeCtx = mpTestContext->NewExchangeToAlice(&delegate);

    writer.Init(std::move(readRequestbuf));
    EXPECT_EQ(readRequestBuilder.Init(&writer), CHIP_NO_ERROR);
    AttributePathIBs::Builder & attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
    AttributePathIB::Builder & attributePathBuilder1 = attributePathListBuilder.CreatePath();
    EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);
    attributePathBuilder1.Node(1).Endpoint(kTestEndpointId).Cluster(kTestClusterId).Attribute(kTestFieldId1).EndOfAttributePathIB();
    EXPECT_EQ(attributePathBuilder1.GetError(), CHIP_NO_ERROR);

    AttributePathIB::Builder & attributePathBuilder2 = attributePathListBuilder.CreatePath();
    EXPECT_EQ(attributePathListBuilder.GetError(), CHIP_NO_ERROR);
    attributePathBuilder2.Node(1).Endpoint(kTestEndpointId).Cluster(kTestClusterId).Attribute(kTestFieldId2).EndOfAttributePathIB();
    EXPECT_EQ(attributePathBuilder2.GetError(), CHIP_NO_ERROR);
    attributePathListBuilder.EndOfAttributePathIBs();

    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
    readRequestBuilder.IsFabricFiltered(false).EndOfReadRequestMessage();
    EXPECT_EQ(readRequestBuilder.GetError(), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(&readRequestbuf), CHIP_NO_ERROR);
    app::ReadHandler readHandler(dummy, exchangeCtx, chip::app::ReadHandler::InteractionType::Read,
                                 app::reporting::GetDefaultReportScheduler());
    readHandler.OnInitialRequest(std::move(readRequestbuf));

    EXPECT_EQ(InteractionModelEngine::GetInstance()->GetReportingEngine().BuildAndSendSingleReportData(&readHandler),
              CHIP_NO_ERROR);

    mpTestContext->DrainAndServiceIO();
}

TEST_F_FROM_FIXTURE(TestReportingEngine, TestMergeOverlappedAttributePath)
{
    EXPECT_EQ(InteractionModelEngine::GetInstance()->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                                                          app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);

    AttributePathParams * clusterInfo = InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.CreateObject();
    clusterInfo->mEndpointId          = 1;
    clusterInfo->mClusterId           = 1;
    clusterInfo->mAttributeId         = 1;

    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mEndpointId  = 1;
        testClusterInfo.mClusterId   = 1;
        testClusterInfo.mAttributeId = 3;
        EXPECT_FALSE(InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
    }
    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mEndpointId  = 1;
        testClusterInfo.mClusterId   = 1;
        testClusterInfo.mAttributeId = 1;
        testClusterInfo.mListIndex   = 2;
        EXPECT_TRUE(InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
    }

    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mEndpointId  = 1;
        testClusterInfo.mClusterId   = 1;
        testClusterInfo.mAttributeId = kInvalidAttributeId;
        EXPECT_TRUE(InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
    }

    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mClusterId   = kInvalidClusterId;
        testClusterInfo.mAttributeId = kInvalidAttributeId;
        EXPECT_TRUE(InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
        EXPECT_TRUE(clusterInfo->mClusterId == kInvalidClusterId && clusterInfo->mAttributeId == kInvalidAttributeId);
    }

    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mEndpointId  = kInvalidEndpointId;
        testClusterInfo.mClusterId   = kInvalidClusterId;
        testClusterInfo.mAttributeId = kInvalidAttributeId;
        EXPECT_TRUE(InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
        EXPECT_TRUE(clusterInfo->mEndpointId == kInvalidEndpointId && clusterInfo->mClusterId == kInvalidClusterId &&
                    clusterInfo->mAttributeId == kInvalidAttributeId);
    }
    InteractionModelEngine::GetInstance()->GetReportingEngine().Shutdown();
}

TEST_F_FROM_FIXTURE(TestReportingEngine, TestMergeAttributePathWhenDirtySetPoolExhausted)
{
    EXPECT_EQ(InteractionModelEngine::GetInstance()->Init(&mpTestContext->GetExchangeManager(), &mpTestContext->GetFabricTable(),
                                                          app::reporting::GetDefaultReportScheduler()),
              CHIP_NO_ERROR);

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();
    InteractionModelEngine::GetInstance()->GetReportingEngine().BumpDirtySetGeneration();

    // Case 1: All dirty paths including the new one are under the same cluster.
    // -> Expected behavior: The dirty set is replaced by a wildcard attribute path under the same cluster.
    for (AttributeId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        EXPECT_TRUE(InsertToDirtySet(AttributePathParams(kTestEndpointId, kTestClusterId, i)));
    }
    EXPECT_EQ(CHIP_NO_ERROR,
              InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                  AttributePathParams(kTestEndpointId, kTestClusterId, CHIP_IM_SERVER_MAX_NUM_DIRTY_SET + 1)));
    EXPECT_TRUE(VerifyDirtySetContent(AttributePathParams(kTestEndpointId, kTestClusterId)));

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();

    // Case 2: All dirty paths including the new one are under the same endpoint.
    // -> Expected behavior: The dirty set is replaced by a wildcard cluster path under the same endpoint.
    for (ClusterId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        EXPECT_TRUE(InsertToDirtySet(AttributePathParams(kTestEndpointId, i, 1)));
    }
    EXPECT_EQ(CHIP_NO_ERROR,
              InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                  AttributePathParams(kTestEndpointId, ClusterId(CHIP_IM_SERVER_MAX_NUM_DIRTY_SET + 1), 1)));
    EXPECT_TRUE(VerifyDirtySetContent(AttributePathParams(kTestEndpointId, kInvalidClusterId)));

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();

    // Case 3: All dirty paths including the new one are under the different endpoints.
    // -> Expected behavior: The dirty set is replaced by a wildcard endpoint.
    for (EndpointId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        EXPECT_TRUE(InsertToDirtySet(AttributePathParams(EndpointId(i), i, i)));
    }
    EXPECT_EQ(CHIP_NO_ERROR,
              InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                  AttributePathParams(EndpointId(CHIP_IM_SERVER_MAX_NUM_DIRTY_SET + 1), 1, 1)));
    EXPECT_TRUE(VerifyDirtySetContent(AttributePathParams()));

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();

    // Case 4: All existing dirty paths are under the same cluster, the new path comes from another cluster.
    // -> Expected behavior: The existing paths are merged into one single wildcard attribute path. New path is inserted
    // as-is.
    for (EndpointId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        EXPECT_TRUE(InsertToDirtySet(AttributePathParams(kTestEndpointId, kTestClusterId, i)));
    }
    EXPECT_EQ(CHIP_NO_ERROR,
              InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                  AttributePathParams(kTestEndpointId + 1, kTestClusterId + 1, 1)));
    EXPECT_TRUE(VerifyDirtySetContent(AttributePathParams(kTestEndpointId, kTestClusterId),
                                      AttributePathParams(kTestEndpointId + 1, kTestClusterId + 1, 1)));

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();

    // Case 5: All existing dirty paths are under the same endpoint, the new path comes from another endpoint.
    // -> Expected behavior: The existing paths are merged into one single wildcard cluster path. New path is inserted as-is.
    for (EndpointId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        EXPECT_TRUE(InsertToDirtySet(AttributePathParams(kTestEndpointId, i, 1)));
    }
    EXPECT_EQ(CHIP_NO_ERROR,
              InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                  AttributePathParams(kTestEndpointId + 1, kTestClusterId + 1, 1)));
    EXPECT_TRUE(VerifyDirtySetContent(AttributePathParams(kTestEndpointId, kInvalidClusterId),
                                      AttributePathParams(kTestEndpointId + 1, kTestClusterId + 1, 1)));

    InteractionModelEngine::GetInstance()->GetReportingEngine().Shutdown();
}

} // namespace reporting
} // namespace app
} // namespace chip
