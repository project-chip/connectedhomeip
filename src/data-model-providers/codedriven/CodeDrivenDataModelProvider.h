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
 * Expected Usage:
 * 1. Instantiate an EndpointInterface and EndpointInterfaceRegistration.
 * 2. Instantiate a ServerClusterInterface and ServerClusterRegistration.
 * 2. Create an instance of CodeDrivenDataModelProvider.
 * 3. Add the EndpointInterfaceRegistration to the CodeDrivenDataModelProvider using AddEndpoint().
 * 4. Add the ServerClusterInterfaceRegistration to the CodeDrivenDataModelProvider using AddCluster().
 * 5. Call Startup() on the CodeDrivenDataModelProvider.
 *
 * Lifecycle:
 * - The CodeDrivenDataModelProvider stores raw pointers to EndpointInterface and ServerClusterInterface.
 *   It does NOT take ownership. Callers must ensure these instances outlive the provider.
 */
class CodeDrivenDataModelProvider : public DataModel::Provider
{
public:
    CodeDrivenDataModelProvider(PersistentStorageDelegate * delegate = nullptr) : mPersistentStorageDelegate(delegate) {}

    /* DataModel::Provider implementation */
    CHIP_ERROR Startup(DataModel::InteractionModelContext context) override;
    CHIP_ERROR Shutdown() override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    /* ProviderMetadataTree implementation */
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<DataModel::EndpointEntry> & out) override;
    CHIP_ERROR SemanticTags(EndpointId endpointId,
                            ReadOnlyBufferBuilder<Clusters::Descriptor::Structs::SemanticTagStruct::Type> & out) override;
    CHIP_ERROR DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) override;
    CHIP_ERROR ClientClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ClusterId> & out) override;
    CHIP_ERROR ServerClusters(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> & out) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & out) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & out) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override;
    void Temporary_ReportAttributeChanged(const AttributePathParams & path) override;

    /**
     * @brief Adds an endpoint to the data model provider.
     *
     * This method registers an endpoint, making it part of the device's data model.
     *
     * The provided `registration` (EndpointInterfaceRegistration) must not already be
     * part of another list (i.e., `registration.next` must be nullptr).
     *
     * The EndpointInterface within the `registration` must be valid (i.e.,
     * `registration.endpointInterface` must not be nullptr).
     *
     * Endpoint ID behavior:
     * - If `registration.endpointId` is set to `kInvalidEndpointId`, the provider will assign a new ID
     *   based on `mNextAvailableEndpointId`. This ID starts at 0 and increments with each new endpoint added.
     * - If the user provides a `registration.endpointId` that's valid
     *   (i.e. `registration.endpointId >= mNextAvailableEndpointId`), it will use the provided ID and update
     *   `mNextAvailableEndpointId` to be `registration.endpointId + 1`.
     * - If `registration.endpointId` is set to an endpoint ID already in use
     *   (i.e., `registration.endpointId < mNextAvailableEndpointId`), it will return an error.
     *
     * @param registration The registration object for the endpoint, containing the
     *                     EndpointInterface and its ID. The registration object must
     *                     outlive the provider.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_INVALID_ARGUMENT if registration.next is not nullptr or
     *                                     registration.endpointInterface is nullptr.
     *         CHIP_ERROR_DUPLICATE_KEY_ID if `registration.endpointId` < `mNextAvailableEndpointId`.
     */
    CHIP_ERROR AddEndpoint(EndpointInterfaceRegistration & registration);
    CHIP_ERROR RemoveEndpoint(EndpointId endpointId);

    /* Add a ServerClusterInterface to the Data Model Provider.
     *
     * Requirements:
     *   - entry MUST NOT be part of any other registration
     *   - paths MUST NOT be part of any other ServerClusterInterface (i.e. only a single
     *     registration for a given `endpointId/clusterId` path)
     *   - endpointId of all paths must already be registered in the provider
     *   - The LIFETIME of entry must outlive the provider (or the entry must be unregistered)
     */
    CHIP_ERROR AddCluster(ServerClusterRegistration & entry);
    CHIP_ERROR RemoveCluster(ServerClusterInterface * entry);

    /// Return the interface registered for the given endpoint ID or nullptr if one does not exist
    EndpointInterface * GetEndpointInterface(EndpointId endpointId);

    /// Return the interface registered for the given cluster path or nullptr if one does not exist
    ServerClusterInterface * GetServerClusterInterface(const ConcreteClusterPath & path);

private:
    EndpointInterfaceRegistry mEndpointInterfaceRegistry;
    ServerClusterInterfaceRegistry mServerClusterRegistry;
    std::optional<ServerClusterContext> mServerClusterContext;
    PersistentStorageDelegate * mPersistentStorageDelegate;
    EndpointId mNextAvailableEndpointId = 0;
};

} // namespace app
} // namespace chip
