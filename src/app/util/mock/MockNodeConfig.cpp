/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "app/util/af-types.h"
#include <app/util/mock/MockNodeConfig.h>

#include <app/util/attribute-storage.h>
#include <initializer_list>
#include <lib/support/CodeUtils.h>

#include <utility>

namespace chip {
namespace Test {
namespace internal {
uint16_t SizeForType(EmberAfAttributeType type)
{
    switch (type)
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
    case ZCL_INT8S_ATTRIBUTE_TYPE:
    case ZCL_INT8U_ATTRIBUTE_TYPE:
    case ZCL_BITMAP8_ATTRIBUTE_TYPE:
    case ZCL_ENUM8_ATTRIBUTE_TYPE:
        return 1;
    case ZCL_INT16S_ATTRIBUTE_TYPE:
    case ZCL_INT16U_ATTRIBUTE_TYPE:
    case ZCL_BITMAP16_ATTRIBUTE_TYPE:
    case ZCL_ENUM16_ATTRIBUTE_TYPE:
        return 2;
    case ZCL_INT24S_ATTRIBUTE_TYPE:
    case ZCL_INT24U_ATTRIBUTE_TYPE:
        return 3;
    case ZCL_SINGLE_ATTRIBUTE_TYPE:
    case ZCL_INT32S_ATTRIBUTE_TYPE:
    case ZCL_INT32U_ATTRIBUTE_TYPE:
    case ZCL_BITMAP32_ATTRIBUTE_TYPE:
        return 4;
    case ZCL_INT40S_ATTRIBUTE_TYPE:
    case ZCL_INT40U_ATTRIBUTE_TYPE:
        return 5;
    case ZCL_INT48S_ATTRIBUTE_TYPE:
    case ZCL_INT48U_ATTRIBUTE_TYPE:
        return 6;
    case ZCL_INT56S_ATTRIBUTE_TYPE:
    case ZCL_INT56U_ATTRIBUTE_TYPE:
        return 7;
    case ZCL_DOUBLE_ATTRIBUTE_TYPE:
    case ZCL_INT64S_ATTRIBUTE_TYPE:
    case ZCL_INT64U_ATTRIBUTE_TYPE:
    case ZCL_BITMAP64_ATTRIBUTE_TYPE:
        return 8;
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
        return kDefaultStringSize;
    case ZCL_ARRAY_ATTRIBUTE_TYPE:
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        // These items are using AAI and are acceptable for tests
        return 0;
    default:
        // this type of attribute is not supported for tests, we cannot guess its size
        //
        // see attribute-type.h for a list of attributes
        ChipLogError(Test, "Warning: size for attribute type 0x%x is not set", type);
        return 0;
    }
}

} // namespace internal

namespace {

template <class T>
const T * findById(const std::vector<T> & vector, decltype(std::declval<T>().id) aId, ptrdiff_t * outIndex = nullptr)
{
    for (auto & element : vector)
    {
        if (element.id == aId)
        {
            if (outIndex != nullptr)
            {
                *outIndex = &element - vector.data();
            }
            return &element;
        }
    }

    if (outIndex != nullptr)
    {
        *outIndex = -1;
    }
    return nullptr;
}

} // namespace

MockClusterConfig::MockClusterConfig(ClusterId aId, std::initializer_list<MockAttributeConfig> aAttributes,
                                     std::initializer_list<MockEventConfig> aEvents,
                                     std::initializer_list<CommandId> aAcceptedCommands,
                                     std::initializer_list<CommandId> aGeneratedCommands, BitMask<MockClusterSide> side) :
    id(aId),
    attributes(aAttributes), events(aEvents), mEmberCluster{}, mAcceptedCommands(aAcceptedCommands),
    mGeneratedCommands(aGeneratedCommands)
{
    VerifyOrDie(aAttributes.size() < UINT16_MAX);

    // Note: This only populates the parts of the ember structs that are currently needed for our tests.
    for (const auto & event : events)
    {
        mEmberEventList.push_back(event.id);
    }

    if (side.Has(MockClusterSide::kServer))
    {
        mEmberCluster.mask |= MATTER_CLUSTER_FLAG_SERVER;
    }
    if (side.Has(MockClusterSide::kClient))
    {
        mEmberCluster.mask |= MATTER_CLUSTER_FLAG_CLIENT;
    }
    mEmberCluster.clusterId      = id;
    mEmberCluster.attributeCount = static_cast<uint16_t>(attributes.size());
    mEmberCluster.eventCount     = static_cast<uint16_t>(mEmberEventList.size());
    mEmberCluster.eventList      = mEmberEventList.data();

    if (!mAcceptedCommands.empty())
    {
        mAcceptedCommands.push_back(kInvalidCommandId);
        mEmberCluster.acceptedCommandList = mAcceptedCommands.data();
    }

    if (!mGeneratedCommands.empty())
    {
        mGeneratedCommands.push_back(kInvalidCommandId);
        mEmberCluster.generatedCommandList = mGeneratedCommands.data();
    }

    for (auto & attr : attributes)
    {
        mAttributeMetaData.push_back(attr.attributeMetaData);
    }

    // Make sure ember side has access to attribute metadata
    mEmberCluster.attributes = mAttributeMetaData.data();
}

MockClusterConfig::MockClusterConfig(const MockClusterConfig & other) :
    id(other.id), attributes(other.attributes), events(other.events), mEmberCluster(other.mEmberCluster),
    mEmberEventList(other.mEmberEventList), mAttributeMetaData(other.mAttributeMetaData),
    mAcceptedCommands(other.mAcceptedCommands), mGeneratedCommands(other.mGeneratedCommands)
{
    // Fix self-referencial dependencies after data copy
    mEmberCluster.attributes = mAttributeMetaData.data();
    if (!mAcceptedCommands.empty())
    {
        mEmberCluster.acceptedCommandList = mAcceptedCommands.data();
    }
    if (!mGeneratedCommands.empty())
    {
        mEmberCluster.generatedCommandList = mGeneratedCommands.data();
    }
}

const MockAttributeConfig * MockClusterConfig::attributeById(AttributeId attributeId, ptrdiff_t * outIndex) const
{
    return findById(attributes, attributeId, outIndex);
}

MockEndpointConfig::MockEndpointConfig(EndpointId aId, std::initializer_list<MockClusterConfig> aClusters,
                                       std::initializer_list<EmberAfDeviceType> aDeviceTypes,
                                       std::initializer_list<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> aTags,
                                       app::EndpointComposition aComposition) :
    id(aId),
    composition(aComposition), clusters(aClusters), mDeviceTypes(aDeviceTypes), mSemanticTags(aTags), mEmberEndpoint{}
{
    VerifyOrDie(aClusters.size() < UINT8_MAX);

    // Note: We're copying all the EmberAfClusters because they need to be contiguous in memory
    for (const auto & cluster : clusters)
    {
        mEmberClusters.push_back(*cluster.emberCluster());
    }
    mEmberEndpoint.clusterCount = static_cast<uint8_t>(mEmberClusters.size());
    mEmberEndpoint.cluster      = mEmberClusters.data();
}

MockEndpointConfig::MockEndpointConfig(const MockEndpointConfig & other) :
    id(other.id), composition(other.composition), clusters(other.clusters), mEmberClusters(other.mEmberClusters),
    mDeviceTypes(other.mDeviceTypes), mSemanticTags(other.mSemanticTags), mEmberEndpoint(other.mEmberEndpoint)
{
    // fix self-referencing pointers
    mEmberEndpoint.cluster = mEmberClusters.data();
}

const MockClusterConfig * MockEndpointConfig::clusterById(ClusterId clusterId, ptrdiff_t * outIndex) const
{
    return findById(clusters, clusterId, outIndex);
}

MockNodeConfig::MockNodeConfig(std::initializer_list<MockEndpointConfig> aEndpoints) : endpoints(aEndpoints)
{
    VerifyOrDie(aEndpoints.size() < kEmberInvalidEndpointIndex);
}

const MockEndpointConfig * MockNodeConfig::endpointById(EndpointId endpointId, ptrdiff_t * outIndex) const
{
    return findById(endpoints, endpointId, outIndex);
}

const MockClusterConfig * MockNodeConfig::clusterByIds(EndpointId endpointId, ClusterId clusterId,
                                                       ptrdiff_t * outClusterIndex) const
{
    auto endpoint = endpointById(endpointId);
    if (endpoint == nullptr)
    {
        if (outClusterIndex != nullptr)
        {
            *outClusterIndex = -1;
        }
        return nullptr;
    }
    return endpoint->clusterById(clusterId, outClusterIndex);
}

} // namespace Test
} // namespace chip
