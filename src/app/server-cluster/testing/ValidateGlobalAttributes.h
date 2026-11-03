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
#include <vector>

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
///     expected - initializer_list or any other iterable of expected attribute entries supporting `std::size(container)` syntax.
///     May be empty for global attributes only.
///
/// @note This function will assert (die) if `cluster.GetPaths()` does not return exactly one path.
///
/// Example Usage:
/// ```
/// ClusterImpl cluster(kTestEndpointId, ....);
/// ASSERT_TRUE(IsAttributesListEqualTo(cluster, { Attributes::SomeAttribute::kMetadataEntry }));
/// ```

template <class T>
bool IsAttributesListEqualTo(app::ServerClusterInterface & cluster, const T & expected)
{
    VerifyOrDie(cluster.GetPaths().size() == 1);
    auto path = cluster.GetPaths()[0];
    ReadOnlyBufferBuilder<app::DataModel::AttributeEntry> attributesBuilder;
    if (CHIP_ERROR err = cluster.Attributes(path, attributesBuilder); err != CHIP_NO_ERROR)
    {
        ChipLogError(Test, "Failed to get attributes list from cluster. Error: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    ReadOnlyBufferBuilder<app::DataModel::AttributeEntry> expectedBuilder;

    SuccessOrDie(expectedBuilder.EnsureAppendCapacity(std::size(expected)));
    for (const auto & entry : expected)
    {
        SuccessOrDie(expectedBuilder.Append(entry));
    }

    SuccessOrDie(expectedBuilder.AppendElements(app::DefaultServerCluster::GlobalAttributes()));

    return EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer());
}

// Overload for std::initializer_list to not get "template argument deduction failed" when calling `IsAttributesListEqualTo(cluster,
// {...})`
template <typename T = const app::DataModel::AttributeEntry>
bool IsAttributesListEqualTo(app::ServerClusterInterface & cluster, std::initializer_list<T> expected)
{
    return IsAttributesListEqualTo<std::initializer_list<T>>(cluster, expected);
}

/// Compares the accepted commands of a cluster against an expected set.
///
/// This function retrieves the accepted commands for the first path returned by `cluster.GetPaths()`
/// and compares them against the `expected` list of items.
///
/// Parameters:
///     cluster - The cluster interface to test.
///     expected - initializer_list or any other iterable of expected accepted command entries supporting `std::size(container)`
///     syntax. May be empty.
///
/// @note This function will assert (die) if `cluster.GetPaths()` does not return exactly one path.
///
/// Example Usage:
///
/// ```
/// ClusterImpl cluster(kTestEndpointId, ...);
/// ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, { Commands::SomeCommand::kMetadataEntry }));
/// ```
template <class T>
bool IsAcceptedCommandsListEqualTo(app::ServerClusterInterface & cluster, const T & expected)
{
    VerifyOrDie(cluster.GetPaths().size() == 1);
    auto path = cluster.GetPaths()[0];
    ReadOnlyBufferBuilder<app::DataModel::AcceptedCommandEntry> commandsBuilder;
    if (CHIP_ERROR err = cluster.AcceptedCommands(path, commandsBuilder); err != CHIP_NO_ERROR)
    {
        ChipLogError(Test, "Failed to get accepted commands list from cluster. Error: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    ReadOnlyBufferBuilder<app::DataModel::AcceptedCommandEntry> expectedBuilder;

    SuccessOrDie(expectedBuilder.EnsureAppendCapacity(std::size(expected)));
    for (const auto & entry : expected)
    {
        SuccessOrDie(expectedBuilder.Append(entry));
    }

    return EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expectedBuilder.TakeBuffer());
}

// Overload for std::initializer_list to not get "template argument deduction failed" when calling
// `IsAcceptedCommandsListEqualTo(cluster, {...})`
template <typename T = const app::DataModel::AcceptedCommandEntry>
bool IsAcceptedCommandsListEqualTo(app::ServerClusterInterface & cluster, std::initializer_list<T> expected)
{
    return IsAcceptedCommandsListEqualTo<std::initializer_list<T>>(cluster, expected);
}

/// Compares the generated commands of a cluster against an expected set.
///
/// This function retrieves the generated commands for the first path returned by `cluster.GetPaths()`
/// and compares them against the `expected` list of items.
///
/// Parameters:
///     cluster - The cluster interface to test.
///     expected - initializer_list or any other iterable of expected generated command entries supporting `std::size(container)`
///     syntax. May be empty.
///
/// @note This function will assert (die) if `cluster.GetPaths()` does not return exactly one path.
///
/// Example Usage:
/// ```
/// ClusterImpl cluster(kTestEndpointId, ...);
/// ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, { Commands::SomeCommandResponse::kMetadataEntry }));
/// ```
template <class T>
bool IsGeneratedCommandsListEqualTo(app::ServerClusterInterface & cluster, const T & expected)
{
    VerifyOrDie(cluster.GetPaths().size() == 1);
    auto path = cluster.GetPaths()[0];
    ReadOnlyBufferBuilder<CommandId> commandsBuilder;
    if (CHIP_ERROR err = cluster.GeneratedCommands(path, commandsBuilder); err != CHIP_NO_ERROR)
    {
        ChipLogError(Test, "Failed to get generated commands list from cluster. Error: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    ReadOnlyBufferBuilder<CommandId> expectedBuilder;

    SuccessOrDie(expectedBuilder.EnsureAppendCapacity(std::size(expected)));
    for (const auto & entry : expected)
    {
        SuccessOrDie(expectedBuilder.Append(entry));
    }

    return EqualGeneratedCommandSets(commandsBuilder.TakeBuffer(), expectedBuilder.TakeBuffer());
}

// Overload for std::initializer_list to not get "template argument deduction failed" when calling
// `IsGeneratedCommandsListEqualTo(cluster, {...})`
template <typename T = const CommandId>
bool IsGeneratedCommandsListEqualTo(app::ServerClusterInterface & cluster, std::initializer_list<T> expected)
{
    return IsGeneratedCommandsListEqualTo<std::initializer_list<T>>(cluster, expected);
}

} // namespace Testing
} // namespace chip
