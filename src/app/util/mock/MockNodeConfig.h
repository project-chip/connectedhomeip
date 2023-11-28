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

#include <app/util/af-types.h>
#include <lib/core/DataModelTypes.h>

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace chip {
namespace Test {

struct MockAttributeConfig
{
    MockAttributeConfig(AttributeId aId) : id(aId) {}
    const AttributeId id;
};

struct MockEventConfig
{
    MockEventConfig(EventId aId) : id(aId) {}
    const EventId id;
};

struct MockClusterConfig
{
    MockClusterConfig(ClusterId aId, std::initializer_list<MockAttributeConfig> aAttributes = {},
                      std::initializer_list<MockEventConfig> aEvents = {});

    const MockAttributeConfig * attributeById(AttributeId attributeId, ptrdiff_t * outIndex = nullptr) const;
    const EmberAfCluster * emberCluster() const { return &mEmberCluster; }

    const ClusterId id;
    const std::vector<MockAttributeConfig> attributes;
    const std::vector<MockEventConfig> events;

private:
    EmberAfCluster mEmberCluster;
    std::vector<EventId> mEmberEventList;
};

struct MockEndpointConfig
{
    MockEndpointConfig(EndpointId aId, std::initializer_list<MockClusterConfig> aClusters = {});

    const MockClusterConfig * clusterById(ClusterId clusterId, ptrdiff_t * outIndex = nullptr) const;
    const EmberAfEndpointType * emberEndpoint() const { return &mEmberEndpoint; }

    const EndpointId id;
    const std::vector<MockClusterConfig> clusters;

private:
    std::vector<EmberAfCluster> mEmberClusters;
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
