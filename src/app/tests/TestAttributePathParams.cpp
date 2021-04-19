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
#include <nlunit-test.h>
#include <support/UnitTestRegistration.h>

namespace chip {
namespace app {
namespace TestAttributePathParams {
void TestSamePath(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    NL_TEST_ASSERT(apSuite, attributePathParams1.IsSamePath(attributePathParams2));
}

void TestDifferentNodeId(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(6, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    NL_TEST_ASSERT(apSuite, !attributePathParams1.IsSamePath(attributePathParams2));
}

void TestDifferentEndpointId(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(1, 6, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    NL_TEST_ASSERT(apSuite, !attributePathParams1.IsSamePath(attributePathParams2));
}

void TestDifferentClusterId(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(1, 2, 6, 4, 5, AttributePathFlags::kFieldIdValid);
    NL_TEST_ASSERT(apSuite, !attributePathParams1.IsSamePath(attributePathParams2));
}

void TestDifferentFieldId(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(1, 2, 3, 6, 5, AttributePathFlags::kFieldIdValid);
    NL_TEST_ASSERT(apSuite, !attributePathParams1.IsSamePath(attributePathParams2));
}

void TestDifferentListIndex(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kListIndexValid);
    AttributePathParams attributePathParams2(1, 2, 3, 4, 6, AttributePathFlags::kListIndexValid);
    NL_TEST_ASSERT(apSuite, !attributePathParams1.IsSamePath(attributePathParams2));
}

void TestDifferentPathFlag(nlTestSuite * apSuite, void * apContext)
{
    AttributePathParams attributePathParams1(1, 2, 3, 4, 5, AttributePathFlags::kFieldIdValid);
    AttributePathParams attributePathParams2(1, 2, 3, 4, 5, AttributePathFlags::kListIndexValid);
    NL_TEST_ASSERT(apSuite, !attributePathParams1.IsSamePath(attributePathParams2));
}
} // namespace TestAttributePathParams
} // namespace app
} // namespace chip

namespace {
const nlTest sTests[] = { NL_TEST_DEF("TestSamePath", chip::app::TestAttributePathParams::TestSamePath),
                          NL_TEST_DEF("TestDifferentNodeId", chip::app::TestAttributePathParams::TestDifferentNodeId),
                          NL_TEST_DEF("TestDifferentEndpointId", chip::app::TestAttributePathParams::TestDifferentEndpointId),
                          NL_TEST_DEF("TestDifferentClusterId", chip::app::TestAttributePathParams::TestDifferentClusterId),
                          NL_TEST_DEF("TestDifferentFieldId", chip::app::TestAttributePathParams::TestDifferentFieldId),
                          NL_TEST_DEF("TestDifferentListIndex", chip::app::TestAttributePathParams::TestDifferentListIndex),
                          NL_TEST_DEF("TestDifferentPathFlag", chip::app::TestAttributePathParams::TestDifferentPathFlag),
                          NL_TEST_SENTINEL() };
}

int TestAttributePathParams()
{
    nlTestSuite theSuite = { "AttributePathParams", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestAttributePathParams)
