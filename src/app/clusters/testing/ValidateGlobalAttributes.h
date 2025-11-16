/*
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
#include "AttributeTesting.h"

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace Testing {

/// Compares the attributes of a cluster against an expected set.
///
/// This function retrieves the attributes for the first path returned by `cluster.GetPaths()`
/// and compares them against the `expected` list. Global attributes are automatically
/// added to the `expected` list, as they must always be present.
///
/// Parameters:
///     cluster - The cluster interface to test.
///     expected - An initializer list of expected attribute entries (may be empty for only globals)
///
/// @note This function will assert (die) if `cluster.GetPaths()` does not return exactly one path.
///
/// Example Usage:
/// ```
/// ClusterImpl cluster(kTestEndpointId, ....);
/// ASSERT_TRUE(IsAttributesListEqualTo(cluster, { Attributes::SomeAttribute::kMetadataEntry }));
/// ```
bool IsAttributesListEqualTo(app::ServerClusterInterface & cluster,
                             std::initializer_list<const app::DataModel::AttributeEntry> expected);

/// Overload of IsAttributesListEqualTo that accepts a pointer and count for cleaner tests
///
/// Parameters:
///     cluster - The cluster interface to test.
///     entries - Pointer to the first attribute entry
///     count - Number of attribute entries
bool IsAttributesListEqualTo(app::ServerClusterInterface & cluster, const app::DataModel::AttributeEntry * entries, size_t count);

/// Compares the accepted commands of a cluster against an expected set.
///
/// This function retrieves the accepted commands for the first path returned by `cluster.GetPaths()`
/// and compares them against the `expected` list of items.
///
/// Parameters:
///     cluster - The cluster interface to test.
///     expected - An initializer list of expected accepted command entries. May be empty.
///
/// @note This function will assert (die) if `cluster.GetPaths()` does not return exactly one path.
///
/// Example Usage:
///
/// ```
/// ClusterImpl cluster(kTestEndpointId, ...);
/// ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, { Commands::SomeCommand::kMetadataEntry }));
/// ```
bool IsAcceptedCommandsListEqualTo(app::ServerClusterInterface & cluster,
                                   std::initializer_list<const app::DataModel::AcceptedCommandEntry> expected);

/// Compares the generated commands of a cluster against an expected set.
///
/// This function retrieves the generated commands for the first path returned by `cluster.GetPaths()`
/// and compares them against the `expected` list of items.
///
/// Parameters:
///     cluster - The cluster interface to test.
///     expected - An initializer list of expected generated command entries. May be empty.
///
/// @note This function will assert (die) if `cluster.GetPaths()` does not return exactly one path.
///
/// Example Usage:
/// ```
/// ClusterImpl cluster(kTestEndpointId, ...);
/// ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, { Commands::SomeCommandResponse::kMetadataEntry }));
/// ```
bool IsGeneratedCommandsListEqualTo(app::ServerClusterInterface & cluster, std::initializer_list<const CommandId> expected);

} // namespace Testing
} // namespace chip
