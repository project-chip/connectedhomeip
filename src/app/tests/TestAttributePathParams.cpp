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

#include <app/AttributePathParams.h>
#include <app/DataVersionFilter.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace chip {
namespace app {
namespace TestPath {

TEST(TestAttributePathParams, TestAttributePathIntersect)
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
                            EXPECT_TRUE(path1.Intersects(path2));
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
        EXPECT_FALSE(path1.Intersects(path2));
    }

    {
        AttributePathParams path1;
        path1.mClusterId = 1;
        AttributePathParams path2;
        path2.mClusterId = 2;
        EXPECT_FALSE(path1.Intersects(path2));
    }

    {
        AttributePathParams path1;
        path1.mAttributeId = 1;
        AttributePathParams path2;
        path2.mAttributeId = 2;
        EXPECT_FALSE(path1.Intersects(path2));
    }
}

TEST(TestAttributePathParams, TestAttributePathIncludedSameFieldId)
{
    AttributePathParams path1;
    AttributePathParams path2;
    AttributePathParams path3;
    path1.mAttributeId = 1;
    path2.mAttributeId = 1;
    path3.mAttributeId = 1;
    EXPECT_TRUE(path1.IsAttributePathSupersetOf(path2));
    path2.mListIndex = 1;
    EXPECT_TRUE(path1.IsAttributePathSupersetOf(path2));
    path1.mListIndex = 0;
    EXPECT_FALSE(path1.IsAttributePathSupersetOf(path3));
    path3.mListIndex = 0;
    EXPECT_TRUE(path1.IsAttributePathSupersetOf(path3));
    path3.mListIndex = 1;
    EXPECT_FALSE(path1.IsAttributePathSupersetOf(path3));
}

TEST(TestAttributePathParams, TestAttributePathIncludedDifferentFieldId)
{
    {
        AttributePathParams path1;
        AttributePathParams path2;
        path1.mAttributeId = 1;
        path2.mAttributeId = 2;
        EXPECT_FALSE(path1.IsAttributePathSupersetOf(path2));
    }
    {
        AttributePathParams path1;
        AttributePathParams path2;
        path2.mAttributeId = 2;
        EXPECT_TRUE(path1.IsAttributePathSupersetOf(path2));
    }
    {
        AttributePathParams path1;
        AttributePathParams path2;
        EXPECT_TRUE(path1.IsAttributePathSupersetOf(path2));
    }
    {
        AttributePathParams path1;
        AttributePathParams path2;

        path1.mAttributeId = 1;
        EXPECT_FALSE(path1.IsAttributePathSupersetOf(path2));
    }
}

TEST(TestAttributePathParams, TestAttributePathIncludedDifferentEndpointId)
{
    AttributePathParams path1;
    AttributePathParams path2;
    path1.mEndpointId = 1;
    path2.mEndpointId = 2;
    EXPECT_FALSE(path1.IsAttributePathSupersetOf(path2));
}

TEST(TestAttributePathParams, TestAttributePathIncludedDifferentClusterId)
{
    AttributePathParams path1;
    AttributePathParams path2;
    path1.mClusterId = 1;
    path2.mClusterId = 2;
    EXPECT_FALSE(path1.IsAttributePathSupersetOf(path2));
}

} // namespace TestPath
} // namespace app
} // namespace chip
