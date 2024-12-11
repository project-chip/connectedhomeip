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

#include <app/data-model-provider/Provider.h>

#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>

namespace chip {
namespace app {

namespace detail {

/// Handles going through callback-based enumeration of generated/accepted commands
/// for CommandHandlerInterface based items.
///
/// Offers the ability to focus on some operation for finding a given
/// command id:
///   - FindFirst will return the first found element
///   - FindExact finds the element with the given id
///   - FindNext finds the element following the given id
class EnumeratorCommandFinder
{
public:
    using HandlerCallbackFunction = CHIP_ERROR (CommandHandlerInterface::*)(const ConcreteClusterPath &,
                                                                            CommandHandlerInterface::CommandIdCallback, void *);

    enum class Operation
    {
        kFindFirst, // Find the first value in the list
        kFindExact, // Find the given value
        kFindNext   // Find the value AFTER this value
    };

    EnumeratorCommandFinder(HandlerCallbackFunction callback) :
        mCallback(callback), mOperation(Operation::kFindFirst), mTarget(kInvalidCommandId)
    {}

    /// Find the given command ID that matches the given operation/path.
    ///
    /// If operation is kFindFirst, then path commandID is ignored. Otherwise it is used as a key to
    /// kFindExact or kFindNext.
    ///
    /// Returns:
    ///    - std::nullopt if no command found using the command handler interface
    ///    - kInvalidCommandId if the find failed (but command handler interface does provide a list)
    ///    - valid id if command handler interface usage succeeds
    std::optional<CommandId> FindCommandId(Operation operation, const ConcreteCommandPath & path);

private:
    HandlerCallbackFunction mCallback;
    Operation mOperation;
    CommandId mTarget;
    std::optional<CommandId> mFound = std::nullopt;

    Loop HandlerCallback(CommandId id);

    static Loop HandlerCallbackFn(CommandId id, void * context)
    {
        auto self = static_cast<EnumeratorCommandFinder *>(context);
        return self->HandlerCallback(id);
    }
};

} // namespace detail

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
/// only one CodegenDataModelProvider per application (you can create more instances,
/// however they would share the exact same underlying data and storage).
class CodegenDataModelProvider : public DataModel::Provider
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

        void Reset() { mCurrentList = mCurrentHint = nullptr; }
    };

public:
    /// clears out internal caching. Especially useful in unit tests,
    /// where path caching does not really apply (the same path may result in different outcomes)
    void Reset()
    {
        mAcceptedCommandsIterator.Reset();
        mGeneratedCommandsIterator.Reset();
        mPreviouslyFoundCluster = std::nullopt;
    }

    void SetPersistentStorageDelegate(PersistentStorageDelegate * delegate) { mPersistentStorageDelegate = delegate; }
    PersistentStorageDelegate * GetPersistentStorageDelegate() { return mPersistentStorageDelegate; }

    /// Generic model implementations
    CHIP_ERROR Shutdown() override
    {
        Reset();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Startup(DataModel::InteractionModelContext context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> Invoke(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                        CommandHandler * handler) override;

    /// attribute tree iteration
    DataModel::EndpointEntry FirstEndpoint() override;
    DataModel::EndpointEntry NextEndpoint(EndpointId before) override;
    std::optional<DataModel::EndpointInfo> GetEndpointInfo(EndpointId endpoint) override;
    bool EndpointExists(EndpointId endpoint) override;

    std::optional<DataModel::DeviceTypeEntry> FirstDeviceType(EndpointId endpoint) override;
    std::optional<DataModel::DeviceTypeEntry> NextDeviceType(EndpointId endpoint,
                                                             const DataModel::DeviceTypeEntry & previous) override;

    std::optional<SemanticTag> GetFirstSemanticTag(EndpointId endpoint) override;
    std::optional<SemanticTag> GetNextSemanticTag(EndpointId endpoint, const SemanticTag & previous) override;

    DataModel::ClusterEntry FirstServerCluster(EndpointId endpoint) override;
    DataModel::ClusterEntry NextServerCluster(const ConcreteClusterPath & before) override;
    std::optional<DataModel::ClusterInfo> GetServerClusterInfo(const ConcreteClusterPath & path) override;

    ConcreteClusterPath FirstClientCluster(EndpointId endpoint) override;
    ConcreteClusterPath NextClientCluster(const ConcreteClusterPath & before) override;

    DataModel::AttributeEntry FirstAttribute(const ConcreteClusterPath & cluster) override;
    DataModel::AttributeEntry NextAttribute(const ConcreteAttributePath & before) override;
    std::optional<DataModel::AttributeInfo> GetAttributeInfo(const ConcreteAttributePath & path) override;

    DataModel::CommandEntry FirstAcceptedCommand(const ConcreteClusterPath & cluster) override;
    DataModel::CommandEntry NextAcceptedCommand(const ConcreteCommandPath & before) override;
    std::optional<DataModel::CommandInfo> GetAcceptedCommandInfo(const ConcreteCommandPath & path) override;

    ConcreteCommandPath FirstGeneratedCommand(const ConcreteClusterPath & cluster) override;
    ConcreteCommandPath NextGeneratedCommand(const ConcreteCommandPath & before) override;

    void Temporary_ReportAttributeChanged(const AttributePathParams & path) override;

protected:
    virtual void InitDataModel();

private:
    // Iteration is often done in a tight loop going through all values.
    // To avoid N^2 iterations, cache a hint of where something is positioned
    uint16_t mEndpointIterationHint      = 0;
    unsigned mServerClusterIterationHint = 0;
    unsigned mClientClusterIterationHint = 0;
    unsigned mAttributeIterationHint     = 0;
    unsigned mDeviceTypeIterationHint    = 0;
    unsigned mSemanticTagIterationHint   = 0;
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

    enum class ClusterSide : uint8_t
    {
        kServer,
        kClient,
    };

    std::optional<ClusterReference> mPreviouslyFoundCluster;
    unsigned mEmberMetadataStructureGeneration = 0;

    // Ember requires a persistence provider, so we make sure we can always have something
    PersistentStorageDelegate * mPersistentStorageDelegate = nullptr;

    /// Finds the specified ember cluster
    ///
    /// Effectively the same as `emberAfFindServerCluster` except with some caching capabilities
    const EmberAfCluster * FindServerCluster(const ConcreteClusterPath & path);

    /// Find the index of the given attribute id
    std::optional<unsigned> TryFindAttributeIndex(const EmberAfCluster * cluster, AttributeId id) const;

    /// Find the index of the given cluster id
    std::optional<unsigned> TryFindClusterIndex(const EmberAfEndpointType * endpoint, ClusterId id, ClusterSide clusterSide) const;

    /// Find the index of the given endpoint id
    std::optional<unsigned> TryFindEndpointIndex(EndpointId id) const;

    using CommandListGetter = const CommandId *(const EmberAfCluster &);

    CommandId FindCommand(const ConcreteCommandPath & path, detail::EnumeratorCommandFinder & handlerFinder,
                          detail::EnumeratorCommandFinder::Operation operation,
                          CodegenDataModelProvider::EmberCommandListIterator & emberIterator, CommandListGetter commandListGetter);
};

} // namespace app
} // namespace chip
