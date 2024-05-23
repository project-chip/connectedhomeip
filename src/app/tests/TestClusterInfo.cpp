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
 *      This file implements unit tests for CommandPathParams
 *
 */

#include <app/AttributePathParams.h>
#include <app/DataVersionFilter.h>
#include <app/EventPathParams.h>
#include <app/util/mock/Constants.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip::Test;

namespace chip {
namespace app {
namespace TestPath {
void TestAttributePathIntersect(nlTestSuite * apSuite, void * apContext)
{
    EndpointId endpointIdArray[2]   = { 1, kInvalidEndpointId };
    ClusterId clusterIdArray[2]     = { 2, kInvalidClusterId };
    AttributeId attributeIdArray[2] = { 3, kInvalidAttributeId };

    for (auto endpointId1 : endpointIdArray)
    {
        for (auto clusterId1 : clusterIdArray)
        {
            for (auto attributeId1 : attributeIdArray)
            {
                for (auto endpointId2 : endpointIdArray)
                {
                    for (auto clusterId2 : clusterIdArray)
                    {
                        for (auto attributeId2 : attributeIdArray)
                        {
                            AttributePathParams path1;
                            path1.mEndpointId  = endpointId1;
                            path1.mClusterId   = clusterId1;
                            path1.mAttributeId = attributeId1;
                            AttributePathParams path2;
                            path2.mEndpointId  = endpointId2;
                            path2.mClusterId   = clusterId2;
                            path2.mAttributeId = attributeId2;
                            NL_TEST_ASSERT(apSuite, path1.Intersects(path2));
                        }
                    }
                }
            }
        }
    }

    {
        AttributePathParams path1;
        path1.mEndpointId = 1;
        AttributePathParams path2;
        path2.mEndpointId = 2;
        NL_TEST_ASSERT(apSuite, !path1.Intersects(path2));
    }

    {
        AttributePathParams path1;
        path1.mClusterId = 1;
        AttributePathParams path2;
        path2.mClusterId = 2;
        NL_TEST_ASSERT(apSuite, !path1.Intersects(path2));
    }

    {
        AttributePathParams path1;
        path1.mAttributeId = 1;
        AttributePathParams path2;
        path2.mAttributeId = 2;
        NL_TEST_ASSERT(apSuite, !path1.Intersects(path2));
    }
}

void TestAttributePathIncludedSameFieldId(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams clusterInfo1;
    AttributePathParams clusterInfo2;
    AttributePathParams clusterInfo3;
    clusterInfo1.mAttributeId = 1;
    clusterInfo2.mAttributeId = 1;
    clusterInfo3.mAttributeId = 1;
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    clusterInfo2.mListIndex = 1;
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    clusterInfo1.mListIndex = 0;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo3));
    clusterInfo3.mListIndex = 0;
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo3));
    clusterInfo3.mListIndex = 1;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo3));
}

void TestAttributePathIncludedDifferentFieldId(nlTestSuite * apSuite, void * apContext)
{
    {
        AttributePathParams clusterInfo1;
        AttributePathParams clusterInfo2;
        clusterInfo1.mAttributeId = 1;
        clusterInfo2.mAttributeId = 2;
        NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    }
    {
        AttributePathParams clusterInfo1;
        AttributePathParams clusterInfo2;
        clusterInfo2.mAttributeId = 2;
        NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    }
    {
        AttributePathParams clusterInfo1;
        AttributePathParams clusterInfo2;
        NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    }
    {
        AttributePathParams clusterInfo1;
        AttributePathParams clusterInfo2;

        clusterInfo1.mAttributeId = 1;
        NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    }
}

void TestAttributePathIncludedDifferentEndpointId(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams clusterInfo1;
    AttributePathParams clusterInfo2;
    clusterInfo1.mEndpointId = 1;
    clusterInfo2.mEndpointId = 2;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
}

void TestAttributePathIncludedDifferentClusterId(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams clusterInfo1;
    AttributePathParams clusterInfo2;
    clusterInfo1.mClusterId = 1;
    clusterInfo2.mClusterId = 2;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
}

/*
{kInvalidEndpointId, kInvalidClusterId, kInvalidEventId},
{kInvalidEndpointId, MockClusterId(1), kInvalidEventId},
{kInvalidEndpointId, MockClusterId(1), MockEventId(1)},
{kMockEndpoint1, kInvalidClusterId, kInvalidEventId},
{kMockEndpoint1, MockClusterId(1), kInvalidEventId},
{kMockEndpoint1, MockClusterId(1), MockEventId(1)},
*/
chip::app::EventPathParams validEventpaths[6];
void InitEventPaths()
{
    validEventpaths[1].mClusterId  = MockClusterId(1);
    validEventpaths[2].mClusterId  = MockClusterId(1);
    validEventpaths[2].mEventId    = MockEventId(1);
    validEventpaths[3].mEndpointId = kMockEndpoint1;
    validEventpaths[4].mEndpointId = kMockEndpoint1;
    validEventpaths[4].mClusterId  = MockClusterId(1);
    validEventpaths[5].mEndpointId = kMockEndpoint1;
    validEventpaths[5].mClusterId  = MockClusterId(1);
    validEventpaths[5].mEventId    = MockEventId(1);
}

void TestEventPathSameEventId(nlTestSuite * apSuite, void * apContext)
{
    ConcreteEventPath testPath(kMockEndpoint1, MockClusterId(1), MockEventId(1));
    for (auto & path : validEventpaths)
    {
        NL_TEST_ASSERT(apSuite, path.IsValidEventPath());
        NL_TEST_ASSERT(apSuite, path.IsEventPathSupersetOf(testPath));
    }
}

void TestEventPathDifferentEventId(nlTestSuite * apSuite, void * apContext)
{
    ConcreteEventPath testPath(kMockEndpoint1, MockClusterId(1), MockEventId(2));
    NL_TEST_ASSERT(apSuite, validEventpaths[0].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, validEventpaths[1].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[2].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, validEventpaths[3].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, validEventpaths[4].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[5].IsEventPathSupersetOf(testPath));
}

void TestEventPathDifferentClusterId(nlTestSuite * apSuite, void * apContext)
{
    ConcreteEventPath testPath(kMockEndpoint1, MockClusterId(2), MockEventId(1));
    NL_TEST_ASSERT(apSuite, validEventpaths[0].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[1].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[2].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, validEventpaths[3].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[4].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[5].IsEventPathSupersetOf(testPath));
}

void TestEventPathDifferentEndpointId(nlTestSuite * apSuite, void * apContext)
{
    ConcreteEventPath testPath(kMockEndpoint2, MockClusterId(1), MockEventId(1));
    NL_TEST_ASSERT(apSuite, validEventpaths[0].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, validEventpaths[1].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, validEventpaths[2].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[3].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[4].IsEventPathSupersetOf(testPath));
    NL_TEST_ASSERT(apSuite, !validEventpaths[5].IsEventPathSupersetOf(testPath));
}

} // namespace TestPath
} // namespace app
} // namespace chip

namespace {
const nlTest sTests[] = {
    NL_TEST_DEF("TestAttributePathIncludedSameFieldId", chip::app::TestPath::TestAttributePathIncludedSameFieldId),
    NL_TEST_DEF("TestAttributePathIncludedDifferentFieldId", chip::app::TestPath::TestAttributePathIncludedDifferentFieldId),
    NL_TEST_DEF("TestAttributePathIncludedDifferentEndpointId", chip::app::TestPath::TestAttributePathIncludedDifferentEndpointId),
    NL_TEST_DEF("TestAttributePathIncludedDifferentClusterId", chip::app::TestPath::TestAttributePathIncludedDifferentClusterId),
    NL_TEST_DEF("TestEventPathSameEventId", chip::app::TestPath::TestEventPathSameEventId),
    NL_TEST_DEF("TestEventPathDifferentEventId", chip::app::TestPath::TestEventPathDifferentEventId),
    NL_TEST_DEF("TestEventPathDifferentClusterId", chip::app::TestPath::TestEventPathDifferentClusterId),
    NL_TEST_DEF("TestEventPathDifferentEndpointId", chip::app::TestPath::TestEventPathDifferentEndpointId),
    NL_TEST_DEF("TestAttributePathIntersect", chip::app::TestPath::TestAttributePathIntersect),
    NL_TEST_SENTINEL()
};
}

int TestPath()
{
    nlTestSuite theSuite = { "TestPath", &sTests[0], nullptr, nullptr };
    chip::app::TestPath::InitEventPaths();
    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPath)
