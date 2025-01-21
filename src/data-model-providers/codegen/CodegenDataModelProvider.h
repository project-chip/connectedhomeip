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
public:
    /// clears out internal caching. Especially useful in unit tests,
    /// where path caching does not really apply (the same path may result in different outcomes)
    void Reset() { mPreviouslyFoundCluster = std::nullopt; }

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
    DataModel::MetadataList<CommandId> GeneratedCommands(const ConcreteClusterPath & path) override;
    DataModel::MetadataList<DataModel::AcceptedCommandEntry> AcceptedCommands(const ConcreteClusterPath & path) override;
    DataModel::MetadataList<SemanticTag> SemanticTags(EndpointId endpointId) override;
    DataModel::MetadataList<DataModel::DeviceTypeEntry> DeviceTypes(EndpointId endpointId) override;
    DataModel::MetadataList<DataModel::EndpointEntry> Endpoints() override;
    DataModel::MetadataList<ClusterId> ClientClusters(EndpointId endpointId) override;
    DataModel::MetadataList<DataModel::ServerClusterEntry> ServerClusters(EndpointId endpointId) override;
    DataModel::MetadataList<DataModel::AttributeEntry> Attributes(const ConcreteClusterPath & path) override;

    void Temporary_ReportAttributeChanged(const AttributePathParams & path) override;

protected:
    // Temporary hack for a test: Initializes the data model for testing purposes only.
    // This method serves as a placeholder and should NOT be used outside of specific tests.
    // It is expected to be removed or replaced with a proper implementation in the future.TODO:(#36837).
    virtual void InitDataModelForTesting();

private:
    // Iteration is often done in a tight loop going through all values.
    // To avoid N^2 iterations, cache a hint of where something is positioned
    uint16_t mEndpointIterationHint = 0;

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

    /// Find the index of the given endpoint id
    std::optional<unsigned> TryFindEndpointIndex(EndpointId id) const;
};

} // namespace app
} // namespace chip
