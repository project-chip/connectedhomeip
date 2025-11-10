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
                             std::initializer_list<const app::DataModel::AttributeEntry> expected)
{
    VerifyOrDie(cluster.GetPaths().size() == 1);
    auto path = cluster.GetPaths()[0];
    ReadOnlyBufferBuilder<app::DataModel::AttributeEntry> attributesBuilder;
    if (cluster.Attributes(path, attributesBuilder) != CHIP_NO_ERROR)
    {
        return false;
    }

    // build expectation with global attributes
    ReadOnlyBufferBuilder<app::DataModel::AttributeEntry> expectedBuilder;

    SuccessOrDie(expectedBuilder.EnsureAppendCapacity(expected.size()));
    for (const auto entry : expected)
    {
        SuccessOrDie(expectedBuilder.Append(entry));
    }
    SuccessOrDie(expectedBuilder.AppendElements(app::DefaultServerCluster::GlobalAttributes()));

    return EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer());
}

/// Compares the accepted commands of a cluster against an expected set.
///
/// This function retrieves the accepted commands for the first path returned by `cluster.GetPaths()`
/// and compares them against the `expected` list of items.
///
/// Parameter:
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
/// Parameter:
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
bool IsGeneratedCommandsListEqualTo(app::ServerClusterInterface & cluster, Span<CommandId> expected);

} // namespace Testing
} // namespace chip
