/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterface.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {
/**
 * @brief An implementation of DataModel::Provider that constructs the data model
 *        programmatically by aggregating EndpointInterface instances.
 *
 * This provider allows applications to define their Matter device data model (endpoints,
 * clusters, attributes, commands) dynamically at runtime. It manages a list of EndpointInterface
 * objects, each representing an endpoint on the device.
 *
 * The expected usage pattern by the application is as follows:
 * 1. Instantiate ServerClusterInterface(s) and ServerClusterRegistration(s).
 * 2. Instantiate EndpointInterface(s) and EndpointInterfaceRegistration(s).
 * 2. Instantiate the CodeDrivenDataModelProvider.
 * 3. Register ServerClusterInterfaceRegistration(s) to the CodeDrivenDataModelProvider using AddCluster().
 * 4. Register EndpointInterfaceRegistration(s) to the CodeDrivenDataModelProvider using AddEndpoint().
 *    Note: Step 4 MUST come after Step 3 (Endpoint needs to know about its clusters).
 * 5. Call Startup() on the CodeDrivenDataModelProvider.
 *
 * Note: if the CodeDrivenDataModelProvider has already been started (runtime change to add/remove Endpoints/Clusters),
 *       the Startup() method on each ServerClusterInterface will be called when the EndpointInterface is added (Step 4).
 *       If the provider hasn't been started, the Startup() method will be called when the provider is started (Step 5).
 *
 * TODO: Notify composition changes when the provider is started up and endpoints are added/removed at runtime.
 *       For now, applications are responsible for handling composition changes and calling markDirty() when needed.
 *
 * Lifecycle:
 * - The CodeDrivenDataModelProvider stores raw pointers to EndpointInterface and ServerClusterInterface.
 *   It does NOT take ownership. Callers must ensure these instances outlive the provider.
 */
class CodeDrivenDataModelProvider : public DataModel::Provider
{
public:
    CodeDrivenDataModelProvider(PersistentStorageDelegate & storage, AttributePersistenceProvider & attributeStorage) :
        mPersistentStorageDelegate(storage), mAttributePersistenceProvider(attributeStorage)
    {}

    /* DataModel::Provider implementation */
    CHIP_ERROR Startup(DataModel::InteractionModelContext context) override;
    CHIP_ERROR Shutdown() override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType,
                                        FabricIndex accessingFabric) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    /* ProviderMetadataTree implementation */
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<DataModel::EndpointEntry> & out) override;
    CHIP_ERROR DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) override;
    CHIP_ERROR ClientClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ClusterId> & out) override;
    CHIP_ERROR ServerClusters(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> & out) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & out) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & out) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override;
    void Temporary_ReportAttributeChanged(const AttributePathParams & path) override;

#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
    CHIP_ERROR EndpointUniqueID(EndpointId endpointId, MutableCharSpan & EndpointUniqueId) override;
#endif

    /**
     * @brief Adds an endpoint to the data model provider.
     *
     * This method registers an endpoint, making it part of the device's data model.
     * If the provider has already been started, this may trigger a Startup() call on
     * each ServerClusterInterface associated with the endpoint.
     * The Startup() call on the associated clusters will ONLY happen if this is the first
     * endpoint associated with the cluster (i.e. ServerClusterInterface.GetPaths() returns
     * at least one path with endpoint ID == registration.endpointEntry.id, and none
     * of the other endpoints in GetPaths() are registered yet). This ensures
     * the cluster is only started once, even if it is associated with multiple endpoints.
     *
     * Prerequisites:
     *   - It MUST be called after all clusters for the endpoint have been registered with
     *     AddCluster().
     *   - The provided `registration` (EndpointInterfaceRegistration) must not already be
     *     part of another list (i.e., `registration.next` must be nullptr).
     *   - The EndpointInterface within the `registration` must be valid (i.e.,
     *     `registration.endpointInterface` must not be nullptr).
     *   - The `registration.endpointEntry.id` must be valid (not `kInvalidEndpointId` and
     *     not used by another endpoint).
     *   - The LIFETIME of `registration` must outlive the provider (or the registration must
     *     be removed using `RemoveEndpoint` before it goes away).
     *
     * @param registration The registration object for the endpoint, containing a valid
     *                     EndpointInterface and Endpoint ID.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_INVALID_ARGUMENT if `registration.next` is not nullptr or
     *                                     `registration.endpointInterface` is nullptr or
     *                                     `registration.endpointEntry.id` is kInvalidEndpointId.
     *         CHIP_ERROR_DUPLICATE_KEY_ID if `registration.endpointEntry.id` is already in use.
     */
    CHIP_ERROR AddEndpoint(EndpointInterfaceRegistration & registration);

    /**
     * @brief Removes an endpoint from the data model provider.
     *
     * This method unregisters an endpoint, removing it from the device's data model.
     * If the provider has already been started, this might trigger a Shutdown() call on
     * each ServerClusterInterface associated with the endpoint.
     * The Shutdown() call on the associated clusters will ONLY happen if this is the last
     * endpoint associated with the cluster (i.e. ServerClusterInterface.GetPaths() returns
     * no paths with valid Endpoint IDs).
     *
     * Note: Removing an Endpoint does not remove any clusters associated with the endpoint.
     *       Those can be removed using RemoveCluster() AFTER the endpoint has been removed.

     * Prerequisites:
     *   - It MUST be called BEFORE removing associted clusters with RemoveCluster() to guarantee
     *     the endpoint removal is atomic.
     *   - The endpoint ID must be valid.
     *
     * @param endpointId The ID of the endpoint to remove.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_NOT_FOUND if no endpoint with the given ID is registered.
     *         CHIP_ERROR_INVALID_ARGUMENT if endpointId is kInvalidEndpointId.
     */
    CHIP_ERROR RemoveEndpoint(EndpointId endpointId);

    /**
     * @brief Add a ServerClusterInterface to the Data Model Provider.
     *
     * Requirements:
     *   - entry MUST NOT be part of any other registration
     *   - paths MUST NOT be part of any other ServerClusterInterface (i.e. only a single
     *     registration for a given `endpointId/clusterId` path).
     *   - The LIFETIME of entry must outlive the provider (or the entry must be unregistered
     *     via RemoveCluster before it goes away).
     *   - If the provider has already been started, this method must be called prior to
     *     calling AddEndpoint() (i.e. the `endpointId` of all paths in `entry.GetPaths()`
     *     must NOT be registered in the provider yet), otherwise this would cause non-atomic
     *     changes to an endpoint, which is not allowed.
     *
     * @param entry The ServerClusterRegistration containing the cluster to register.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_INVALID_ARGUMENT if `entry.next` is not `nullptr` or
     *                                     `entry.serverClusterInterface` is `nullptr` or
     *                                     `entry.serverClusterInterface.GetPaths()` is empty/invalid.
     *         CHIP_ERROR_DUPLICATE_KEY_ID if the cluster is already registered.
     *         CHIP_ERROR_INCORRECT_STATE if the provider has been started and an endpoint for
     *                                    one of the cluster paths has already been registered.
     */
    CHIP_ERROR AddCluster(ServerClusterRegistration & entry);

    /**
     * @brief Remove a ServerClusterInterface from the Data Model Provider.
     *
     * To avoid violating the requirement of non-atomic changes to endpoints, this SHALL only be
     * called after all endpoints associated with the cluster have been removed using RemoveEndpoint().
     *
     * Requirements:
     *   - entry MUST be valid
     *   - The `endpointId` of all paths in `entry.GetPaths()` are no longer registered in the provider.
     *
     * @param entry The ServerClusterInterface to remove.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_INVALID_ARGUMENT if `entry` is nullptr.
     *         CHIP_ERROR_NOT_FOUND if the entry is not registered.
     *         CHIP_ERROR_INCORRECT_STATE if an endpoint for one of the cluster paths is still registered.
     */
    CHIP_ERROR RemoveCluster(ServerClusterInterface * entry);

private:
    EndpointInterfaceRegistry mEndpointInterfaceRegistry;
    ServerClusterInterfaceRegistry mServerClusterRegistry;
    std::optional<ServerClusterContext> mServerClusterContext;
    std::optional<DataModel::InteractionModelContext> mInteractionModelContext;
    PersistentStorageDelegate & mPersistentStorageDelegate;
    AttributePersistenceProvider & mAttributePersistenceProvider;

    /// Return the interface registered for the given endpoint ID or nullptr if one does not exist
    EndpointInterface * GetEndpointInterface(EndpointId endpointId);

    /// Return the interface registered for the given cluster path or nullptr if one does not exist
    ServerClusterInterface * GetServerClusterInterface(const ConcreteClusterPath & path);
};

} // namespace app
} // namespace chip
