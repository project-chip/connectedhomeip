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

#include <app/EventPathParams.h>
#include <app/util/mock/Constants.h>
#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using namespace chip::Test;

namespace chip {
namespace app {
namespace TestEventPathParams {

class TestEventPathParams : public ::testing::Test
{
public:
    static void SetUpTestSuite() { InitEventPaths(); }

    static void InitEventPaths();

    static chip::app::EventPathParams validEventpaths[6];
};

chip::app::EventPathParams TestEventPathParams::validEventpaths[6];

TEST_F(TestEventPathParams, SamePath)
{
    EventPathParams eventPathParams1(2, 3, 4);
    EventPathParams eventPathParams2(2, 3, 4);
    EXPECT_TRUE(eventPathParams1.IsSamePath(eventPathParams2));
}

TEST_F(TestEventPathParams, DifferentEndpointId)
{
    EventPathParams eventPathParams1(2, 3, 4);
    EventPathParams eventPathParams2(6, 3, 4);
    EXPECT_FALSE(eventPathParams1.IsSamePath(eventPathParams2));
}

TEST_F(TestEventPathParams, DifferentClusterId)
{
    EventPathParams eventPathParams1(2, 3, 4);
    EventPathParams eventPathParams2(2, 6, 4);
    EXPECT_FALSE(eventPathParams1.IsSamePath(eventPathParams2));
}

TEST_F(TestEventPathParams, DifferentEventId)
{
    EventPathParams eventPathParams1(2, 3, 4);
    EventPathParams eventPathParams2(2, 3, 6);
    EXPECT_FALSE(eventPathParams1.IsSamePath(eventPathParams2));
}

/* after Init, validEventpaths array will have the following values:
{kInvalidEndpointId, kInvalidClusterId, kInvalidEventId},
{kInvalidEndpointId, MockClusterId(1), kInvalidEventId},
{kInvalidEndpointId, MockClusterId(1), MockEventId(1)},
{kMockEndpoint1, kInvalidClusterId, kInvalidEventId},
{kMockEndpoint1, MockClusterId(1), kInvalidEventId},
{kMockEndpoint1, MockClusterId(1), MockEventId(1)},
*/
void TestEventPathParams::InitEventPaths()
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

TEST_F(TestEventPathParams, ConcreteEventPathSameEventId)
{
    ConcreteEventPath testPath(kMockEndpoint1, MockClusterId(1), MockEventId(1));
    for (auto & path : validEventpaths)
    {
        EXPECT_TRUE(path.IsValidEventPath());
        EXPECT_TRUE(path.IsEventPathSupersetOf(testPath));
    }
}
TEST_F(TestEventPathParams, ConcreteEventPathDifferentEndpointId)
{
    ConcreteEventPath testPath(kMockEndpoint2, MockClusterId(1), MockEventId(1));
    EXPECT_TRUE(validEventpaths[0].IsEventPathSupersetOf(testPath));
    EXPECT_TRUE(validEventpaths[1].IsEventPathSupersetOf(testPath));
    EXPECT_TRUE(validEventpaths[2].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[3].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[4].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[5].IsEventPathSupersetOf(testPath));
}

TEST_F(TestEventPathParams, ConcreteEventPathDifferentClusterId)
{
    ConcreteEventPath testPath(kMockEndpoint1, MockClusterId(2), MockEventId(1));
    EXPECT_TRUE(validEventpaths[0].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[1].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[2].IsEventPathSupersetOf(testPath));
    EXPECT_TRUE(validEventpaths[3].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[4].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[5].IsEventPathSupersetOf(testPath));
}

TEST_F(TestEventPathParams, ConcreteEventPathDifferentEventId)
{
    ConcreteEventPath testPath(kMockEndpoint1, MockClusterId(1), MockEventId(2));
    EXPECT_TRUE(validEventpaths[0].IsEventPathSupersetOf(testPath));
    EXPECT_TRUE(validEventpaths[1].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[2].IsEventPathSupersetOf(testPath));
    EXPECT_TRUE(validEventpaths[3].IsEventPathSupersetOf(testPath));
    EXPECT_TRUE(validEventpaths[4].IsEventPathSupersetOf(testPath));
    EXPECT_FALSE(validEventpaths[5].IsEventPathSupersetOf(testPath));
}

} // namespace TestEventPathParams
} // namespace app
} // namespace chip
