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

#include <app/data-model-interface/DataModel.h>

#include <app/util/af-types.h>

namespace chip {
namespace app {

/// An implementation of `InteractionModel::Model` that relies on code-generation
/// via zap/ember.
///
/// The Ember framework uses generated files (like endpoint-config.h and various
/// other generated metadata) to provide a cluster model.
///
/// This class will use global functions generally residing in `app/util`
/// as well as application-specific overrides to provide data model functionality.
///
/// Given that this relies on global data at link time, there generally can be
/// only one CodegenDataModel per application (you can create more instances,
/// however they would share the exact same underlying data and storage).
class CodegenDataModel : public chip::app::InteractionModel::DataModel
{
private:
    /// Ember commands are stored as a `CommandId *` pointer that is either null (i.e. no commands)
    /// or is terminated with 0xFFFF_FFFF aka kInvalidCommandId
    ///
    /// Since iterator implementations in the data model use Next(before_path) calls, iterating
    /// such lists from the beginning would be very inefficient as O(n^2).
    ///
    /// This class maintains a cached position inside such iteration, such that `Next` calls
    /// can be faster.
    class EmberCommandListIterator
    {
    private:
        const CommandId * mCurrentList = nullptr;
        const CommandId * mCurrentHint = nullptr; // Invariant: mCurrentHint is INSIDE mCurrentList
    public:
        EmberCommandListIterator() = default;

        /// Returns the first command in the given list (or nullopt if list is null or starts with 0xFFFFFFF)
        std::optional<CommandId> First(const CommandId * list);

        /// Returns the command after `previousId` in the given list
        std::optional<CommandId> Next(const CommandId * list, CommandId previousId);

        /// Checks if the given command id exists in the given list
        bool Exists(const CommandId * list, CommandId toCheck);
    };

public:
    /// Generic model implementations
    CHIP_ERROR Shutdown() override { return CHIP_NO_ERROR; }

    CHIP_ERROR ReadAttribute(const InteractionModel::ReadAttributeRequest & request, InteractionModel::ReadState & state,
                             AttributeValueEncoder & encoder) override;
    CHIP_ERROR WriteAttribute(const InteractionModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder) override;
    CHIP_ERROR Invoke(const InteractionModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments,
                      InteractionModel::InvokeReply & reply) override;

    /// attribute tree iteration
    EndpointId FirstEndpoint() override;
    EndpointId NextEndpoint(EndpointId before) override;

    InteractionModel::ClusterEntry FirstCluster(EndpointId endpoint) override;
    InteractionModel::ClusterEntry NextCluster(const ConcreteClusterPath & before) override;
    std::optional<InteractionModel::ClusterInfo> GetClusterInfo(const ConcreteClusterPath & path) override;

    InteractionModel::AttributeEntry FirstAttribute(const ConcreteClusterPath & cluster) override;
    InteractionModel::AttributeEntry NextAttribute(const ConcreteAttributePath & before) override;
    std::optional<InteractionModel::AttributeInfo> GetAttributeInfo(const ConcreteAttributePath & path) override;

    InteractionModel::CommandEntry FirstAcceptedCommand(const ConcreteClusterPath & cluster) override;
    InteractionModel::CommandEntry NextAcceptedCommand(const ConcreteCommandPath & before) override;
    std::optional<InteractionModel::CommandInfo> GetAcceptedCommandInfo(const ConcreteCommandPath & path) override;

    ConcreteCommandPath FirstGeneratedCommand(const ConcreteClusterPath & cluster) override;
    ConcreteCommandPath NextGeneratedCommand(const ConcreteCommandPath & before) override;

private:
    // Iteration is often done in a tight loop going through all values.
    // To avoid N^2 iterations, cache a hint of where something is positioned
    uint16_t mEndpointIterationHint  = 0;
    unsigned mClusterIterationHint   = 0;
    unsigned mAttributeIterationHint = 0;
    EmberCommandListIterator mAcceptedCommandsIterator;
    EmberCommandListIterator mGeneratedCommandsIterator;

    // represents a remembered cluster reference that has been found as
    // looking for clusters is very common (for every attribute iteration)
    struct ClusterReference
    {
        ConcreteClusterPath path;
        const EmberAfCluster * cluster;

        ClusterReference(const ConcreteClusterPath p, const EmberAfCluster * c) : path(p), cluster(c) {}
    };
    std::optional<ClusterReference> mPreviouslyFoundCluster;

    /// Finds the specified ember cluster
    ///
    /// Effectively the same as `emberAfFindServerCluster` except with some caching capabilities
    const EmberAfCluster * FindServerCluster(const ConcreteClusterPath & path);

    /// Find the index of the given attribute id
    std::optional<unsigned> TryFindAttributeIndex(const EmberAfCluster * cluster, chip::AttributeId id) const;

    /// Find the index of the given cluster id
    std::optional<unsigned> TryFindServerClusterIndex(const EmberAfEndpointType * endpoint, chip::ClusterId id) const;

    /// Find the index of the given endpoint id
    std::optional<unsigned> TryFindEndpointIndex(chip::EndpointId id) const;
};

} // namespace app
} // namespace chip
