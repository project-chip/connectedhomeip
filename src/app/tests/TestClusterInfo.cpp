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
void TestDirty(nlTestSuite * apSuite, void * apContext)
{
    ClusterInfo clusterInfo1;
    clusterInfo1.SetDirty();
    NL_TEST_ASSERT(apSuite, clusterInfo1.IsDirty());
    clusterInfo1.ClearDirty();
    NL_TEST_ASSERT(apSuite, !clusterInfo1.IsDirty());
}

void TestAttributePathSelectorLinkList(nlTestSuite * apSuite, void * apContext)
{
    int number = 0;
    ClusterInfo clusterInfo1;
    clusterInfo1.PushAttributePathSelectorHead();
    clusterInfo1.PushAttributePathSelectorHead();
    AttributePathSelector * current = clusterInfo1.mpAttributePathSelector;
    while (current != nullptr)
    {
        number ++;
        current = current->mpNext;
    }
    NL_TEST_ASSERT(apSuite, number == 2);
    clusterInfo1.PopAllAttributePathSelector();
    NL_TEST_ASSERT(apSuite, number == 0);
}
} // namespace TestClusterInfo
} // namespace app
} // namespace chip

namespace {
const nlTest sTests[] = { NL_TEST_DEF("TestDirty", chip::app::TestClusterInfo::TestDirty), NL_TEST_SENTINEL(),
                          NL_TEST_DEF("TestAttributePathSelectorLinkList", chip::app::TestClusterInfo::TestAttributePathSelectorLinkList), NL_TEST_SENTINEL() };
}

int TestClusterInfo()
{
    nlTestSuite theSuite = { "ClusterInfo", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestClusterInfo)
