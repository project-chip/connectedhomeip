#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/push-av-stream-transport-server/constants.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-delegate.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-storage.h>
#include <app/clusters/tls-certificate-management-server/TLSCertificateManagementCluster.h>
#include <app/clusters/tls-client-management-server/TLSClientManagementCluster.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <functional>
#include <protocols/interaction_model/StatusCode.h>
#include <string>
#include <uriparser/Uri.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

// Internal namespace for helper functions
namespace Internal {

std::string extractTextRange(const UriTextRangeA & range);
std::string extractPath(const UriPathSegmentA * pathHead);

} // namespace Internal

class PushAvStreamTransportServerLogic
{
public:
    PushAvStreamTransportServerLogic(EndpointId aEndpoint, BitFlags<PushAvStreamTransport::Feature> aFeatures);
    ~PushAvStreamTransportServerLogic();

    void SetDelegate(PushAvStreamTransportDelegate * delegate)
    {
        mDelegate = delegate;
        if (mDelegate == nullptr)
        {
            ChipLogError(Zcl, "Push AV Stream Transport : Trying to set delegate to null");
            return;
        }
    }

    void SetTLSClientManagementDelegate(TLSClientManagementDelegate * delegate)
    {
        mTLSClientManagementDelegate = delegate;
        if (mTLSClientManagementDelegate == nullptr)
        {
            ChipLogError(Zcl, "Push AV Stream Transport : Trying to set TLS Client Management delegate to null");
            return;
        }
    }

    void SetTLSCertificateManagementDelegate(TLSCertificateManagementDelegate * delegate)
    {
        mTLSCertificateManagementDelegate = delegate;
        if (mTLSCertificateManagementDelegate == nullptr)
        {
            ChipLogError(Zcl, "Push AV Stream Transport: Trying to set TLS Certificate Management delegate to null");
            return;
        }
    }

    Protocols::InteractionModel::Status
    NotifyTransportStarted(uint16_t connectionID, PushAvStreamTransport::TransportTriggerTypeEnum triggerType,
                           Optional<PushAvStreamTransport::TriggerActivationReasonEnum> activationReason =
                               Optional<PushAvStreamTransport::TriggerActivationReasonEnum>());

    Protocols::InteractionModel::Status NotifyTransportStopped(uint16_t connectionID,
                                                               PushAvStreamTransport::TransportTriggerTypeEnum triggerType);

    enum class UpsertResultEnum : uint8_t
    {
        kInserted = 0x00,
        kUpdated  = 0x01,
    };

    struct PushAVStreamTransportDeallocateCallbackContext
    {
        PushAvStreamTransportServerLogic * instance;
        uint16_t connectionID;
    };

    EndpointId mEndpointId = kInvalidEndpointId;

    std::vector<std::shared_ptr<PushAVStreamTransportDeallocateCallbackContext>> mTimerContexts;

    BitFlags<PushAvStreamTransport::Feature> mFeatures;

    std::vector<PushAvStreamTransport::SupportedFormatStruct> mSupportedFormats;

    std::vector<PushAvStreamTransport::TransportConfigurationStorage> mCurrentConnections;

    CHIP_ERROR Init();

    void Shutdown();

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
                                    const Optional<PushAvStreamTransport::TriggerActivationReasonEnum> activationReason,
                                    const PushAvStreamTransport::ContainerFormatEnum containerType,
                                    const Optional<uint64_t> cmafSessionNumber = Optional<uint64_t>());
    Protocols::InteractionModel::Status GeneratePushTransportEndEvent(const uint16_t connectionID);

private:
    PushAvStreamTransportDelegate * mDelegate                            = nullptr;
    TLSClientManagementDelegate * mTLSClientManagementDelegate           = nullptr;
    TLSCertificateManagementDelegate * mTLSCertificateManagementDelegate = nullptr;

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
