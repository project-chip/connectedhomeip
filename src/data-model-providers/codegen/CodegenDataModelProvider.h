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
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/SingleEndpointServerClusterRegistry.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {

/// An implementation of `DataModel::Provider` that relies on code-generation
/// via zap/ember.
///
/// The Ember framework uses generated files (like endpoint-config.h and various
/// other generated metadata) to provide a cluster model.
///
/// This class will use global functions generally residing in `app/util`
/// as well as application-specific overrides to provide data model functionality.
///
/// Given that this relies on global data at link time, there generally can be
/// only one CodegenDataModelProvider per application. Per-cluster CodegenIntegration
/// function access the global singleton instance via `CodegenDataModelProvider::Instance()`.
class CodegenDataModelProvider : public DataModel::Provider
{
public:
    // access to the typed global singleton of this class.
    static CodegenDataModelProvider & Instance();

    /// clears out internal caching. Especially useful in unit tests,
    /// where path caching does not really apply (the same path may result in different outcomes)
    void Reset() { mPreviouslyFoundCluster = std::nullopt; }

    void SetPersistentStorageDelegate(PersistentStorageDelegate * delegate)
    {
        VerifyOrDie(!mInitialized);
        mPersistentStorageDelegate = delegate;
    }
    PersistentStorageDelegate * GetPersistentStorageDelegate() { return mPersistentStorageDelegate; }

    SingleEndpointServerClusterRegistry & Registry() { return mRegistry; }

    /// Initialize clusters without starting them. This allows applications to configure
    /// cluster delegates and other properties before the clusters are started via Startup().
    /// If not called explicitly, Startup() will call this method automatically.
    virtual CHIP_ERROR Init();

    /// Generic model implementations
    CHIP_ERROR Startup(DataModel::InteractionModelContext context) override;
    CHIP_ERROR Shutdown() override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    void ListAttributeWriteNotification(const ConcreteAttributePath & aPath, DataModel::ListWriteOperation opType,
                                        FabricIndex accessingFabric) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    /// attribute tree iteration
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<DataModel::EndpointEntry> & out) override;
    CHIP_ERROR DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & builder) override;
    CHIP_ERROR ClientClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ClusterId> & builder) override;
    CHIP_ERROR ServerClusters(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> & builder) override;
#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
    CHIP_ERROR EndpointUniqueID(EndpointId endpointId, MutableCharSpan & epUniqueId) override;
#endif
    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    void Temporary_ReportAttributeChanged(const AttributePathParams & path) override;

protected:
    // Temporary hack for a test: Initializes the data model for testing purposes only.
    // This method serves as a placeholder and should NOT be used outside of specific tests.
    // It is expected to be removed or replaced with a proper implementation in the future.TODO:(#36837).
    virtual void InitDataModelForTesting();

private:
    // Context is available after startup and cleared in shutdown.
    // This has a value for as long as we assume the context is valid.
    std::optional<DataModel::InteractionModelContext> mContext;

    // Iteration is often done in a tight loop going through all values.
    // To avoid N^2 iterations, cache a hint of where something is positioned
    uint16_t mEndpointIterationHint = 0;

    // Tracks whether Init() has been called
    bool mInitialized = false;

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

    SingleEndpointServerClusterRegistry mRegistry;

    /// Finds the specified ember cluster
    ///
    /// Effectively the same as `emberAfFindServerCluster` except with some caching capabilities
    const EmberAfCluster * FindServerCluster(const ConcreteClusterPath & path);

    /// Find the index of the given endpoint id
    std::optional<unsigned> TryFindEndpointIndex(EndpointId id) const;
};

} // namespace app
} // namespace chip
