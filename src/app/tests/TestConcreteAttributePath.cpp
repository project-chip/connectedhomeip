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

#include <app/ConcreteAttributePath.h>
#include <lib/support/UnitTestRegistration.h>
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

void TestConcreteDataAttributePathMatchesConcreteAttributePathEquality(nlTestSuite * aSuite, void * aContext)
{
    ConcreteAttributePath path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath data_path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath data_path_with_version(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                                     /*aDataVersion=*/MakeOptional(4U));
    ConcreteDataAttributePath data_path_with_list(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                                  /*aListOp=*/ConcreteDataAttributePath::ListOperation::ReplaceAll,
                                                  /*aListIndex=*/5U);

    NL_TEST_ASSERT(aSuite, data_path.MatchesConcreteAttributePath(path));
    NL_TEST_ASSERT(aSuite, data_path_with_version.MatchesConcreteAttributePath(path));
    NL_TEST_ASSERT(aSuite, data_path_with_list.MatchesConcreteAttributePath(path));
}

void TestConcreteDataAttributePathMatchesConcreteAttributePathInequality(nlTestSuite * aSuite, void * aContext)
{
    ConcreteAttributePath path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath data_path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/4);

    NL_TEST_ASSERT(aSuite, !data_path.MatchesConcreteAttributePath(path));
}

void TestConcreteDataAttributePathEqualityDefaultConstructor(nlTestSuite * aSuite, void * aContext)
{
    ConcreteDataAttributePath one;
    ConcreteDataAttributePath two;
    NL_TEST_ASSERT(aSuite, one == two);
}

void TestConcreteDataAttributePathEqualityConcreteAttributePathConstructor(nlTestSuite * aSuite, void * aContext)
{
    ConcreteAttributePath path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath one(path);
    ConcreteDataAttributePath two(path);
    NL_TEST_ASSERT(aSuite, one == two);
}

void TestConcreteDataAttributePathInequalityConcreteAttributePathConstructorDifferentAttributeId(nlTestSuite * aSuite,
                                                                                                 void * aContext)
{
    ConcreteAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/4);
    ConcreteDataAttributePath one(path_one);
    ConcreteDataAttributePath two(path_two);
    NL_TEST_ASSERT(aSuite, one != two);
}

void TestConcreteDataAttributePathEqualityConcreteAttributePathArgsConstructor(nlTestSuite * aSuite, void * aContext)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    NL_TEST_ASSERT(aSuite, one == two);
}

void TestConcreteDataAttributePathInequalityConcreteAttributePathArgsConstructorDifferentAttributeId(nlTestSuite * aSuite,
                                                                                                     void * aContext)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/4);
    NL_TEST_ASSERT(aSuite, one != two);
}

void TestConcreteDataAttributePathEqualityDataVersionConstructor(nlTestSuite * aSuite, void * aContext)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(4U));
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(4U));
    NL_TEST_ASSERT(aSuite, one == two);
}

void TestConcreteDataAttributePathInequalityDataVersionConstructorDifferentDataVersion(nlTestSuite * aSuite, void * aContext)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(4U));
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(5U));
    NL_TEST_ASSERT(aSuite, one != two);
}

void TestConcreteDataAttributePathEqualityListConstructor(nlTestSuite * aSuite, void * aContext)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/5);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/5);
    NL_TEST_ASSERT(aSuite, one == two);
}

void TestConcreteDataAttributePathInequalityListConstructorDifferentListOp(nlTestSuite * aSuite, void * aContext)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/5);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceItem, /*aListIndex=*/5);
    NL_TEST_ASSERT(aSuite, one != two);
}

void TestConcreteDataAttributePathInequalityListConstructorDifferentListIndex(nlTestSuite * aSuite, void * aContext)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/5);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/6);
    NL_TEST_ASSERT(aSuite, one != two);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestConcreteAttributePathEqualityDefaultConstructor", TestConcreteAttributePathEqualityDefaultConstructor),
    NL_TEST_DEF("TestConcreteAttributePathEquality", TestConcreteAttributePathEquality),
    NL_TEST_DEF("TestConcreteAttributePathInequalityDifferentAttributeId", TestConcreteAttributePathInequalityDifferentAttributeId),
    NL_TEST_DEF("TestConcreteDataAttributePathMatchesConcreteAttributePathEquality",
                TestConcreteDataAttributePathMatchesConcreteAttributePathEquality),
    NL_TEST_DEF("TestConcreteDataAttributePathMatchesConcreteAttributePathInequality",
                TestConcreteDataAttributePathMatchesConcreteAttributePathInequality),
    NL_TEST_DEF("TestConcreteDataAttributePathEqualityDefaultConstructor", TestConcreteDataAttributePathEqualityDefaultConstructor),
    NL_TEST_DEF("TestConcreteDataAttributePathEqualityConcreteAttributePathConstructor",
                TestConcreteDataAttributePathEqualityConcreteAttributePathConstructor),
    NL_TEST_DEF("TestConcreteDataAttributePathInequalityConcreteAttributePathConstructorDifferentAttributeId",
                TestConcreteDataAttributePathInequalityConcreteAttributePathConstructorDifferentAttributeId),
    NL_TEST_DEF("TestConcreteDataAttributePathEqualityConcreteAttributePathArgsConstructor",
                TestConcreteDataAttributePathEqualityConcreteAttributePathArgsConstructor),
    NL_TEST_DEF("TestConcreteDataAttributePathInequalityConcreteAttributePathArgsConstructorDifferentAttributeId",
                TestConcreteDataAttributePathInequalityConcreteAttributePathArgsConstructorDifferentAttributeId),
    NL_TEST_DEF("TestConcreteDataAttributePathEqualityDataVersionConstructor",
                TestConcreteDataAttributePathEqualityDataVersionConstructor),
    NL_TEST_DEF("TestConcreteDataAttributePathInequalityDataVersionConstructorDifferentDataVersion",
                TestConcreteDataAttributePathInequalityDataVersionConstructorDifferentDataVersion),
    NL_TEST_DEF("TestConcreteDataAttributePathEqualityListConstructor", TestConcreteDataAttributePathEqualityListConstructor),
    NL_TEST_DEF("TestConcreteDataAttributePathInequalityListConstructorDifferentListOp",
                TestConcreteDataAttributePathInequalityListConstructorDifferentListOp),
    NL_TEST_DEF("TestConcreteDataAttributePathInequalityListConstructorDifferentListIndex",
                TestConcreteDataAttributePathInequalityListConstructorDifferentListIndex),
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
