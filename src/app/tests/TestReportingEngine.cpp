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

#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/Engine.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>

#include <cinttypes>
#include <nlunit-test.h>

using TestContext = chip::Test::AppContext;

namespace chip {

constexpr ClusterId kTestClusterId        = 6;
constexpr EndpointId kTestEndpointId      = 1;
constexpr chip::AttributeId kTestFieldId1 = 1;
constexpr chip::AttributeId kTestFieldId2 = 2;

namespace app {
namespace reporting {
class TestReportingEngine
{
public:
    static void TestBuildAndSendSingleReportData(nlTestSuite * apSuite, void * apContext);
    static void TestMergeOverlappedAttributePath(nlTestSuite * apSuite, void * apContext);
    static void TestMergeAttributePathWhenDirtySetPoolExhausted(nlTestSuite * apSuite, void * apContext);

private:
    static bool InsertToDirtySet(const AttributePathParams & aPath);

    struct ExpectedDirtySetContent : public AttributePathParams
    {
        ExpectedDirtySetContent(const AttributePathParams & path) : AttributePathParams(path) {}
        bool verified = false;
    };

    template <typename... Args>
    static bool VerifyDirtySetContent(const Args &... args)
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
};

void TestReportingEngine::TestBuildAndSendSingleReportData(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter writer;
    System::PacketBufferHandle readRequestbuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ReadRequestMessage::Builder readRequestBuilder;
    DummyDelegate dummy;

    err = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    TestExchangeDelegate delegate;
    Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(&delegate);

    writer.Init(std::move(readRequestbuf));
    err = readRequestBuilder.Init(&writer);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    AttributePathIBs::Builder & attributePathListBuilder = readRequestBuilder.CreateAttributeRequests();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    AttributePathIB::Builder & attributePathBuilder1 = attributePathListBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder1.Node(1).Endpoint(kTestEndpointId).Cluster(kTestClusterId).Attribute(kTestFieldId1).EndOfAttributePathIB();
    NL_TEST_ASSERT(apSuite, attributePathBuilder1.GetError() == CHIP_NO_ERROR);

    AttributePathIB::Builder & attributePathBuilder2 = attributePathListBuilder.CreatePath();
    NL_TEST_ASSERT(apSuite, attributePathListBuilder.GetError() == CHIP_NO_ERROR);
    attributePathBuilder2.Node(1).Endpoint(kTestEndpointId).Cluster(kTestClusterId).Attribute(kTestFieldId2).EndOfAttributePathIB();
    NL_TEST_ASSERT(apSuite, attributePathBuilder2.GetError() == CHIP_NO_ERROR);
    attributePathListBuilder.EndOfAttributePathIBs();

    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    readRequestBuilder.IsFabricFiltered(false).EndOfReadRequestMessage();
    NL_TEST_ASSERT(apSuite, readRequestBuilder.GetError() == CHIP_NO_ERROR);
    err = writer.Finalize(&readRequestbuf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    app::ReadHandler readHandler(dummy, exchangeCtx, chip::app::ReadHandler::InteractionType::Read);
    readHandler.OnInitialRequest(std::move(readRequestbuf));
    err = InteractionModelEngine::GetInstance()->GetReportingEngine().BuildAndSendSingleReportData(&readHandler);

    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void TestReportingEngine::TestMergeOverlappedAttributePath(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    err               = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    AttributePathParams * clusterInfo = InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.CreateObject();
    clusterInfo->mEndpointId          = 1;
    clusterInfo->mClusterId           = 1;
    clusterInfo->mAttributeId         = 1;

    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mEndpointId  = 1;
        testClusterInfo.mClusterId   = 1;
        testClusterInfo.mAttributeId = 3;
        NL_TEST_ASSERT(apSuite,
                       !InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
    }
    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mEndpointId  = 1;
        testClusterInfo.mClusterId   = 1;
        testClusterInfo.mAttributeId = 1;
        testClusterInfo.mListIndex   = 2;
        NL_TEST_ASSERT(apSuite,
                       InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
    }

    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mEndpointId  = 1;
        testClusterInfo.mClusterId   = 1;
        testClusterInfo.mAttributeId = kInvalidAttributeId;
        NL_TEST_ASSERT(apSuite,
                       InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
    }

    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mClusterId   = kInvalidClusterId;
        testClusterInfo.mAttributeId = kInvalidAttributeId;
        NL_TEST_ASSERT(apSuite,
                       InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
        NL_TEST_ASSERT(apSuite, clusterInfo->mClusterId == kInvalidClusterId && clusterInfo->mAttributeId == kInvalidAttributeId);
    }

    {
        AttributePathParams testClusterInfo;
        testClusterInfo.mEndpointId  = kInvalidEndpointId;
        testClusterInfo.mClusterId   = kInvalidClusterId;
        testClusterInfo.mAttributeId = kInvalidAttributeId;
        NL_TEST_ASSERT(apSuite,
                       InteractionModelEngine::GetInstance()->GetReportingEngine().MergeOverlappedAttributePath(testClusterInfo));
        NL_TEST_ASSERT(apSuite,
                       clusterInfo->mEndpointId == kInvalidEndpointId && clusterInfo->mClusterId == kInvalidClusterId &&
                           clusterInfo->mAttributeId == kInvalidAttributeId);
    }
    InteractionModelEngine::GetInstance()->GetReportingEngine().Shutdown();
}

bool TestReportingEngine::InsertToDirtySet(const AttributePathParams & aPath)
{
    auto path = InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.CreateObject();
    VerifyOrReturnError(path != nullptr, false);
    *path             = aPath;
    path->mGeneration = InteractionModelEngine::GetInstance()->GetReportingEngine().GetDirtySetGeneration();
    return true;
}

void TestReportingEngine::TestMergeAttributePathWhenDirtySetPoolExhausted(nlTestSuite * apSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    err               = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();
    InteractionModelEngine::GetInstance()->GetReportingEngine().BumpDirtySetGeneration();

    // Case 1: All dirty paths including the new one are under the same cluster.
    // -> Expected behavior: The dirty set is replaced by a wildcard attribute path under the same cluster.
    for (AttributeId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        NL_TEST_ASSERT(apSuite, InsertToDirtySet(AttributePathParams(kTestEndpointId, kTestClusterId, i)));
    }
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                           AttributePathParams(kTestEndpointId, kTestClusterId, CHIP_IM_SERVER_MAX_NUM_DIRTY_SET + 1)));
    NL_TEST_ASSERT(apSuite, VerifyDirtySetContent(AttributePathParams(kTestEndpointId, kTestClusterId)));

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();

    // Case 2: All dirty paths including the new one are under the same endpoint.
    // -> Expected behavior: The dirty set is replaced by a wildcard cluster path under the same endpoint.
    for (ClusterId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        NL_TEST_ASSERT(apSuite, InsertToDirtySet(AttributePathParams(kTestEndpointId, i, 1)));
    }
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                           AttributePathParams(kTestEndpointId, ClusterId(CHIP_IM_SERVER_MAX_NUM_DIRTY_SET + 1), 1)));
    NL_TEST_ASSERT(apSuite, VerifyDirtySetContent(AttributePathParams(kTestEndpointId, kInvalidClusterId)));

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();

    // Case 3: All dirty paths including the new one are under the different endpoints.
    // -> Expected behavior: The dirty set is replaced by a wildcard endpoint.
    for (EndpointId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        NL_TEST_ASSERT(apSuite, InsertToDirtySet(AttributePathParams(EndpointId(i), i, i)));
    }
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                           AttributePathParams(EndpointId(CHIP_IM_SERVER_MAX_NUM_DIRTY_SET + 1), 1, 1)));
    NL_TEST_ASSERT(apSuite, VerifyDirtySetContent(AttributePathParams()));

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();

    // Case 4: All existing dirty paths are under the same cluster, the new path comes from another cluster.
    // -> Expected behavior: The existing paths are merged into one single wildcard attribute path. New path is inserted as-is.
    for (EndpointId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        NL_TEST_ASSERT(apSuite, InsertToDirtySet(AttributePathParams(kTestEndpointId, kTestClusterId, i)));
    }
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                           AttributePathParams(kTestEndpointId + 1, kTestClusterId + 1, 1)));
    NL_TEST_ASSERT(apSuite,
                   VerifyDirtySetContent(AttributePathParams(kTestEndpointId, kTestClusterId),
                                         AttributePathParams(kTestEndpointId + 1, kTestClusterId + 1, 1)));

    InteractionModelEngine::GetInstance()->GetReportingEngine().mGlobalDirtySet.ReleaseAll();

    // Case 5: All existing dirty paths are under the same endpoint, the new path comes from another endpoint.
    // -> Expected behavior: The existing paths are merged into one single wildcard cluster path. New path is inserted as-is.
    for (EndpointId i = 1; i <= CHIP_IM_SERVER_MAX_NUM_DIRTY_SET; i++)
    {
        NL_TEST_ASSERT(apSuite, InsertToDirtySet(AttributePathParams(kTestEndpointId, i, 1)));
    }
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR ==
                       InteractionModelEngine::GetInstance()->GetReportingEngine().InsertPathIntoDirtySet(
                           AttributePathParams(kTestEndpointId + 1, kTestClusterId + 1, 1)));
    NL_TEST_ASSERT(apSuite,
                   VerifyDirtySetContent(AttributePathParams(kTestEndpointId, kInvalidClusterId),
                                         AttributePathParams(kTestEndpointId + 1, kTestClusterId + 1, 1)));

    InteractionModelEngine::GetInstance()->GetReportingEngine().Shutdown();
}

} // namespace reporting
} // namespace app
} // namespace chip

namespace {
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("CheckBuildAndSendSingleReportData", chip::app::reporting::TestReportingEngine::TestBuildAndSendSingleReportData),
    NL_TEST_DEF("TestMergeOverlappedAttributePath", chip::app::reporting::TestReportingEngine::TestMergeOverlappedAttributePath),
    NL_TEST_DEF("TestMergeAttributePathWhenDirtySetPoolExhausted", chip::app::reporting::TestReportingEngine::TestMergeAttributePathWhenDirtySetPoolExhausted),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestReportingEngine",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestReportingEngine()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestReportingEngine)
