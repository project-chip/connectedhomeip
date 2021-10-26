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
 *     This file defines read handler for a CHIP Interaction Data model
 *
 */

#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributePathExpandIterator.h>
#include <app/ClusterInfo.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <app/InteractionModelDelegate.h>
#include <app/util/mock/Constants.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>

#include <nlunit-test.h>

using namespace chip;

namespace {

using P = app::ConcreteAttributePath;

void TestAllWildcard(nlTestSuite * apSuite, void * apContext)
{
    app::ClusterInfo clusInfo;

    app::ConcreteAttributePath path;
    P paths[] = {
        P(0xFFFE, 0xFFF1'0001, 0x0000'FFFD), P(0xFFFE, 0xFFF1'0001, 0x0000'FFFC), P(0xFFFE, 0xFFF1'0002, 0x0000'FFFD),
        P(0xFFFE, 0xFFF1'0002, 0x0000'FFFC), P(0xFFFE, 0xFFF1'0002, 0xFFF1'0001), P(0xFFFD, 0xFFF1'0001, 0x0000'FFFD),
        P(0xFFFD, 0xFFF1'0001, 0x0000'FFFC), P(0xFFFD, 0xFFF1'0002, 0x0000'FFFD), P(0xFFFD, 0xFFF1'0002, 0x0000'FFFC),
        P(0xFFFD, 0xFFF1'0002, 0xFFF1'0001), P(0xFFFD, 0xFFF1'0002, 0xFFF1'0002), P(0xFFFD, 0xFFF1'0003, 0x0000'FFFD),
        P(0xFFFD, 0xFFF1'0003, 0x0000'FFFC), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0001), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0002),
        P(0xFFFD, 0xFFF1'0003, 0xFFF1'0003), P(0xFFFC, 0xFFF1'0001, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0001, 0x0000'FFFC),
        P(0xFFFC, 0xFFF1'0001, 0xFFF1'0001), P(0xFFFC, 0xFFF1'0002, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0002, 0x0000'FFFC),
        P(0xFFFC, 0xFFF1'0002, 0xFFF1'0001), P(0xFFFC, 0xFFF1'0002, 0xFFF1'0002), P(0xFFFC, 0xFFF1'0002, 0xFFF1'0003),
        P(0xFFFC, 0xFFF1'0002, 0xFFF1'0004), P(0xFFFC, 0xFFF1'0003, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0003, 0x0000'FFFC),
        P(0xFFFC, 0xFFF1'0004, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0004, 0x0000'FFFC),
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Proceed())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04" PRIX16 " / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        // NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestWildcardEndpoint(nlTestSuite * apSuite, void * apContext)
{
    app::ClusterInfo clusInfo;
    clusInfo.mClusterId = Test::MockClusterId(3);
    clusInfo.mFieldId   = Test::MockAttributeId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        P(0xFFFD, 0xFFF1'0003, 0xFFF1'0003),
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Proceed())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04" PRIX16 " / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        // NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestWildcardCluster(nlTestSuite * apSuite, void * apContext)
{
    app::ClusterInfo clusInfo;
    clusInfo.mEndpointId = Test::kMockEndpoint3;
    clusInfo.mFieldId    = app::Clusters::Globals::Attributes::ClusterRevision::Id;

    app::ConcreteAttributePath path;
    P paths[] = {
        P(0xFFFC, 0xFFF1'0001, 0x0000'FFFD),
        P(0xFFFC, 0xFFF1'0002, 0x0000'FFFD),
        P(0xFFFC, 0xFFF1'0003, 0x0000'FFFD),
        P(0xFFFC, 0xFFF1'0004, 0x0000'FFFD),
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Proceed())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04" PRIX16 " / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        // NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestWildcardAttribute(nlTestSuite * apSuite, void * apContext)
{
    app::ClusterInfo clusInfo;
    clusInfo.mEndpointId = Test::kMockEndpoint2;
    clusInfo.mClusterId  = Test::MockClusterId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        P(0xFFFD, 0xFFF1'0003, 0x0000'FFFD), P(0xFFFD, 0xFFF1'0003, 0x0000'FFFC), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0001),
        P(0xFFFD, 0xFFF1'0003, 0xFFF1'0002), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0003),
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Proceed())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04" PRIX16 " / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        // NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestNoWildcard(nlTestSuite * apSuite, void * apContext)
{
    app::ClusterInfo clusInfo;
    clusInfo.mEndpointId = Test::kMockEndpoint2;
    clusInfo.mClusterId  = Test::MockClusterId(3);
    clusInfo.mFieldId    = Test::MockAttributeId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        P(0xFFFD, 0xFFF1'0003, 0xFFF1'0003),
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Proceed())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04" PRIX16 " / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        // NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestMultipleClusInfo(nlTestSuite * apSuite, void * apContext)
{

    app::ClusterInfo clusInfo1;

    app::ClusterInfo clusInfo2;
    clusInfo2.mClusterId = Test::MockClusterId(3);
    clusInfo2.mFieldId   = Test::MockAttributeId(3);

    app::ClusterInfo clusInfo3;
    clusInfo3.mEndpointId = Test::kMockEndpoint3;
    clusInfo3.mFieldId    = app::Clusters::Globals::Attributes::ClusterRevision::Id;

    app::ClusterInfo clusInfo4;
    clusInfo4.mEndpointId = Test::kMockEndpoint2;
    clusInfo4.mClusterId  = Test::MockClusterId(3);

    app::ClusterInfo clusInfo5;
    clusInfo5.mEndpointId = Test::kMockEndpoint2;
    clusInfo5.mClusterId  = Test::MockClusterId(3);
    clusInfo5.mFieldId    = Test::MockAttributeId(3);

    clusInfo1.mpNext = &clusInfo2;
    clusInfo2.mpNext = &clusInfo3;
    clusInfo3.mpNext = &clusInfo4;
    clusInfo4.mpNext = &clusInfo5;

    app::ConcreteAttributePath path;
    P paths[] = {
        P(0xFFFE, 0xFFF1'0001, 0x0000'FFFD), P(0xFFFE, 0xFFF1'0001, 0x0000'FFFC), P(0xFFFE, 0xFFF1'0002, 0x0000'FFFD),
        P(0xFFFE, 0xFFF1'0002, 0x0000'FFFC), P(0xFFFE, 0xFFF1'0002, 0xFFF1'0001), P(0xFFFD, 0xFFF1'0001, 0x0000'FFFD),
        P(0xFFFD, 0xFFF1'0001, 0x0000'FFFC), P(0xFFFD, 0xFFF1'0002, 0x0000'FFFD), P(0xFFFD, 0xFFF1'0002, 0x0000'FFFC),
        P(0xFFFD, 0xFFF1'0002, 0xFFF1'0001), P(0xFFFD, 0xFFF1'0002, 0xFFF1'0002), P(0xFFFD, 0xFFF1'0003, 0x0000'FFFD),
        P(0xFFFD, 0xFFF1'0003, 0x0000'FFFC), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0001), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0002),
        P(0xFFFD, 0xFFF1'0003, 0xFFF1'0003), P(0xFFFC, 0xFFF1'0001, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0001, 0x0000'FFFC),
        P(0xFFFC, 0xFFF1'0001, 0xFFF1'0001), P(0xFFFC, 0xFFF1'0002, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0002, 0x0000'FFFC),
        P(0xFFFC, 0xFFF1'0002, 0xFFF1'0001), P(0xFFFC, 0xFFF1'0002, 0xFFF1'0002), P(0xFFFC, 0xFFF1'0002, 0xFFF1'0003),
        P(0xFFFC, 0xFFF1'0002, 0xFFF1'0004), P(0xFFFC, 0xFFF1'0003, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0003, 0x0000'FFFC),
        P(0xFFFC, 0xFFF1'0004, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0004, 0x0000'FFFC), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0003),
        P(0xFFFC, 0xFFF1'0001, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0002, 0x0000'FFFD), P(0xFFFC, 0xFFF1'0003, 0x0000'FFFD),
        P(0xFFFC, 0xFFF1'0004, 0x0000'FFFD), P(0xFFFD, 0xFFF1'0003, 0x0000'FFFD), P(0xFFFD, 0xFFF1'0003, 0x0000'FFFC),
        P(0xFFFD, 0xFFF1'0003, 0xFFF1'0001), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0002), P(0xFFFD, 0xFFF1'0003, 0xFFF1'0003),
        P(0xFFFD, 0xFFF1'0003, 0xFFF1'0003),
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo1); iter.Get(path); iter.Proceed())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04" PRIX16 " / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        // NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

static int TestSetup(void * inContext)
{
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    return SUCCESS;
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
        NL_TEST_DEF("TestAllWildcard", TestAllWildcard),
        NL_TEST_DEF("TestWildcardEndpoint", TestWildcardEndpoint),
        NL_TEST_DEF("TestWildcardCluster", TestWildcardCluster),
        NL_TEST_DEF("TestWildcardAttribute", TestWildcardAttribute),
        NL_TEST_DEF("TestNoWildcard", TestNoWildcard),
        NL_TEST_DEF("TestMultipleClusInfo", TestMultipleClusInfo),
        NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestAttributePathExpandIterator",
    &sTests[0],
    TestSetup,
    TestTeardown,
};
// clang-format on

} // namespace

int TestAttributePathExpandIterator()
{
    nlTestRunner(&sSuite, nullptr);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestAttributePathExpandIterator)
