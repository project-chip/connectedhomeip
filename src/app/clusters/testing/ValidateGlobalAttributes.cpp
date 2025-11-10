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
#include <app/clusters/testing/ValidateGlobalAttributes.h>

namespace chip::Testing {

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

    SuccessOrDie(expectedBuilder.EnsureAppendCapacity(expected.size()));
    for (const auto & entry : expected)
    {
        SuccessOrDie(expectedBuilder.Append(entry));
    }

    return EqualAcceptedCommandSets(commandsBuilder.TakeBuffer(), expectedBuilder.TakeBuffer());
}

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

} // namespace chip::Testing
