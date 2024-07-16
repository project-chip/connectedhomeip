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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

namespace {

TEST(TestConcreteAttributePath, TestConcreteAttributePathEqualityDefaultConstructor)
{
    ConcreteAttributePath path_one;
    ConcreteAttributePath path_two;
    EXPECT_EQ(path_one, path_two);
}

TEST(TestConcreteAttributePath, TestConcreteAttributePathEquality)
{
    ConcreteAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    EXPECT_EQ(path_one, path_two);
}

TEST(TestConcreteAttributePath, TestConcreteAttributePathInequalityDifferentAttributeId)
{
    ConcreteAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/4);
    EXPECT_NE(path_one, path_two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathMatchesConcreteAttributePathEquality)
{
    ConcreteAttributePath path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath data_path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath data_path_with_version(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                                     /*aDataVersion=*/MakeOptional(4U));
    ConcreteDataAttributePath data_path_with_list(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                                  /*aListOp=*/ConcreteDataAttributePath::ListOperation::ReplaceAll,
                                                  /*aListIndex=*/5U);

    EXPECT_TRUE(data_path.MatchesConcreteAttributePath(path));
    EXPECT_TRUE(data_path_with_version.MatchesConcreteAttributePath(path));
    EXPECT_TRUE(data_path_with_list.MatchesConcreteAttributePath(path));
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathMatchesConcreteAttributePathInequality)
{
    ConcreteAttributePath path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath data_path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/4);

    EXPECT_FALSE(data_path.MatchesConcreteAttributePath(path));
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathEqualityDefaultConstructor)
{
    ConcreteDataAttributePath one;
    ConcreteDataAttributePath two;
    EXPECT_EQ(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathEqualityConcreteAttributePathConstructor)
{
    ConcreteAttributePath path(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath one(path);
    ConcreteDataAttributePath two(path);
    EXPECT_EQ(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathInequalityConcreteAttributePathConstructorDifferentAttributeId)
{
    ConcreteAttributePath path_one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteAttributePath path_two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/4);
    ConcreteDataAttributePath one(path_one);
    ConcreteDataAttributePath two(path_two);
    EXPECT_NE(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathEqualityConcreteAttributePathArgsConstructor)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    EXPECT_EQ(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathInequalityConcreteAttributePathArgsConstructorDifferentAttributeId)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/4);
    EXPECT_NE(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathEqualityDataVersionConstructor)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(4U));
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(4U));
    EXPECT_EQ(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathInequalityDataVersionConstructorDifferentDataVersion)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(4U));
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3, /*aDataVersion=*/MakeOptional(5U));
    EXPECT_NE(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathEqualityListConstructor)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/5);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/5);
    EXPECT_EQ(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathInequalityListConstructorDifferentListOp)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/5);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceItem, /*aListIndex=*/5);
    EXPECT_NE(one, two);
}

TEST(TestConcreteAttributePath, TestConcreteDataAttributePathInequalityListConstructorDifferentListIndex)
{
    ConcreteDataAttributePath one(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/5);
    ConcreteDataAttributePath two(/*aEndpointId=*/1, /*aClusterId=*/2, /*aAttributeId=*/3,
                                  ConcreteDataAttributePath::ListOperation::ReplaceAll, /*aListIndex=*/6);
    EXPECT_NE(one, two);
}

} // anonymous namespace
