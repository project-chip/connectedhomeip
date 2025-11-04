#include "AttributeTesting.h"
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {

namespace Testing {

// Compare the attributes of the cluster against the expected set.
// Will use the first path returned by `GetPaths()` on the cluster.
// Dies if `GetPaths()` doesn't return a list with one path.
bool IsAttributesListEqualTo(app::ServerClusterInterface & cluster, Span<app::DataModel::AttributeEntry> expected)
{
    VerifyOrDie(cluster.GetPaths().size() == 1);
    auto path = cluster.GetPaths()[0];
    ReadOnlyBufferBuilder<app::DataModel::AttributeEntry> attributesBuilder;
    if (cluster.Attributes(path, attributesBuilder) != CHIP_NO_ERROR)
    {
        return false;
    }
    return EqualAttributeSets(attributesBuilder.TakeBuffer(), expected);
}

// Compare the accepted commands of the cluster against the expected set.
// Will use the first path returned by `GetPaths()` on the cluster.
// Dies if `GetPaths()` doesn't return a list with one path.
bool IsAcceptedCommandsListEqualTo(app::ServerClusterInterface & cluster, Span<app::DataModel::AcceptedCommandEntry> expected)
{
    VerifyOrDie(cluster.GetPaths().size() == 1);
    auto path = cluster.GetPaths()[0];
    ReadOnlyBufferBuilder<app::DataModel::AcceptedCommandEntry> commandsBuilder;
    if (cluster.AcceptedCommands(path, commandsBuilder) != CHIP_NO_ERROR)
    {
        return false;
    }
    return EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expected);
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
