/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/support/UnitTestRegistration.h>
#include <app/ConcreteAttributePath.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app;

namespace {

void TestConcreteAttributePathEqualityDefaultConstructor(nlTestSuite * aSuite, void * aContext)
{
    ConcreteAttributePath path_one;
    ConcreteAttributePath path_two;
    NL_TEST_ASSERT(aSuite, path_one == path_two);
}

void TestConcreteAttributePathEquality(nlTestSuite * aSuite, void * aContext)
{
    ConcreteAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    NL_TEST_ASSERT(aSuite, path_one == path_two);
}

void TestConcreteAttributePathInequalityDifferentAttributeId(nlTestSuite * aSuite, void * aContext)
{
    ConcreteAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/4);
    NL_TEST_ASSERT(aSuite, path_one != path_two);
}

void TestConcreteReadAttributePathEqualityDefaultConstructor(nlTestSuite* aSuite, void * aContext) {
    ConcreteReadAttributePath path_one;
    ConcreteReadAttributePath path_two;
    NL_TEST_ASSERT(aSuite, path_one == path_two);
}

void TestConcreteReadAttributePathInequalityDifferentListIndex(nlTestSuite* aSuite, void * aContext) {
    ConcreteReadAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aListIndex=*/4);
    ConcreteReadAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aListIndex=*/5);
    NL_TEST_ASSERT(aSuite, path_one != path_two);
}

void TestConcreteDataAttributePathEqualityDefaultConstructor(nlTestSuite* aSuite, void * aContext) {
    ConcreteDataAttributePath path_one;
    ConcreteDataAttributePath path_two;
    NL_TEST_ASSERT(aSuite, path_one == path_two);
}

void TestConcreteDataAttributePathInequalityDifferentDataVersion(nlTestSuite* aSuite, void * aContext) {
    ConcreteDataAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(4));
    ConcreteDataAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(5));
    NL_TEST_ASSERT(aSuite, path_one != path_two);
}

void TestConcreteDataAttributePathInequalityDifferentListOp(nlTestSuite* aSuite, void * aContext) {
    ConcreteDataAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/4);
    ConcreteDataAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, ConcreteDataAttributePath::ListOperation::ReplaceItem, /*aListIndex=*/4);
    NL_TEST_ASSERT(aSuite, path_one != path_two);
}

void TestConcreteDataAttributePathInequalityDifferentListIndex(nlTestSuite* aSuite, void * aContext) {
    ConcreteDataAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, ConcreteDataAttributePath::ListOperation::NotList, /*aListIndex=*/4);
    ConcreteDataAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, ConcreteDataAttributePath::ListOperation::NotList, /*aListIndex=*/5);
    NL_TEST_ASSERT(aSuite, path_one != path_two);
}

const nlTest sTests[] = {
  NL_TEST_DEF("TestConcreteAttributePathEqualityDefaultConstructor", TestConcreteAttributePathEqualityDefaultConstructor),
  NL_TEST_DEF("TestConcreteAttributePathEquality", TestConcreteAttributePathEquality),
  NL_TEST_DEF("TestConcreteAttributePathInequalityDifferentAttributeId", TestConcreteAttributePathInequalityDifferentAttributeId),
  NL_TEST_DEF("TestConcreteReadAttributePathEqualityDefaultConstructor", TestConcreteReadAttributePathEqualityDefaultConstructor),
  NL_TEST_DEF("TestConcreteReadAttributePathInequalityDifferentListIndex", TestConcreteReadAttributePathInequalityDifferentListIndex),
  NL_TEST_DEF("TestConcreteDataAttributePathEqualityDefaultConstructor", TestConcreteDataAttributePathEqualityDefaultConstructor),
  NL_TEST_DEF("TestConcreteDataAttributePathInequalityDifferentDataVersion", TestConcreteDataAttributePathInequalityDifferentDataVersion),
  NL_TEST_DEF("TestConcreteDataAttributePathInequalityDifferentListOp", TestConcreteDataAttributePathInequalityDifferentListOp),
  NL_TEST_DEF("TestConcreteDataAttributePathInequalityDifferentListIndex", TestConcreteDataAttributePathInequalityDifferentListIndex),
  NL_TEST_SENTINEL()
};

} // anonymous namespace

int TestConcreteAttributePath()
{
    nlTestSuite theSuite = { "ConcreteAttributePath", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestConcreteAttributePath)
