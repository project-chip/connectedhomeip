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
 *     This file contains the mock implementation for the generated attribute-storage.cpp
 *     - It contains three endpoints, 0xFFFE, 0xFFFD, 0xFFFC
 *     - It contains four clusters: 0xFFF1'0001 to 0xFFF1'0004
 *     - All cluster has two global attribute (0x0000'FFFC, 0x0000'FFFD)
 *     - Some clusters has some cluster-specific attributes, with 0xFFF1 prefix.
 *
 *    Note: The ember's attribute-storage.cpp will include some app specific generated files. So we cannot use it directly. This
 *    might be fixed with a mock endpoint-config.h
 */

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/util/mock/Constants.h>

#include <app/ClusterInfo.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <app/InteractionModelDelegate.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>

typedef uint8_t EmberAfClusterMask;

using namespace chip;
using namespace chip::Test;
using namespace chip::app;

namespace {

EndpointId endpoints[]    = { kMockEndpoint1, kMockEndpoint2, kMockEndpoint3 };
uint16_t clusterIndex[]   = { 0, 2, 5 };
uint8_t clusterCount[]    = { 2, 3, 4 };
ClusterId clusters[]      = { MockClusterId(1), MockClusterId(2), MockClusterId(1), MockClusterId(2), MockClusterId(3),
                         MockClusterId(1), MockClusterId(2), MockClusterId(3), MockClusterId(4) };
uint16_t attributeIndex[] = { 0, 2, 5, 7, 11, 16, 19, 25, 27 };
uint16_t attributeCount[] = { 2, 3, 2, 4, 5, 3, 6, 2, 2 };
AttributeId attributes[]  = {
    // clang-format off
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id,
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id,
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1), MockAttributeId(2),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3), MockAttributeId(4),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id,
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id
    // clang-format on
};

} // namespace

uint16_t emberAfEndpointCount(void)
{
    return ArraySize(endpoints);
}

uint16_t emberAfIndexFromEndpoint(chip::EndpointId endpoint)
{
    for (uint16_t i = 0; i < ArraySize(endpoints); i++)
    {
        if (endpoints[i] == endpoint)
        {
            return i;
        }
    }
    return UINT16_MAX;
}

uint8_t emberAfClusterCount(chip::EndpointId endpoint, bool server)
{
    for (uint16_t i = 0; i < ArraySize(endpoints); i++)
    {
        if (endpoints[i] == endpoint)
        {
            return clusterCount[i];
        }
    }
    return 0;
}

uint16_t emberAfGetServerAttributeCount(chip::EndpointId endpoint, chip::ClusterId cluster)
{
    uint16_t endpointIndex          = emberAfIndexFromEndpoint(endpoint);
    uint16_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint16_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            return attributeCount[i + clusterIndex[endpointIndex]];
        }
    }
    return 0;
}

uint16_t emberAfGetServerAttributeIndexByAttributeId(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                     chip::AttributeId attributeId)
{
    uint16_t endpointIndex          = emberAfIndexFromEndpoint(endpoint);
    uint16_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint16_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            uint16_t clusterAttributeOffset = attributeIndex[i + clusterIndex[endpointIndex]];
            for (uint16_t j = 0; j < emberAfGetServerAttributeCount(endpoint, cluster); j++)
            {
                if (attributes[clusterAttributeOffset + j] == attributeId)
                {
                    return j;
                }
            }
            break;
        }
    }
    return UINT16_MAX;
}

chip::EndpointId emberAfEndpointFromIndex(uint16_t index)
{
    VerifyOrDie(index < ArraySize(endpoints));
    return endpoints[index];
}

chip::Optional<chip::ClusterId> emberAfGetNthClusterId(chip::EndpointId endpoint, uint8_t n, bool server)
{
    if (n >= emberAfClusterCount(endpoint, server))
    {
        return chip::Optional<chip::ClusterId>::Missing();
    }
    return chip::Optional<chip::ClusterId>(clusters[clusterIndex[emberAfIndexFromEndpoint(endpoint)] + n]);
}

chip::Optional<chip::AttributeId> emberAfGetServerAttributeIdByIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                                     uint16_t index)
{
    uint16_t endpointIndex          = emberAfIndexFromEndpoint(endpoint);
    uint16_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint16_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            uint16_t clusterAttributeOffset = attributeIndex[i + clusterIndex[endpointIndex]];
            if (index < emberAfGetServerAttributeCount(endpoint, cluster))
            {
                return Optional<AttributeId>(attributes[clusterAttributeOffset + index]);
            }
            break;
        }
    }
    return Optional<AttributeId>::Missing();
}

uint8_t emberAfClusterIndex(chip::EndpointId endpoint, chip::ClusterId cluster, EmberAfClusterMask mask)
{
    uint16_t endpointIndex          = emberAfIndexFromEndpoint(endpoint);
    uint16_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint8_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            return i;
        }
    }
    return UINT8_MAX;
}
