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
     * clusters, attributes, commands) using code. It manages a list of EndpointInterface
     * objects, each representing an endpoint on the device.
     *
     * Expected Usage:
     * 1. Instantiate an EndpointInterface implementation.
     * 2. Create an instance of CodeDrivenDataModelProvider.
     * 3. Add the endpoint provider instances to the CodeDrivenDataModelProvider using AddEndpoint().
     * 4. Call Startup() on the CodeDrivenDataModelProvider.
     *
     * Lifecycle:
     * - The CodeDrivenDataModelProvider stores raw pointers to EndpointInterface.
     *   It does NOT take ownership. Callers must ensure these instances outlive the provider.
     */

    class CodeDrivenDataModelProvider : public DataModel::Provider {
    public:
        CHIP_ERROR Startup(DataModel::InteractionModelContext context) override;
        CHIP_ERROR Shutdown() override;

        DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
            AttributeValueEncoder & encoder) override;
        DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
            AttributeValueDecoder & decoder) override;

        void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType) override;
        std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
            TLV::TLVReader & input_arguments, CommandHandler * handler) override;

        /* Attribute tree iteration */
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
         * @brief Sets the persistent storage delegate for the provider.
         *
         * This function MUST be called before Startup(). Calling it after Startup() is not recommended
         * as it may lead to unintended side effects on clusters that are already started.
         */
        void SetPersistentStorageDelegate(PersistentStorageDelegate * delegate);
        PersistentStorageDelegate * GetPersistentStorageDelegate() const;

        /* Lifecycle Management:
         * The CodeDrivenDataModelProvider stores pointers to EndpointInterface, but does NOT take ownership.
         * Any EndpointInterface instance MUST outlive the CodeDrivenDataModelProvider it is registered with.
         * Similarly, EndpointInterfaceRegistration objects passed to AddEndpoint must outlive their registration.
         */
        CHIP_ERROR AddEndpoint(EndpointInterfaceRegistration & registration);
        CHIP_ERROR RemoveEndpoint(EndpointId endpointId);

    private:
        EndpointInterface * GetEndpointInterface(EndpointId endpointId);
        ServerClusterInterface * GetServerCluster(EndpointId endpointId, ClusterId clusterId,
            DataModel::ActionReturnStatus * outStatus);
        EndpointInterfaceRegistry mEndpointInterfaceRegistry;
        std::optional<ServerClusterContext> mServerClusterContext;
    };

} // namespace app
} // namespace chip
