/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>

#include <cstdint>

namespace chip {
namespace app {

// pre declare the type to not add its header file as a dependency
namespace DataModel { template<class T> class Nullable; }
namespace Clusters {
namespace StaticApplicationConfig {

/// An enumeration acting as a placeholder of "no features defined
/// for a cluster, enforcing that the feature map type is always
/// a form of bitflags"
enum class NoFeatureFlagsDefined : uint32_t
{
};

/// Defines a structure that describes the configuration of a cluster
/// on a specific endpoint
template <typename FeatureEnumType>
struct ClusterConfiguration
{
    EndpointId endpointNumber;
    BitFlags<FeatureEnumType> featureMap;
    Span<const AttributeId> enabledAttributes;
    Span<const CommandId> enabledCommands;
};

// Helper to remove DataModel::Nullable from optional attributes' types, so that they can be stored in a compileTime object
// Because DataModel::Nullable is not a literal type, it cannot be used in a compile-time context.
template<typename AttributeType>
struct RemoveNullable
{
    using Type = AttributeType;
};

template<typename T>
struct RemoveNullable<DataModel::Nullable<T>>
{
    using Type = T;
};

template<typename AttributeType>
using RemoveNullableT = typename RemoveNullable<AttributeType>::Type;

} // namespace StaticApplicationConfig
} // namespace Clusters
} // namespace app
} // namespace chip
