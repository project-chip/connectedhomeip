#pragma once

#include "AttributeTesting.h"
#include "lib/core/CHIPError.h"

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace Testing {

// Compare the attributes of the cluster against the expected set.
// Will use the first path returned by `GetPaths()` on the cluster.
// Dies if `GetPaths()` doesn't return a list with one path.
//
// NOTE: will AUTOMATICALLY add global attributes to the expected list, as those MUST always be available
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

    VerifyOrDie(expectedBuilder.EnsureAppendCapacity(expected.size()) == CHIP_NO_ERROR);
    for (const auto entry : expected)
    {
        VerifyOrDie(expectedBuilder.Append(entry) == CHIP_NO_ERROR);
    }
    VerifyOrDie(expectedBuilder.AppendElements(app::DefaultServerCluster::GlobalAttributes()) == CHIP_NO_ERROR);

    return EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer());
}

// Compare the accepted commands of the cluster against the expected set.
// Will use the first path returned by `GetPaths()` on the cluster.
// Dies if `GetPaths()` doesn't return a list with one path.
bool IsAcceptedCommandsListEqualTo(app::ServerClusterInterface & cluster,
                                   std::initializer_list<const app::DataModel::AcceptedCommandEntry> expected)
{
    VerifyOrDie(cluster.GetPaths().size() == 1);
    auto path = cluster.GetPaths()[0];
    ReadOnlyBufferBuilder<app::DataModel::AcceptedCommandEntry> commandsBuilder;
    if (cluster.AcceptedCommands(path, commandsBuilder) != CHIP_NO_ERROR)
    {
        return false;
    }

    ReadOnlyBufferBuilder<app::DataModel::AcceptedCommandEntry> expectedBuilder;

    VerifyOrDie(expectedBuilder.EnsureAppendCapacity(expected.size()) == CHIP_NO_ERROR);
    for (const auto entry : expected)
    {
        VerifyOrDie(expectedBuilder.Append(entry) == CHIP_NO_ERROR);
    }

    return EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expectedBuilder.TakeBuffer());
}

// Compare the generated commands of the cluster against the expected set.
// Will use the first path returned by `GetPaths()` on the cluster.
// Dies if `GetPaths()` doesn't return a list with one path.
bool IsGeneratedCommandsListEqualTo(app::ServerClusterInterface & cluster, Span<CommandId> expected)
{
    VerifyOrDie(cluster.GetPaths().size() == 1);
    auto path = cluster.GetPaths()[0];
    ReadOnlyBufferBuilder<CommandId> commandsBuilder;
    if (cluster.GeneratedCommands(path, commandsBuilder) != CHIP_NO_ERROR)
    {
        return false;
    }
    return EqualGeneratedCommandSets(commandsBuilder.TakeBuffer(), expected);
}

} // namespace Testing
} // namespace chip
