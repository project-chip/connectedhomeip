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
#include <nlunit-test.h>
#include <support/UnitTestRegistration.h>

namespace chip {
namespace app {
namespace TestClusterInfo {
void TestSamePath(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    ClusterInfo clusterInfo1(attributePathParams1, false);
    ClusterInfo clusterInfo2(attributePathParams2, false);
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsSamePath(clusterInfo2));
}

void TestDifferentPath(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(6, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    ClusterInfo clusterInfo1(attributePathParams1, false);
    ClusterInfo clusterInfo2(attributePathParams2, false);
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsSamePath(clusterInfo2));
}

void TestDirty(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    ClusterInfo clusterInfo1(attributePathParams1, false);
    clusterInfo1.SetDirty();
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsDirty());
    clusterInfo1.ClearDirty();
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsDirty());
}
} // namespace TestClusterInfo
} // namespace app
} // namespace chip

namespace {
const nlTest sTests[] = { NL_TEST_DEF("TestSamePath", chip::app::TestClusterInfo::TestSamePath),
                          NL_TEST_DEF("TestDifferentPath", chip::app::TestClusterInfo::TestDifferentPath),
                          NL_TEST_DEF("TestDirtiness", chip::app::TestClusterInfo::TestDirty), NL_TEST_SENTINEL() };
}

int TestClusterInfo()
{
    nlTestSuite theSuite = { "ClusterInfo", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestClusterInfo)
