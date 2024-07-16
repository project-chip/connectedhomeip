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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributePathExpandIterator.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <app/util/mock/Constants.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/LinkedList.h>
#include <lib/support/logging/CHIPLogging.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;

namespace {

using P = app::ConcreteAttributePath;

TEST(TestAttributePathExpandIterator, TestAllWildcard)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint1, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(2), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(1), MockAttributeId(1) },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(2) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(3) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(4) },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::AttributeList::Id },
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, ArraySize(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, ArraySize(paths));
}

TEST(TestAttributePathExpandIterator, TestWildcardEndpoint)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mClusterId   = chip::Test::MockClusterId(3);
    clusInfo.mValue.mAttributeId = chip::Test::MockAttributeId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, ArraySize(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, ArraySize(paths));
}

TEST(TestAttributePathExpandIterator, TestWildcardCluster)
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

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, ArraySize(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, ArraySize(paths));
}

TEST(TestAttributePathExpandIterator, TestWildcardClusterGlobalAttributeNotInMetadata)
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

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, ArraySize(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, ArraySize(paths));
}

TEST(TestAttributePathExpandIterator, TestWildcardAttribute)
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
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, ArraySize(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, ArraySize(paths));
}

TEST(TestAttributePathExpandIterator, TestNoWildcard)
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

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, ArraySize(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, ArraySize(paths));
}

TEST(TestAttributePathExpandIterator, TestMultipleClusInfo)
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
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint1, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint1, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint2, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(2), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint2, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(1) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(2) },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(1), MockAttributeId(1) },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint3, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(1) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(2) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(3) },
        { kMockEndpoint3, MockClusterId(2), MockAttributeId(4) },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint3, MockClusterId(2), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint3, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::ClusterRevision::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::FeatureMap::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::GeneratedCommandList::Id },
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::AcceptedCommandList::Id },
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint3, MockClusterId(4), Clusters::Globals::Attributes::EventList::Id },
#endif
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
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::EventList::Id },
#endif
        { kMockEndpoint2, MockClusterId(3), Clusters::Globals::Attributes::AttributeList::Id },
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo1); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        EXPECT_LT(index, ArraySize(paths));
        EXPECT_EQ(paths[index], path);
        index++;
    }
    EXPECT_EQ(index, ArraySize(paths));
}

} // namespace
