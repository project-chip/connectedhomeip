/*
 *
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

#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportLogic.h>
#include <app/clusters/push-av-stream-transport-server/constants.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-delegate.h>
#include <app/server-cluster/DefaultServerCluster.h>

namespace chip {
namespace app {
namespace Clusters {

/// Integration of Push AV Stream Transport logic within the matter data model
///
/// Translates between matter calls and Push AV Stream Transport logic
class PushAvStreamTransportServer : public DefaultServerCluster
{
public:
    /**
     * @brief Creates a Push AV Stream Transport server instance
     *
     * This instance needs to be initialized by calling Init() before it can be registered
     * and used by the interaction model.
     *
     * @param aEndpointId The endpoint on which this cluster exists (must match zap configuration)
     * @param aFeatures   Bitflags indicating which features are supported by this instance
     *
     * @note The caller must ensure the delegate lives throughout the instance's lifetime
     */
    PushAvStreamTransportServer(EndpointId aEndpointId, BitFlags<PushAvStreamTransport::Feature> aFeatures) :
        DefaultServerCluster({ aEndpointId, PushAvStreamTransport::Id }), mLogic(aEndpointId, aFeatures)
    {}

    PushAvStreamTransportServerLogic & GetLogic() { return mLogic; }

    void SetDelegate(PushAvStreamTransportDelegate * delegate)
    {
        mLogic.SetDelegate(delegate);
        if (delegate != nullptr)
        {
            delegate->SetPushAvStreamTransportServer(this);
        }
    }

    void SetTLSClientManagementDelegate(TlsClientManagementDelegate * delegate) { mLogic.SetTLSClientManagementDelegate(delegate); }

    void SetTlsCertificateManagementDelegate(TlsCertificateManagementDelegate * delegate)
    {
        mLogic.SetTlsCertificateManagementDelegate(delegate);
    }

    /**
     * @brief API for application layer to notify when transport has started
     *
     * This should be called by the application layer when a transport begins streaming.
     * It will generate the appropriate PushTransportBegin event.
     *
     * @param connectionID The connection ID of the transport that started
     * @param triggerType The type of trigger that started the transport
     * @param activationReason Optional reason for the activation
     * @return Status::Success if event was generated successfully, failure otherwise
     */
    Protocols::InteractionModel::Status
    NotifyTransportStarted(uint16_t connectionID, PushAvStreamTransport::TransportTriggerTypeEnum triggerType,
                           Optional<PushAvStreamTransport::TriggerActivationReasonEnum> activationReason =
                               Optional<PushAvStreamTransport::TriggerActivationReasonEnum>())
    {
        return mLogic.NotifyTransportStarted(connectionID, triggerType, activationReason);
    }

    /**
     * @brief API for application layer to notify when transport has stopped
     *
     * This should be called by the application layer when a transport stops streaming.
     * It will generate the appropriate PushTransportEnd event.
     *
     * @param connectionID The connection ID of the transport that stopped
     * @param triggerType The type of trigger that started the transport
     * @return Status::Success if event was generated successfully, failure otherwise
     */
    Protocols::InteractionModel::Status NotifyTransportStopped(uint16_t connectionID,
                                                               PushAvStreamTransport::TransportTriggerTypeEnum triggerType)
    {
        return mLogic.NotifyTransportStopped(connectionID, triggerType);
    }

    CHIP_ERROR Init() { return mLogic.Init(); }

    void Shutdown(ClusterShutdownType type) override
    {
        DefaultServerCluster::Shutdown(type);
        mLogic.Shutdown();
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

private:
    PushAvStreamTransportServerLogic mLogic;

    // Helpers to read list items
    CHIP_ERROR ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder, FabricIndex fabricIndex);
    CHIP_ERROR ReadAndEncodeSupportedFormats(const AttributeValueEncoder::ListEncodeHelper & encoder);
};

} // namespace Clusters
} // namespace app
} // namespace chip
