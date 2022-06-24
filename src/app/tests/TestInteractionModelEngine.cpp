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

#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>

#include <nlunit-test.h>

using TestContext = chip::Test::AppContext;

namespace chip {
namespace app {
class TestInteractionModelEngine
{
public:
    static void TestAttributePathParamsPushRelease(nlTestSuite * apSuite, void * apContext);
    static void TestRemoveDuplicateConcreteAttribute(nlTestSuite * apSuite, void * apContext);
    static int GetAttributePathListLength(ObjectList<AttributePathParams> * apattributePathParamsList);
};

int TestInteractionModelEngine::GetAttributePathListLength(ObjectList<AttributePathParams> * apAttributePathParamsList)
{
    int length                               = 0;
    ObjectList<AttributePathParams> * runner = apAttributePathParamsList;
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
    err               = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ObjectList<AttributePathParams> * attributePathParamsList = nullptr;
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
    err               = InteractionModelEngine::GetInstance()->Init(&ctx.GetExchangeManager(), &ctx.GetFabricTable());
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ObjectList<AttributePathParams> * attributePathParamsList = nullptr;
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

} // namespace app
} // namespace chip

namespace {

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("TestAttributePathParamsPushRelease", chip::app::TestInteractionModelEngine::TestAttributePathParamsPushRelease),
                NL_TEST_DEF("TestRemoveDuplicateConcreteAttribute", chip::app::TestInteractionModelEngine::TestRemoveDuplicateConcreteAttribute),
                NL_TEST_SENTINEL()
        };
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestInteractionModelEngine",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestInteractionModelEngine()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestInteractionModelEngine)
