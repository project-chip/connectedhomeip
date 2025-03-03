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

#pragma once

#include <app-common/zap-generated/attribute-type.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <lib/core/DataModelTypes.h>

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace chip {
namespace Test {

namespace internal {

constexpr uint16_t kDefaultStringSize = 16; // note: this is INCLUDING the length byte(s)

// Determine an appropriate size for the given type.
// NOTE: this is for test only, not all types are included
uint16_t SizeForType(EmberAfAttributeType type);

constexpr EmberAfAttributeMetadata DefaultAttributeMetadata(chip::AttributeId id)
{
    return EmberAfAttributeMetadata{
        .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(0)),
        .attributeId   = id,
        .size          = 4,
        .attributeType = ZCL_INT32U_ATTRIBUTE_TYPE,
        .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NULLABLE,
    };
}

} // namespace internal

struct MockAttributeConfig
{
    MockAttributeConfig(AttributeId aId) : id(aId), attributeMetaData(internal::DefaultAttributeMetadata(aId)) {}
    MockAttributeConfig(AttributeId aId, EmberAfAttributeMetadata metadata) : id(aId), attributeMetaData(metadata) {}
    MockAttributeConfig(AttributeId aId, EmberAfAttributeType type,
                        EmberAfAttributeMask mask = MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NULLABLE) :
        id(aId),
        attributeMetaData(internal::DefaultAttributeMetadata(aId))
    {
        attributeMetaData.attributeType = type;
        attributeMetaData.mask          = mask;
        attributeMetaData.size          = internal::SizeForType(type);
    }

    const AttributeId id;
    EmberAfAttributeMetadata attributeMetaData;
};

struct MockEventConfig
{
    MockEventConfig(EventId aId) : id(aId) {}
    const EventId id;
};

enum class MockClusterSide : uint8_t
{
    kServer = 0x1,
    kClient = 0x2,
};

struct MockClusterConfig
{
    MockClusterConfig(ClusterId aId, std::initializer_list<MockAttributeConfig> aAttributes = {},
                      std::initializer_list<MockEventConfig> aEvents = {}, std::initializer_list<CommandId> aAcceptedCommands = {},
                      std::initializer_list<CommandId> aGeneratedCommands = {},
                      BitMask<MockClusterSide> side = BitMask<MockClusterSide>().Set(MockClusterSide::kServer));

    // Cluster-config is self-referential: mEmberCluster.attributes references  mAttributeMetaData.data()
    MockClusterConfig(const MockClusterConfig & other);
    MockClusterConfig & operator=(const MockClusterConfig &) = delete;

    const MockAttributeConfig * attributeById(AttributeId attributeId, ptrdiff_t * outIndex = nullptr) const;
    const EmberAfCluster * emberCluster() const { return &mEmberCluster; }

    const ClusterId id;
    const std::vector<MockAttributeConfig> attributes;
    const std::vector<MockEventConfig> events;

private:
    EmberAfCluster mEmberCluster;
    std::vector<EventId> mEmberEventList;
    std::vector<EmberAfAttributeMetadata> mAttributeMetaData;
    std::vector<CommandId> mAcceptedCommands;
    std::vector<CommandId> mGeneratedCommands;
};

struct MockEndpointConfig
{
    MockEndpointConfig(EndpointId aId, std::initializer_list<MockClusterConfig> aClusters = {},
                       std::initializer_list<EmberAfDeviceType> aDeviceTypes                                    = {},
                       std::initializer_list<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> aTags = {},
                       app::EndpointComposition composition = app::EndpointComposition::kFullFamily);

    // Endpoint-config is self-referential: mEmberEndpoint.clusters references  mEmberClusters.data()
    MockEndpointConfig(const MockEndpointConfig & other);
    MockEndpointConfig & operator=(const MockEndpointConfig &) = delete;

    const MockClusterConfig * clusterById(ClusterId clusterId, ptrdiff_t * outIndex = nullptr) const;
    const EmberAfEndpointType * emberEndpoint() const { return &mEmberEndpoint; }
    Span<const EmberAfDeviceType> deviceTypes() const
    {
        return Span<const EmberAfDeviceType>(mDeviceTypes.data(), mDeviceTypes.size());
    }

    Span<const app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> semanticTags() const
    {
        return Span<const app::Clusters::Descriptor::Structs::SemanticTagStruct::Type>(mSemanticTags.data(), mSemanticTags.size());
    }

    const EndpointId id;
    const app::EndpointComposition composition;
    const std::vector<MockClusterConfig> clusters;

private:
    std::vector<EmberAfCluster> mEmberClusters;
    std::vector<EmberAfDeviceType> mDeviceTypes;
    std::vector<app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> mSemanticTags;
    EmberAfEndpointType mEmberEndpoint;
};

struct MockNodeConfig
{
    MockNodeConfig(std::initializer_list<MockEndpointConfig> aEndpoints);

    const MockEndpointConfig * endpointById(EndpointId endpointId, ptrdiff_t * outIndex = nullptr) const;
    const MockClusterConfig * clusterByIds(EndpointId endpointId, ClusterId clusterId, ptrdiff_t * outClusterIndex = nullptr) const;

    const std::vector<MockEndpointConfig> endpoints;
};

} // namespace Test
} // namespace chip
