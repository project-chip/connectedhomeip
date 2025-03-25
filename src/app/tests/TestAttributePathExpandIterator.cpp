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

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributePathExpandIterator.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <app/util/mock/Constants.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/LinkedList.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;

namespace {

using P = app::ConcreteAttributePath;

struct TestAttributePathExpandIterator : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestAttributePathExpandIterator, TestAllWildcard)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint1, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(2), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(1), MockAttributeId(1) },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(2) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(3) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(4) },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::AttributeList::Id },
    };

    size_t index = 0;

    auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);

    while (true)
    {
        // re-create the iterator
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);

        if (!iter.Next(path))
        {
            break;
        }

        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, MATTER_ARRAY_SIZE(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, MATTER_ARRAY_SIZE(paths));
}

TEST_F(TestAttributePathExpandIterator, TestWildcardEndpoint)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mClusterId   = chip::Test::MockClusterId(3);
    clusInfo.mValue.mAttributeId = chip::Test::MockAttributeId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
    };

    size_t index = 0;

    auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);
    while (true)
    {
        // re-create the iterator
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);

        if (!iter.Next(path))
        {
            break;
        }
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, MATTER_ARRAY_SIZE(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, MATTER_ARRAY_SIZE(paths));
}

TEST_F(TestAttributePathExpandIterator, TestWildcardCluster)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mEndpointId  = chip::Test::kMockEndpoint3;
    clusInfo.mValue.mAttributeId = app::Clusters::Globals::Attributes::ClusterRevision::Id;

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::ClusterRevision::Id },
    };

    size_t index = 0;

    auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);
    while (true)
    {
        // re-create the iterator
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);

        if (!iter.Next(path))
        {
            break;
        }
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, MATTER_ARRAY_SIZE(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, MATTER_ARRAY_SIZE(paths));
}

TEST_F(TestAttributePathExpandIterator, TestWildcardClusterGlobalAttributeNotInMetadata)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mEndpointId  = chip::Test::kMockEndpoint3;
    clusInfo.mValue.mAttributeId = app::Clusters::Globals::Attributes::AttributeList::Id;

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::AttributeList::Id },
    };

    size_t index = 0;

    auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);

    while (true)
    {
        // re-create the iterator
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);

        if (!iter.Next(path))
        {
            break;
        }
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, MATTER_ARRAY_SIZE(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, MATTER_ARRAY_SIZE(paths));
}

TEST_F(TestAttributePathExpandIterator, TestWildcardAttribute)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mEndpointId = chip::Test::kMockEndpoint2;
    clusInfo.mValue.mClusterId  = chip::Test::MockClusterId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
    };

    size_t index = 0;

    auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);

    while (true)
    {
        // re-create the iterator
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);

        if (!iter.Next(path))
        {
            break;
        }
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, MATTER_ARRAY_SIZE(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, MATTER_ARRAY_SIZE(paths));
}

TEST_F(TestAttributePathExpandIterator, TestNoWildcard)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mEndpointId  = chip::Test::kMockEndpoint2;
    clusInfo.mValue.mClusterId   = chip::Test::MockClusterId(3);
    clusInfo.mValue.mAttributeId = chip::Test::MockAttributeId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
    };

    size_t index = 0;

    auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);
    while (true)
    {
        // re-create the iterator
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);

        if (!iter.Next(path))
        {
            break;
        }
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, MATTER_ARRAY_SIZE(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, MATTER_ARRAY_SIZE(paths));
}

TEST_F(TestAttributePathExpandIterator, TestFixedPathExpansion)
{
    // expansion logic requires that:
    //   - paths for wildcard expansion ARE VALIDATED
    //   - path WITHOUT wildcard expansion ARE NOT VALIDATED

    // invalid attribute across all clusters returns empty
    {
        SingleLinkedListNode<app::AttributePathParams> clusInfo;
        clusInfo.mValue.mAttributeId = 122333;

        auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);
        ConcreteAttributePath path;

        EXPECT_FALSE(iter.Next(path));
    }

    // invalid cluster with a valid attribute (featuremap) returns empty
    {
        SingleLinkedListNode<app::AttributePathParams> clusInfo;
        clusInfo.mValue.mClusterId   = 122344;
        clusInfo.mValue.mAttributeId = Clusters::Globals::Attributes::FeatureMap::Id;

        auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);
        ConcreteAttributePath path;

        EXPECT_FALSE(iter.Next(path));
    }

    // invalid cluster with wildcard attribute returns empty
    {
        SingleLinkedListNode<app::AttributePathParams> clusInfo;
        clusInfo.mValue.mClusterId = 122333;

        auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);
        ConcreteAttributePath path;

        EXPECT_FALSE(iter.Next(path));
    }

    // even though all above WERE invalid, if we specify a non-wildcard path it is returned as-is
    {
        SingleLinkedListNode<app::AttributePathParams> clusInfo;
        clusInfo.mValue.mEndpointId  = 1;
        clusInfo.mValue.mClusterId   = 122344;
        clusInfo.mValue.mAttributeId = 122333;

        auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo);
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);
        ConcreteAttributePath path;

        EXPECT_TRUE(iter.Next(path));
        EXPECT_EQ(path.mEndpointId, clusInfo.mValue.mEndpointId);
        EXPECT_EQ(path.mClusterId, clusInfo.mValue.mClusterId);
        EXPECT_EQ(path.mAttributeId, clusInfo.mValue.mAttributeId);

        EXPECT_FALSE(iter.Next(path));
    }
}

TEST_F(TestAttributePathExpandIterator, TestMultipleClusInfo)
{

    SingleLinkedListNode<app::AttributePathParams> clusInfo1;

    SingleLinkedListNode<app::AttributePathParams> clusInfo2;
    clusInfo2.mValue.mClusterId   = chip::Test::MockClusterId(3);
    clusInfo2.mValue.mAttributeId = chip::Test::MockAttributeId(3);

    SingleLinkedListNode<app::AttributePathParams> clusInfo3;
    clusInfo3.mValue.mEndpointId  = chip::Test::kMockEndpoint3;
    clusInfo3.mValue.mAttributeId = app::Clusters::Globals::Attributes::ClusterRevision::Id;

    SingleLinkedListNode<app::AttributePathParams> clusInfo4;
    clusInfo4.mValue.mEndpointId = chip::Test::kMockEndpoint2;
    clusInfo4.mValue.mClusterId  = chip::Test::MockClusterId(3);

    SingleLinkedListNode<app::AttributePathParams> clusInfo5;
    clusInfo5.mValue.mEndpointId  = chip::Test::kMockEndpoint2;
    clusInfo5.mValue.mClusterId   = chip::Test::MockClusterId(3);
    clusInfo5.mValue.mAttributeId = chip::Test::MockAttributeId(3);

    clusInfo1.mpNext = &clusInfo2;
    clusInfo2.mpNext = &clusInfo3;
    clusInfo3.mpNext = &clusInfo4;
    clusInfo4.mpNext = &clusInfo5;

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint1, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(2), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(1), MockAttributeId(1) },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(2) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(3) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(4) },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
    };

    // Test that a one-shot iterate through all works
    {
        size_t index = 0;

        auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo1);
        app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);

        while (iter.Next(path))
        {
            ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                          ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
            EXPECT_LT(index, MATTER_ARRAY_SIZE(paths));
            EXPECT_EQ(paths[index], path);
            index++;
        }
        EXPECT_EQ(index, MATTER_ARRAY_SIZE(paths));
    }

    // identical test, however this checks that position re-use works
    // the same as a one-shot iteration.
    {
        size_t index = 0;

        auto position = AttributePathExpandIterator::Position::StartIterating(&clusInfo1);
        while (true)
        {
            // re-create the iterator
            app::AttributePathExpandIterator iter(CodegenDataModelProviderInstance(nullptr /* delegate */), position);

            if (!iter.Next(path))
            {
                break;
            }
            ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                          ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
            EXPECT_LT(index, MATTER_ARRAY_SIZE(paths));
            EXPECT_EQ(paths[index], path);
            index++;
        }
        EXPECT_EQ(index, MATTER_ARRAY_SIZE(paths));
    }
}

} // namespace
