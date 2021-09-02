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

#include <app/ClusterInfo.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

namespace chip {
namespace app {
namespace TestClusterInfo {
void TestDirty(nlTestSuite * apSuite, void * apContext)
{
    ClusterInfo clusterInfo1;
    clusterInfo1.SetDirty();
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsDirty());
    clusterInfo1.ClearDirty();
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsDirty());
}

void TestAttributePathIncludedSameFieldId(nlTestSuite * apSuite, void * apContext)
{
    ClusterInfo clusterInfo1;
    ClusterInfo clusterInfo2;
    ClusterInfo clusterInfo3;
    clusterInfo1.mFlags.Set(ClusterInfo::Flags::kFieldIdValid);
    clusterInfo2.mFlags.Set(ClusterInfo::Flags::kFieldIdValid);
    clusterInfo3.mFlags.Set(ClusterInfo::Flags::kFieldIdValid);
    clusterInfo1.mFieldId = 1;
    clusterInfo2.mFieldId = 1;
    clusterInfo3.mFieldId = 1;
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    clusterInfo2.mFlags.Set(ClusterInfo::Flags::kListIndexValid);
    clusterInfo2.mListIndex = 1;
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    clusterInfo1.mFlags.Set(ClusterInfo::Flags::kListIndexValid);
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo3));
    clusterInfo3.mFlags.Set(ClusterInfo::Flags::kListIndexValid);
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo3));
    clusterInfo3.mListIndex = 1;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo3));
}

void TestAttributePathIncludedDifferentFieldId(nlTestSuite * apSuite, void * apContext)
{
    ClusterInfo clusterInfo1;
    ClusterInfo clusterInfo2;
    clusterInfo1.mFlags.Set(ClusterInfo::Flags::kFieldIdValid);
    clusterInfo2.mFlags.Set(ClusterInfo::Flags::kFieldIdValid);
    clusterInfo1.mFieldId = 1;
    clusterInfo2.mFieldId = 2;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    clusterInfo1.mFieldId = 0xFFFFFFFF;
    clusterInfo2.mFieldId = 2;
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    clusterInfo1.mFieldId = 0xFFFFFFFF;
    clusterInfo2.mFieldId = 0xFFFFFFFF;
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
    clusterInfo1.mFieldId = 1;
    clusterInfo2.mFieldId = 0xFFFFFFFF;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
}

void TestAttributePathIncludedDifferentEndpointId(nlTestSuite * apSuite, void * apContext)
{
    ClusterInfo clusterInfo1;
    ClusterInfo clusterInfo2;
    clusterInfo1.mEndpointId = 1;
    clusterInfo2.mEndpointId = 2;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
}

void TestAttributePathIncludedDifferentClusterId(nlTestSuite * apSuite, void * apContext)
{
    ClusterInfo clusterInfo1;
    ClusterInfo clusterInfo2;
    clusterInfo1.mClusterId = 1;
    clusterInfo2.mClusterId = 2;
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsAttributePathSupersetOf(clusterInfo2));
}
} // namespace TestClusterInfo
} // namespace app
} // namespace chip

namespace {
const nlTest sTests[] = {
    NL_TEST_DEF("TestDirty", chip::app::TestClusterInfo::TestDirty),
    NL_TEST_DEF("TestAttributePathIncludedSameFieldId", chip::app::TestClusterInfo::TestAttributePathIncludedSameFieldId),
    NL_TEST_DEF("TestAttributePathIncludedDifferentFieldId", chip::app::TestClusterInfo::TestAttributePathIncludedDifferentFieldId),
    NL_TEST_DEF("TestAttributePathIncludedDifferentEndpointId",
                chip::app::TestClusterInfo::TestAttributePathIncludedDifferentEndpointId),
    NL_TEST_DEF("TestAttributePathIncludedDifferentClusterId",
                chip::app::TestClusterInfo::TestAttributePathIncludedDifferentClusterId),
    NL_TEST_SENTINEL()
};
}

int TestClusterInfo()
{
    nlTestSuite theSuite = { "ClusterInfo", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestClusterInfo)
