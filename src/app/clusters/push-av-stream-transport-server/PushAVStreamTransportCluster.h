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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/push-av-stream-transport-server/constants.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-delegate.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-storage.h>
#include <app/clusters/tls-certificate-management-server/tls-certificate-management-server.h>
#include <app/clusters/tls-client-management-server/tls-client-management-server.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <functional>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

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
        DefaultServerCluster({ aEndpointId, PushAvStreamTransport::Id }), mFeatures(aFeatures),
        mSupportedFormats{ PushAvStreamTransport::SupportedFormatStruct{ PushAvStreamTransport::ContainerFormatEnum::kCmaf,
                                                                         PushAvStreamTransport::IngestMethodsEnum::kCMAFIngest } }
    {}

    void SetDelegate(PushAvStreamTransportDelegate * delegate)
    {
        if (delegate == nullptr)
        {
            ChipLogError(Zcl, "Push AV Stream Transport : Trying to set delegate to null");
            return;
        }

        mDelegate = delegate;
        mDelegate->SetPushAvStreamTransportServer(this);
    }

    void SetTLSClientManagementDelegate(TlsClientManagementDelegate * delegate)
    {
        mTLSClientManagementDelegate = delegate;
        if (mTLSClientManagementDelegate == nullptr)
        {
            ChipLogError(Zcl, "Push AV Stream Transport : Trying to set TLS Client Management delegate to null");
            return;
        }
    }

    void SetTlsCertificateManagementDelegate(TlsCertificateManagementDelegate * delegate)
    {
        mTlsCertificateManagementDelegate = delegate;
        if (mTlsCertificateManagementDelegate == nullptr)
        {
            ChipLogError(Zcl, "Push AV Stream Transport: Trying to set TLS Certificate Management delegate to null");
            return;
        }
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
                               Optional<PushAvStreamTransport::TriggerActivationReasonEnum>());
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
                                                               PushAvStreamTransport::TransportTriggerTypeEnum triggerType);

    CHIP_ERROR Init()
    {
        LoadPersistentAttributes();
        return CHIP_NO_ERROR;
    }

    void Shutdown() override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    enum class UpsertResultEnum : uint8_t
    {
        kInserted = 0x00,
        kUpdated  = 0x01,
    };

    struct PushAVStreamTransportDeallocateCallbackContext
    {
        PushAvStreamTransportServer * instance;
        uint16_t connectionID;
    };

    std::vector<std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext>> mTimerContexts;

    BitFlags<PushAvStreamTransport::Feature> mFeatures;

    std::vector<PushAvStreamTransport::SupportedFormatStruct> mSupportedFormats;

    std::vector<PushAvStreamTransport::TransportConfigurationStorage> mCurrentConnections;

    // Methods coming from previous LOGIC implementation
    bool HasFeature(PushAvStreamTransport::Feature feature) const;

    Protocols::InteractionModel::Status ValidateIncomingTransportOptions(
        const PushAvStreamTransport::Structs::TransportOptionsStruct::DecodableType & transportOptions);

    std::optional<DataModel::ActionReturnStatus>
    HandleAllocatePushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                const PushAvStreamTransport::Commands::AllocatePushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleDeallocatePushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                  const PushAvStreamTransport::Commands::DeallocatePushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleModifyPushTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                              const PushAvStreamTransport::Commands::ModifyPushTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleSetTransportStatus(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                             const PushAvStreamTransport::Commands::SetTransportStatus::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleManuallyTriggerTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                   const PushAvStreamTransport::Commands::ManuallyTriggerTransport::DecodableType & commandData);

    std::optional<DataModel::ActionReturnStatus>
    HandleFindTransport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                        const PushAvStreamTransport::Commands::FindTransport::DecodableType & commandData);

    // Send Push AV Stream Transport events
    Protocols::InteractionModel::Status
    GeneratePushTransportBeginEvent(const uint16_t connectionID, const PushAvStreamTransport::TransportTriggerTypeEnum triggerType,
                                    const Optional<PushAvStreamTransport::TriggerActivationReasonEnum> activationReason);
    Protocols::InteractionModel::Status GeneratePushTransportEndEvent(const uint16_t connectionID);

private:
    // Previous LOGIC state move to CLUSTER implementation
    PushAvStreamTransportDelegate * mDelegate                            = nullptr;
    TlsClientManagementDelegate * mTLSClientManagementDelegate           = nullptr;
    TlsCertificateManagementDelegate * mTlsCertificateManagementDelegate = nullptr;

    // Helpers to read list items
    CHIP_ERROR ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder, FabricIndex fabricIndex);
    CHIP_ERROR ReadAndEncodeSupportedFormats(const AttributeValueEncoder::ListEncodeHelper & encoder);

    /// Convenience method that returns if the internal delegate is null and will log
    /// an error if the check returns true
    bool IsNullDelegateWithLogging(EndpointId endpointIdForLogging);

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

    // Helper functions
    uint16_t GenerateConnectionID();

    PushAvStreamTransport::TransportConfigurationStorage * FindStreamTransportConnection(const uint16_t connectionID);

    PushAvStreamTransport::TransportConfigurationStorage * FindStreamTransportConnectionWithinFabric(const uint16_t connectionID,
                                                                                                     FabricIndex fabricIndex);

    // Add/Remove Management functions for transport
    UpsertResultEnum
    UpsertStreamTransportConnection(const PushAvStreamTransport::TransportConfigurationStorage & transportConfiguration);

    void RemoveStreamTransportConnection(const uint16_t connectionID);

    static void PushAVStreamTransportDeallocateCallback(chip::System::Layer *, void * callbackContext);

    void RemoveTimerAppState(const uint16_t connectionID);

    Protocols::InteractionModel::Status CheckPrivacyModes(Globals::StreamUsageEnum streamUsage);

    /**
     * @brief Schedule deallocate with a given timeout
     *
     * @param connectionID    ID of the connection to deallocate
     * @param timeoutSec      timeout in seconds
     * @return               CHIP_ERROR code indicating the result of the operation
     */
    CHIP_ERROR ScheduleTransportDeallocate(uint16_t connectionID, uint32_t timeoutSec);

    /**
     * @brief Validates the provided URL.
     *
     * @param url The URL to validate
     * @return true if URL is valid, false otherwise
     */
    bool ValidateUrl(const std::string & url);
};

} // namespace Clusters
} // namespace app
} // namespace chip
