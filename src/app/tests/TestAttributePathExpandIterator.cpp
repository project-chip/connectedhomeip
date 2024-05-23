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
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;

namespace {

using P = app::ConcreteAttributePath;

void TestAllWildcard(nlTestSuite * apSuite, void * apContext)
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
        NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestWildcardEndpoint(nlTestSuite * apSuite, void * apContext)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mClusterId   = Test::MockClusterId(3);
    clusInfo.mValue.mAttributeId = Test::MockAttributeId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestWildcardCluster(nlTestSuite * apSuite, void * apContext)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mEndpointId  = Test::kMockEndpoint3;
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
        NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestWildcardClusterGlobalAttributeNotInMetadata(nlTestSuite * apSuite, void * apContext)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mEndpointId  = Test::kMockEndpoint3;
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
        NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestWildcardAttribute(nlTestSuite * apSuite, void * apContext)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mEndpointId = Test::kMockEndpoint2;
    clusInfo.mValue.mClusterId  = Test::MockClusterId(3);

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
        NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestNoWildcard(nlTestSuite * apSuite, void * apContext)
{
    SingleLinkedListNode<app::AttributePathParams> clusInfo;
    clusInfo.mValue.mEndpointId  = Test::kMockEndpoint2;
    clusInfo.mValue.mClusterId   = Test::MockClusterId(3);
    clusInfo.mValue.mAttributeId = Test::MockAttributeId(3);

    app::ConcreteAttributePath path;
    P paths[] = {
        { kMockEndpoint2, MockClusterId(3), MockAttributeId(3) },
    };

    size_t index = 0;

    for (app::AttributePathExpandIterator iter(&clusInfo); iter.Get(path); iter.Next())
    {
        ChipLogDetail(AppServer, "Visited Attribute: 0x%04X / " ChipLogFormatMEI " / " ChipLogFormatMEI, path.mEndpointId,
                      ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

void TestMultipleClusInfo(nlTestSuite * apSuite, void * apContext)
{

    SingleLinkedListNode<app::AttributePathParams> clusInfo1;

    SingleLinkedListNode<app::AttributePathParams> clusInfo2;
    clusInfo2.mValue.mClusterId   = Test::MockClusterId(3);
    clusInfo2.mValue.mAttributeId = Test::MockAttributeId(3);

    SingleLinkedListNode<app::AttributePathParams> clusInfo3;
    clusInfo3.mValue.mEndpointId  = Test::kMockEndpoint3;
    clusInfo3.mValue.mAttributeId = app::Clusters::Globals::Attributes::ClusterRevision::Id;

    SingleLinkedListNode<app::AttributePathParams> clusInfo4;
    clusInfo4.mValue.mEndpointId = Test::kMockEndpoint2;
    clusInfo4.mValue.mClusterId  = Test::MockClusterId(3);

    SingleLinkedListNode<app::AttributePathParams> clusInfo5;
    clusInfo5.mValue.mEndpointId  = Test::kMockEndpoint2;
    clusInfo5.mValue.mClusterId   = Test::MockClusterId(3);
    clusInfo5.mValue.mAttributeId = Test::MockAttributeId(3);

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
        NL_TEST_ASSERT(apSuite, index < ArraySize(paths) && paths[index] == path);
        index++;
    }
    NL_TEST_ASSERT(apSuite, index == ArraySize(paths));
}

static int TestSetup(void * inContext)
{
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    return SUCCESS;
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
        NL_TEST_DEF("TestAllWildcard", TestAllWildcard),
        NL_TEST_DEF("TestWildcardEndpoint", TestWildcardEndpoint),
        NL_TEST_DEF("TestWildcardCluster", TestWildcardCluster),
        NL_TEST_DEF("TestWildcardClusterGlobalAttributeNotInMetadata",
                    TestWildcardClusterGlobalAttributeNotInMetadata),
        NL_TEST_DEF("TestWildcardAttribute", TestWildcardAttribute),
        NL_TEST_DEF("TestNoWildcard", TestNoWildcard),
        NL_TEST_DEF("TestMultipleClusInfo", TestMultipleClusInfo),
        NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestAttributePathExpandIterator",
    &sTests[0],
    TestSetup,
    TestTeardown,
};
// clang-format on

} // namespace

int TestAttributePathExpandIterator()
{
    nlTestRunner(&sSuite, nullptr);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestAttributePathExpandIterator)
