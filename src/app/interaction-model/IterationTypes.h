/*
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
#pragma once

#include <cstdint>
#include <optional>

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {
namespace InteractionModel {

enum class ClusterQualityFlags : uint32_t
{
    kDiagnosticsData = 0x0001, // `K` quality, may be filtered out in subscriptions
};

struct ClusterInfo
{
    DataVersion dataVersion; // current version of this cluster
    BitFlags<ClusterQualityFlags> flags;
};

struct ClusterEntry
{
    ConcreteClusterPath path;
    ClusterInfo info;
};

enum class AttributeQualityFlags : uint32_t
{
    kListAttribute  = 0x0001, // This attribute is a list attribute
    kChangesOmitted = 0x0002, // `C` quality on attributes
};

struct AttributeInfo
{
    BitFlags<AttributeQualityFlags> flags;
};

struct AttributeEntry
{
    ConcreteAttributePath path;
    AttributeInfo info;
};

/// Provides metadata information for a data model
///
/// The data model can be viewed as a tree of endpoint/cluster/attribute
/// where each element can be iterated through independently
///
/// Iteration rules:
///   - kInvalidEndpointId will be returned when iteration ends (or generally kInvalid* for paths)
///   - Any internal iteration errors are just logged (callers do not handle iteration CHIP_ERROR)
///   - Iteration order is NOT guaranteed globally. Only the following is guaranteed:
///     - when iterating over an endpoint, ALL clusters of that endpoint will be iterated first, before
///       switching the endpoint (order of clusters themselves not guaranteed)
///     - when iterating over a cluster, ALL attributes of that cluster will be iterated first, before
///       switching to a new cluster
///     - uniqueness and completeness (iterate over all possible distinct values as long as no
///       internal structural changes occur)
class AttributeTreeIterator
{
public:
    virtual ~AttributeTreeIterator() = default;

    virtual EndpointId FirstEndpoint()                 = 0;
    virtual EndpointId NextEndpoint(EndpointId before) = 0;

    virtual ClusterEntry FirstCluster(EndpointId endpoint)                              = 0;
    virtual ClusterEntry NextCluster(const ConcreteClusterPath & before)                = 0;
    virtual std::optional<ClusterInfo> GetClusterInfo(const ConcreteClusterPath & path) = 0;

    virtual AttributeEntry FirstAttribute(const ConcreteClusterPath & cluster)                = 0;
    virtual AttributeEntry NextAttribute(const ConcreteAttributePath & before)                = 0;
    virtual std::optional<AttributeInfo> GetAttributeInfo(const ConcreteAttributePath & path) = 0;
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
